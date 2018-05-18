///////////////////////////////////////////////////////////
/// @file	SharedLock.h
/// @brief	共有メモリーのロック
/// @author	shuji-morimoto
/// Copyright (C) 2013- Mamezou. All rights reserved.
///////////////////////////////////////////////////////////

#ifndef __PICO_IPC_SHARED_LOCK__
#define __PICO_IPC_SHARED_LOCK__

#include "SharedMemory.h"
#include "Thread.h"

namespace PicoIPC {

///////////////////////////////////////////////////////////
/// @class SharedLock
/// @brief	共有メモリーのロック/ロック解除を自動化する
///         自动化共享内存的锁定/解除锁定
/// Semaphoreを利用してロックする
///               使用锁定
/// コンストラクタでSharedMemoryをロックし、デストラクタで解除するため
///  用转换器                              将其锁定，并以死亡方式解除
/// スタック領域(ローカル変数)で利用すること
///
/// 使い方
///   struct UserDefinedStruct
///   {
///       int    a;
///       bool   b;
///       double c;
///   };
///
///   SharedMemory *shm = ...  // UserDefinedStruct用のSharedMemoryが作成済みとする
///     :
///
///   // {から}までのブロックスコープでUserDefinedStructをロックする場合
///   {
///      SharedLock<UserDefinedStruct> l(shm);
///      l->x = 1; 
///      l->y = true; 
///      l->z = 1.23; 
///   }
///
///   // 関数内でUserDefinedStructをロックする場合
///   void function(SharedMemory *shm) {
///      SharedLock<UserDefinedStruct> l(shm);
///      l->x = 1; 
///      l->y = true; 
///      l->z = 1.23; 
///   }
///
///////////////////////////////////////////////////////////
template <typename T>
class SharedLock
{
public:
	///////////////////////////////////////////////////////////
	/// @brief		コンストラクタ
	/// @param[in]	memory SharedMemory
	/// @note		コンストラクタが終了するとmemoryがロック状態となる
	///////////////////////////////////////////////////////////
	SharedLock(SharedMemory *memory)
		: mMemory(memory)
		, mData(memory->Data<T>())
		, mIsYieldEnd(false)
	{
		mMemory->Wait();
	}

	///////////////////////////////////////////////////////////
	/// @brief		コンストラクタ
	/// @param[in]	memory SharedMemory
	/// @param[in]	isYieldEnd デストラクタでCPUを放棄する場合はtrue
	/// @note		コンストラクタが終了するとmemoryがロック状態となる
	///////////////////////////////////////////////////////////
	SharedLock(SharedMemory *memory, bool isYieldEnd)
		: mMemory(memory)
		, mData(memory->Data<T>())
		, mIsYieldEnd(isYieldEnd)
	{
		mMemory->Wait();
	}

	///////////////////////////////////////////////////////////
	/// @brief		デストラクタ
	/// @note		デストラクタが終了するとmemoryのロックが解除された状態となる
	/// @note		isYieldEnd がtrueのときCPUを放棄し、他のプロセスやスレッドにCPUを割り当てる
	///////////////////////////////////////////////////////////
	~SharedLock()
	{
		mMemory->Post();
		if (mIsYieldEnd) {
			PicoIPC::Thread::Yield();
		}
	}

	///////////////////////////////////////////////////////////
	/// @brief		アロー演算子(Arrow operator)オーバーライド
	/// 
	/// SharedMemoryが持つデータを返す
	/// 
	///////////////////////////////////////////////////////////
	T *operator->() const
	{
		return mData;
	}

private:
	SharedMemory *mMemory;     ///< SharedMemory
	T            *mData;       ///< SharedMemoryが持つデータ
	bool          mIsYieldEnd; ///< デストラクタ時にCPU放棄するか
};
}
#endif
