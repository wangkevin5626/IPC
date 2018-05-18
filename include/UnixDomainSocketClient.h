///////////////////////////////////////////////////////////
/// @file	UnixDomainSocketClient.h
/// @brief	UNIXドメインソケットクライアント
/// @author	shuji-morimoto
/// Copyright (C) 2013- Mamezou. All rights reserved.
///////////////////////////////////////////////////////////

#ifndef __UNIX_DOMAIN_SOCKET_CLIENT__
#define __UNIX_DOMAIN_SOCKET_CLIENT__

#include "UnixDomainSocket.h"
#include "Mutex.h"
#include "Thread.h"

namespace PicoIPC {

///////////////////////////////////////////////////////////
/// @class	INotifyReceiver
/// @brief	通知メッセージ受信インタフェース
/// 
/// - サーバーからの通知メッセージを受信したときにコールされる
/// - このインタフェースを実装して通知メッセージを受信したときの処理を実装する
/// - 受信したときの処理は速やかに終えること
///
///////////////////////////////////////////////////////////
class INotifyReceiver
{
public:
	///////////////////////////////////////////////////////////
	/// @brief		デストラクタ
	///////////////////////////////////////////////////////////
	virtual ~INotifyReceiver() {};

	///////////////////////////////////////////////////////////
	/// @brief		通知受信処理を実装する
	///
	/// この処理を終えるまで次の受信を待つため速やかに処理を終えること
	///
	///////////////////////////////////////////////////////////
	virtual void ReceiveNotify(ByteBuffer &update) = 0;
};


///////////////////////////////////////////////////////////
/// @class	UnixDomainSocketClient
/// @brief	UNIXドメインソケットクライアント
/// 
/// - UnixDomainSocketを継承したクラス
/// - 受信処理は内部でスレッド処理しているためIRunnableを実装している
/// - 接続相手(サーバー)にリクエストを送信し、応答を受信して何らかの処理を行う
/// - 接続相手(サーバー)からの通知を受けるにはINotifyReceiverを実装すること
///
///////////////////////////////////////////////////////////
class UnixDomainSocketClient : public UnixDomainSocket, public IRunnable
{
public:
	///////////////////////////////////////////////////////////
	/// @brief		コンストラクタ
	/// @param[in]	path ソケットを表すファイルパス
	/// @note		pathは通信相手(サーバー)と同じパスを設定する必要がある
	///////////////////////////////////////////////////////////
	UnixDomainSocketClient(const std::string &path);

	///////////////////////////////////////////////////////////
	/// @brief		デストラクタ
	///////////////////////////////////////////////////////////
	virtual ~UnixDomainSocketClient();

	///////////////////////////////////////////////////////////
	/// @brief		INotifyReceiverを設定する
	/// @param[in]	receiver INotifyReceiver
	/// @note		
	///////////////////////////////////////////////////////////
	void SetNotifyReceiver(INotifyReceiver *receiver);

	///////////////////////////////////////////////////////////
	/// @brief		接続相手(サーバー)にリクエストを送信し、応答を受信する
	/// @param[in]	request 送信データ
	/// @param[out]	response 受信データ
	/// @return		Error 失敗したときエラー内容がErrorに設定される
	/// @note		応答を受信するまで待つ
	///////////////////////////////////////////////////////////
	Error SendReceive(ByteBuffer &request, ByteBuffer &response);

	///////////////////////////////////////////////////////////
	/// @brief		接続相手(サーバー)にPINGを送信する
	/// @param[in]	なし
	/// @return		Error 失敗したときエラー内容がErrorに設定される
	/// @note		成功の場合、サーバーは受信待ち状態
	///////////////////////////////////////////////////////////
	Error Ping();

	///////////////////////////////////////////////////////////
	/// @brief		implements IRunnable::Run()
	/// @note		このメソッドをコールしてはいけない
	///////////////////////////////////////////////////////////
	void Run();

private:
	Mutex            mMutex;          ///< 送信処理の同期をとるためのMutex
	Thread           mResponseThread; ///< 受信処理Thread
	INotifyReceiver *mReceiver;       ///< INotifyReceiver
	bool             mIsActive;       ///< 活性化状態

	Mutex            mResponseMutex;  ///< 受信用Mutex
	bool             mResponseArrived;///< 受信データ到着状態
	Error            mIsResponseError;///< 受信エラー情報
	ByteBuffer       mResponseHeader; ///< 受信ヘッダー情報
	ByteBuffer       mResponse;       ///< 受信ボディー情報

	void Start(bool isBlock); // start response data service (block here if isBlock is true)
	void Stop();			  // stop service
	Error PrivateSendReceive(ByteBuffer &request, ByteBuffer &response, unsigned int requestType);
};
}
#endif
