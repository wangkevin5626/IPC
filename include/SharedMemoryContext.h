///////////////////////////////////////////////////////////
/// @file	SharedMemoryContext.h
/// @brief	共有メモリーコンテキスト
/// @author	shuji-morimoto
/// Copyright (C) 2013- Mamezou. All rights reserved.
///////////////////////////////////////////////////////////

#ifndef __PICO_IPC_SHARED_MEMORY_CONTEXT__
#define __PICO_IPC_SHARED_MEMORY_CONTEXT__

#include <string>
#include <map>
#include "SharedMemory.h"

namespace PicoIPC {

///////////////////////////////////////////////////////////
/// @class SharedMemoryContext
/// @brief	共有メモリーの生成と破棄を管理する
///          管理共享存储器的生成和废弃
///////////////////////////////////////////////////////////
class SharedMemoryContext
{
public:
	///////////////////////////////////////////////////////////
	/// @brief		コンストラクタ
	///////////////////////////////////////////////////////////
	SharedMemoryContext();

	///////////////////////////////////////////////////////////
	/// @brief		デストラクタ
	///
	/// Bind()により作成されたSharedMemoryは自動的に開放される
	///
	///////////////////////////////////////////////////////////
	virtual ~SharedMemoryContext();

	///////////////////////////////////////////////////////////
	/// @brief		バインドするPOD型データを指定してSharedMemoryを取得する
	/// @param[in]	name 名前
	/// @param[in]	isOwner 所有権
	/// @note		nameは'/'で開始する必要がある。例) "/shared_memory1"
    ///                       有必要开始。例子
	/// @note		isOwnerがtrueの場合 共有メモリーが存在しない場合は作成を行う<br/>
    ///                                不存在共享内存的情况下进行制作
	/// 			falseの場合は作成済みの共有メモリーを利用する、共有メモリーが存在しない場合はNULLを返す<br/>
    ///                     的情况下，使用已创建的共享存储器，如果不存在共享内存，则返回NUMS
	/// 			SharedMemoryの開放はSharedMemoryContextが行うため<br/>
	///				利用者は取得したSharedMemoryをdeleteしてはいけない<br/>
	///              利用者は取得した
	/// 			例) SharedMemory *sm = context->bind<MyStruct>("/shared_memory", false);
	///////////////////////////////////////////////////////////
	template<typename T> SharedMemory *Bind(const std::string &name, bool isOwner)
	{
		return BindSharedMemory(name, sizeof(T), isOwner);
	}

private:
	std::map<const std::string, SharedMemory *> mSharedMemories; ///< SharedMemory一覧

	SharedMemory *BindSharedMemory(const std::string &name, size_t size, bool isOwner);
};

}
#endif
