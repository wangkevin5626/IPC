///////////////////////////////////////////////////////////
/// @file	MessageQueue.h
/// @brief	メッセージキュー
/// @author	shuji-morimoto
/// Copyright (C) 2013- Mamezou. All rights reserved.
///////////////////////////////////////////////////////////

#ifndef __PICO_IPC_MESSAGE_QUEUE__
#define __PICO_IPC_MESSAGE_QUEUE__

#include <mqueue.h>
#include <string>
#include <vector>
#include "Error.h"
#include "ByteBuffer.h"

namespace PicoIPC {

class MessageQueue;

///////////////////////////////////////////////////////////
/// @class	INotifyMessage
/// @brief	メッセージ着信通知インタフェース
/// 
/// - メッセージ通知はそれまで空のキューに新しいメッセージが到着した場合のみ行われる
/// - このインタフェースを実装して通知メッセージを受信したときの処理を実装する
///
///////////////////////////////////////////////////////////
class INotifyMessage
{
public:
	///////////////////////////////////////////////////////////
	/// @brief		デストラクタ
	///////////////////////////////////////////////////////////
	virtual ~INotifyMessage() {};

	///////////////////////////////////////////////////////////
	/// @brief		メッセージ到着時の処理を実施する
	/// 
	/// メッセージキューが空の状態でメッセージが到着したときにコールされる。
	/// メッセージが到着したかどうかポーリングしなくてもよいため効率がよい。
	///
	/// メッセージキューにメッセージがあるときに、メッセージが到着
	/// してもコールされないことに注意
	/// 
	/// @param[in]	mq 通知するメッセージキュー
	/// @note		
	///////////////////////////////////////////////////////////
	virtual void FirstMessageArrived(MessageQueue &mq) = 0;
};


///////////////////////////////////////////////////////////
/// @class MessageQueue
/// @brief	POSIXメッセージキュー
///
/// スレッド間やプロセス間で非同期メッセージの形でのデータの
/// やり取りを行う機構を提供する
///
/// メッセージキューの設定について
/// メッセージキューのデフォルト設定は予めシステムで決められている
///
/// $ sysctl -a 2> /dev/null | grep mqueue
/// fs.mqueue.msg_default = 10
/// fs.mqueue.msg_max = 10
/// fs.mqueue.msgsize_default = 8192
/// fs.mqueue.msgsize_max = 8192
/// fs.mqueue.queues_max = 256
///
/// $ ls /proc/sys/fs/mqueue
/// msg_default  msg_max  msgsize_default  msgsize_max  queues_max
///
/// Read
/// cat /proc/sys/fs/mqueue/msg_max で値参照ができる
///
/// Write
/// sudo sysctl -w fs.mqueue.msg_max=1000
/// sudo sh -c 'echo 1000 > /proc/sys/fs/mqueue/msg_max で値設定ができる(admin権限が必要)
///
/// 登録できるデフォルトメッセージ数: /proc/sys/fs/mqueue/msg_default
/// 登録できる最大メッセージ数      : /proc/sys/fs/mqueue/msg_max
/// デフォルトメッセージ長          : /proc/sys/fs/mqueue/msgsize_default
/// 最大メッセージ長                : /proc/sys/fs/mqueue/msgsize_max
/// 最大メッセージキュー数          : /proc/sys/fs/mqueue/queues_max
/// これらの値はシステム全体で適用される
/// (実機環境ではmsg_default,msgsize_defaultが存在しない)
/// 
/// システム起動時に値を変更するには(開発環境のみ)
/// /etc/sysctl.d/NN-xxxxx.confを作成し、fs.mqueue.xxxxx = valueとして登録する
/// 
///////////////////////////////////////////////////////////
class MessageQueue {
public:
	///////////////////////////////////////////////////////////
	/// @brief		指定した名前のメッセージキューが存在するか確認する
	/// @return		Error
	/// @note		nameは'/'で開始する必要がある。例) "/mq1"
	///////////////////////////////////////////////////////////
	static Error Exist(const std::string &name);

	///////////////////////////////////////////////////////////
	/// @brief		コンストラクタ
	///
	/// 作成済みのメッセージキューを参照利用する
	///
	/// @param[in]	name 名前
	/// @note		nameは'/'で開始する必要がある。例) "/mq1"
	/// @note		メッセージキューが存在するかはExist()で判断する
	///////////////////////////////////////////////////////////
	MessageQueue(const std::string &name);

	///////////////////////////////////////////////////////////
	/// @brief		コンストラクタ
	///
	/// メッセージキューを新規作成して利用する
	///
	/// @param[in]	name 名前
	/// @param[in]	maxMessageCount メッセージキューに登録できる最大メッセージ数 > 0
	/// @param[in]	maxMessageSize 最大メッセージ長 > 0
	/// @note		nameは'/'で開始する必要がある。例) "/mq1"
	/// @note		メッセージキューの作成/削除を行う<br/>
	///////////////////////////////////////////////////////////
	MessageQueue(const std::string &name, long maxMessageCount, long maxMessageSize);

	///////////////////////////////////////////////////////////
	/// @brief		デストラクタ
	/// @note
	///////////////////////////////////////////////////////////
	virtual ~MessageQueue();

	///////////////////////////////////////////////////////////
	/// @brief		名前を取得する
	/// @return		名前
	/// @note
	///////////////////////////////////////////////////////////
	const std::string &Name() const;

