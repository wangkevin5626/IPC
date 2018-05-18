///////////////////////////////////////////////////////////
/// @file	UnixDomainSocket.h
/// @brief	UNIXドメインソケット
/// @author	shuji-morimoto
/// Copyright (C) 2013- Mamezou. All rights reserved.
///////////////////////////////////////////////////////////

#ifndef __PICO_IPC_UNIX_DOMAIN_SOCKET__
#define __PICO_IPC_UNIX_DOMAIN_SOCKET__

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "ByteBuffer.h"
#include "Error.h"

namespace PicoIPC {

///////////////////////////////////////////////////////////
/// @class UnixDomainSocket
/// @brief	UNIXドメインソケットでデータグラムを利用する
/// 
///  - 同一ホスト間での1対1型のプロセス間通信で利用
///  - TCP接続よりも高速
///  - データが送信単位で分割される
///  - データがなくならない
///  - データの順番は狂わない
///  - 接続先がいないときは接続エラーになる
/// 
///  [送信/受信 メッセージデータ構造]
///               Top                                            Bottom
///               0                                                     n
///               +-----------------+-----------------+-----------------+
///   Category    | Protocol Header |     Header      |      Body       |
///               +-----------------+-----------------+-----------------+
///   Data Type   |    ------       |   ByteBuffer    |   ByteBuffer    |
///               +-----------------+-----------------+-----------------+
///   Owner       |    Framework    |   Application   |   Application   |   
///               +-----------------+-----------------+-----------------+
///   Data Length |  Fixed Length   | Variable Length | Variable Length |
///               |    8 byte       |   < 512 byte    | n byte < limit  |
///               +-----------------+-----------------+-----------------+
///
///////////////////////////////////////////////////////////
class UnixDomainSocket
{
public:
	///////////////////////////////////////////////////////////
	/// @brief		コンストラクタ
	/// @param[in]	path ソケットを表すファイルパス
	/// @param[in]	isOwner 所有権
	/// @note		pathはファイル作成可能なパスでなければならない
	/// @note		1対1で通信する。同一pathかつisOwnerがtrueとfalse
	/// 			との間で通信接続を行う(所有権という意味はない)
	///////////////////////////////////////////////////////////
	UnixDomainSocket(const std::string &path, bool isOwner);

	///////////////////////////////////////////////////////////
	/// @brief		デストラクタ
	///////////////////////////////////////////////////////////
	virtual ~UnixDomainSocket();

	///////////////////////////////////////////////////////////
	/// @brief		ソケットを開く
	/// @return		Error 失敗したときエラー内容がErrorに設定される
	///////////////////////////////////////////////////////////
	Error OpenSocket();

	///////////////////////////////////////////////////////////
	/// @brief		ソケットを閉じる
	///////////////////////////////////////////////////////////
	void CloseSocket();

	///////////////////////////////////////////////////////////
	/// @brief		ソケットが開いているか確認する
	/// @return		trueのときソケットが開いている
	///////////////////////////////////////////////////////////
	bool IsOpend();

	///////////////////////////////////////////////////////////
	/// @brief		接続相手にデータを送信する
	/// @param[in]	header ヘッダーデータ
	/// @param[in]	body  ボディーデータ
	/// @return		Error 失敗したときエラー内容がErrorに設定される
	/// @note		header、bodyの内容は利用者が設定する
	///////////////////////////////////////////////////////////
	Error Send(const ByteBuffer &header, const ByteBuffer &body);

	///////////////////////////////////////////////////////////
	/// @brief		接続相手からデータを受信する
	/// @param[out]	header ヘッダーデータ
	/// @param[out]	body  ボディーデータ
	/// @return		Error 失敗したときエラー内容がErrorに設定される
	/// @note		接続相手がSend()したheader、bodyを受信する
	///////////////////////////////////////////////////////////
	Error Receive(ByteBuffer &outHeader, ByteBuffer &outBody);

	///////////////////////////////////////////////////////////
	/// @brief		最大送受信データサイズを指定する
	/// @param[in]	limit 最大送受信データサイズ
	/// @note		0:無制限, default:0xffffff(16.7Mb)
	///////////////////////////////////////////////////////////
	void SetLimitSize(unsigned int limit);

	///////////////////////////////////////////////////////////
	/// @brief		最大送受信データサイズを取得する
	/// @note		0は無制限を表す
	///////////////////////////////////////////////////////////
	unsigned int LimitSize();

private:
	std::string  mPath;       ///< ファイルパス
	bool         mIsOwner;    ///< データ送受信ファイルパス切替フラグ
	int          mTxSocketFd; ///< 送信用ソケットファイルディスクリプタ
	int          mRxSocketFd; ///< 受信用ソケットファイルディスクリプタ
	sockaddr_un  mTxAddress;  ///< 送信用UNIXドメインソケットアドレス構造体
	sockaddr_un  mRxAddress;  ///< 受信用UNIXドメインソケットアドレス構造体
	bool         mIsOpend;    ///< ソケットオープン状態
	unsigned int mLimitSize;  ///< 最大送受信データサイズ
};
}
#endif
