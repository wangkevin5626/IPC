#include <stdio.h>
#include <unistd.h>
#include <vector>
#include "Mutex.h"
#include "MutexLock.h"
#include "Thread.h"

using namespace PicoIPC;

class SleepHandler : public IRunnable
{
public:
	void Run()
	{
		Thread *t = Thread::CurrentThread();
		::printf("[%lu] lock\n", t->ThreadId());
		MutexLock lock(static_cast<Mutex *>(t->Parameter()));
		::printf("[%lu] locked\n", t->ThreadId());
		Thread::Sleep(3);
		::printf("[%lu] wakeup\n", t->ThreadId());
	}
};

class CondWaitHandler : public IRunnable
{
public:
	void Run()
	{
		Thread *t = Thread::CurrentThread();
		::printf("[%lu] start cond_wait\n", t->ThreadId());
		MutexLock lock(static_cast<Mutex *>(t->Parameter()));
		::printf("[%lu] wait\n", t->ThreadId());
		lock.Wait();
		::printf("[%lu] wakeup\n", t->ThreadId());
	}
};

class CondSignalHandler : public IRunnable
{
public:
	void Run()
	{
		Thread::Sleep(3);
		Thread *t = Thread::CurrentThread();
		::printf("[%lu] start cond_signal\n", t->ThreadId());
		MutexLock lock(static_cast<Mutex *>(t->Parameter()));
		::printf("[%lu] signal\n", t->ThreadId());
		lock.Signal();
	}
};

class CondBrodcastHandler : public IRunnable
{
public:
	void Run()
	{
		Thread::Sleep(3);
		Thread *t = Thread::CurrentThread();
		::printf("[%lu] start cond_brodcast\n", t->ThreadId());
		MutexLock lock(static_cast<Mutex *>(t->Parameter()));
		::printf("[%lu] broadcast\n", t->ThreadId());
		lock.Broadcast();
	}
};

class LoopHandler : public IRunnable
{
public:
	void Run()
	{
		Thread *t = Thread::CurrentThread();
		Mutex *mutex = static_cast<Mutex *>(t->Parameter());
		while (true) {
			MutexLock lock(mutex, true);
			::printf("[%lu] %s\n", t->ThreadId(), t->Name().c_str());
		}
	}
};

void test1()
{
	printf("\ntest1() lock/unlock test\n");
	Mutex mutex;
	SleepHandler sleep_handler;

	Thread t1(&sleep_handler, &mutex);
	Thread t2(&sleep_handler, &mutex);

	t1.Start();
	t2.Start();

	t1.Join();
	t2.Join();
}

void test2()
{
	printf("\ntest2() wait/signal test\n");
	Mutex mutex;
	CondWaitHandler cond_wait;
	CondSignalHandler cond_signal;;
	
	Thread t1(&cond_wait, &mutex);
	Thread t2(&cond_signal, &mutex);// wakeup only one thread

	t1.Start();
	t2.Start();

	t1.Join();
	t2.Join();
}

void test3()
{
	printf("\ntest3() wait/broadcast test\n");
	Mutex mutex;
	CondWaitHandler cond_wait;
	CondBrodcastHandler cond_brodcast;
	
	Thread t1(&cond_wait, &mutex);
	Thread t2(&cond_wait, &mutex);
	Thread t3(&cond_brodcast, &mutex);// wakeup all thread
	
	t1.Start();
	t2.Start();
	t3.Start();

	t1.Join();
	t2.Join();
	t3.Join();
}

void test4()
{
	printf("\ntest4() multi thread test\n");
	Mutex mutex;
	LoopHandler loop;
	
	std::vector<Thread *> ts;
	int thread_count = 4;
	for (int i = 0; i < thread_count; i++) {
		Thread *t = new Thread(&loop, &mutex);
		char c[32];
		sprintf(c, "No.%d", i);
		t->SetName(c);
		ts.push_back(t);
	}

	for (int i = 0; i < 3; i++) {
		printf("start all threads\n");
		for (int i = 0; i < thread_count; i++) {
			ts[i]->Start();
		}

		Thread::MilliSleep(1);
		printf("cancel all threads\n");
		for (int i = 0; i < thread_count; i++) {
			ts[i]->Cancel();
		}

		printf("join all threads\n");
		for (int i = 0; i < thread_count; i++) {
			ts[i]->Join();
		}
		printf("all thread stopped\n\n");
	}

	printf("delete all threads\n");
	for (int i = 0; i < thread_count; i++) {
		delete ts[i];
	}

	ts.clear();
}


struct Test5
{
	Mutex *mutex;
	bool isYieldEnd;
	bool start;
};

class Test5Handler : public IRunnable
{
public:
	Test5Handler(std::string say)
		: message(say)
	{
	}

	void Run()
	{
		Thread *t = Thread::CurrentThread();
		Test5 *p = static_cast<Test5*>(t->Parameter());
		Mutex *mutex = p->mutex;
		bool isYieldEnd = p->isYieldEnd;

		while (!p->start) {
			Thread::MilliSleep(1);
		}

		for (int i = 0; i < 30; i++) {
			MutexLock lock(mutex, isYieldEnd);
			::printf("%s\n",message.c_str());
		}
	}

private:
	std::string message;

};

void test5(bool isYieldEnd)
{
	printf("\ntest5(%s)\n", isYieldEnd?"true":"false");
	Test5Handler handlerHello("Hello");
	Test5Handler handlerWorld("		World");
	Mutex mutex;

	Test5 param;
	param.mutex = &mutex;
	param.isYieldEnd = isYieldEnd;
	param.start = false;

	Thread t1(&handlerHello, &param);
	Thread t2(&handlerWorld, &param);;

	t1.Start();
	t2.Start();

	Thread::MilliSleep(500);
	param.start = true;
	t1.Join();
	t2.Join();
}

int main(int argc, char *argv[]) {
	test1();
	test2();
	test3();
	test4();
	test5(false); // ## true or false. check the difference !!
	test5(true); // ## true or false. check the difference !!

	return 0;
}