	///////////////////////////////////////////////////////////
	/// @brief		メッセージキューに登録できる最大メッセージ数を取得する
	/// @param[in]	なし
	/// @return		最大メッセージ数
	/// @note		コンストラクタで指定したmaxMessageCountを取得する
	///////////////////////////////////////////////////////////
	long MaxMessageCount();

	///////////////////////////////////////////////////////////
	/// @brief		メッセージキューに登録できるメッセージの最大サイズを取得する
	/// @param[in]	なし
	/// @return		メッセージ長
	/// @note		コンストラクタで指定したmaxMessageSizeを取得する
	///////////////////////////////////////////////////////////
	long MaxMessageSize();

	///////////////////////////////////////////////////////////
	/// @brief		現在メッセージキューに入っているメッセージ数を取得する
	/// @param[in]	なし
	/// @return		現在メッセージキューに入っているメッセージ数
	/// @note		
	///////////////////////////////////////////////////////////
	long CurrentMessageCount();

	///////////////////////////////////////////////////////////
	/// @brief		メッセージキューに入っているメッセージをクリアする
	/// @param[in]	なし
	/// @return		なし
	/// @note		
	///////////////////////////////////////////////////////////
	void Clear();

	///////////////////////////////////////////////////////////
	/// @brief		メッセージキューにメッセージを送信する
	/// @param[in]	message メッセージ
	/// @return		Error 失敗したときエラー内容がErrorに設定される
	/// @note		メッセージキューに空きがない時、空きができるまでブロックする
	/// @note		メッセージサイズが0のメッセージも可能
	///////////////////////////////////////////////////////////
	Error Send(const ByteBuffer &message);

	///////////////////////////////////////////////////////////
	/// @brief		タイムアウト付きでメッセージキューにメッセージを送信する
	/// @param[in]	message メッセージ
	/// @param[in]	millisec ミリ秒
	/// @return		Error 失敗したときエラー内容がErrorに設定される
	/// @note		millisecが0のときは送信できるまでブロックする
	/// @note		メッセージキューに空きがないとき、指定時間待っても登録できないときエラーとなる
	/// @note		メッセージサイズが0のメッセージも可能
	///////////////////////////////////////////////////////////
	Error TimedSend(const ByteBuffer &message, unsigned long millisec);

	///////////////////////////////////////////////////////////
	/// @brief		メッセージキューからメッセージを受信する
	/// @param[out]	outMessage メッセージ
	/// @return		Error 失敗したときエラー内容がErrorに設定される
	/// @note		メッセージキューが空の時、新規に追加されたメッセージを取得できるまでブロックする
	/// @note		メッセージサイズが0のメッセージも可能
	///////////////////////////////////////////////////////////
	Error Receive(ByteBuffer &outMessage);

	///////////////////////////////////////////////////////////
	/// @brief		タイムアウト付きでメッセージキューからメッセージを受信する
	/// @param[out]	message メッセージ
	/// @param[in]	millisec ミリ秒
	/// @return		Error 失敗したときエラー内容がErrorに設定される
	/// @note		メッセージキューが空の時、指定時間待っても取得できないときエラーとなる
	/// @note		millisecが0のときは取得できるまでブロックする
	/// @note		メッセージサイズが0のメッセージも可能
	///////////////////////////////////////////////////////////
	Error TimedReceive(ByteBuffer &outMessage, unsigned long millisec);

	///////////////////////////////////////////////////////////
	/// @brief		メッセージキューに溜まっているすべてのメッセージを受信する
	/// @param[out]	outMessages メッセージ一覧
	/// @return		Error 失敗したときエラー内容がErrorに設定される
	/// @note		メッセージがないときはErrorは成功で返り、outMessagesサイズは0となる
	/// @note		メッセージサイズが0のメッセージも可能
	///////////////////////////////////////////////////////////
	Error Receive(std::vector<ByteBuffer> &outMessages);

	///////////////////////////////////////////////////////////
	/// @brief		INotifyMessageインタフェースを設定する
	/// @param[in]	notification 通知を受けるハンドラー
	/// @note		メッセージキューが空のキュー(size 0)の時から<br/>
	///             新しいメッセージが到着したときのみ通知することに注意
	/// @note		NULLを設定すると通知を行わない
	///////////////////////////////////////////////////////////
	void SetNotifyMessage(INotifyMessage *notification);

	///////////////////////////////////////////////////////////
	/// @brief		INotifyMessageインタフェースを取得する
	/// @param[in]	なし
	/// @return		INotifyMessage 
	/// @note		
	///////////////////////////////////////////////////////////
	INotifyMessage *NotifyMessage();

private:
public:
	std::string     mName;         ///< 名前
	bool            mIsOwner;      ///< 所有権
	mqd_t           mMessageQueue; ///< POSIX メッセージキュー
	mq_attr         mAttribute;    ///< メッセージキュー属性
	INotifyMessage *mNotification; ///< INotifyMessage
	sigevent        mSignalEvent;  ///< シグナルイベント

	///////////////////////////////////////////////////////////
	/// @brief	初期化を行う
	/// @param[in]	maxMessageCount メッセージキューに登録できる最大メッセージ数
	/// @param[in]	maxMessageSize 最大メッセージ長
	///////////////////////////////////////////////////////////
	void Init(long maxMessageCount, long maxMessageSize);
};
}
#endif
