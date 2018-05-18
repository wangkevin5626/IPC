#include <stdio.h>
#include <unistd.h>
#include <vector>

#include "Thread.h"
#include "Mutex.h"

using namespace PicoIPC;

class Test0Woker : public IRunnable
{
public:
	void Run()
	{
		Thread *t = Thread::CurrentThread();
		::printf("[%lu] start Test0Woker\n", t->ThreadId());
		Thread::Sleep(3);
		::printf("[%lu] end Test0Woker\n", t->ThreadId());
	}

	void Cleanup()
	{
		Thread *t = Thread::CurrentThread();
		::printf("[%lu] cleanup Test0Woker\n", t->ThreadId());
	}
};


void test0()
{
	::printf("\ntest0() \n");
	Test0Woker worker;
	Thread t(&worker, NULL);

	for (int i = 0; i < 5; i++) {
		::printf("count:%d\n",i);
		t.Start();
		t.Start(); // do nothing
		if (i%2) {
			Thread::MilliSleep(500);
			::printf("cancel\n");
			t.Cancel();
			t.Cancel(); // do nothing
		}
		t.Join();
		t.Join(); // do nothing
		::printf("cancel 2\n");
		t.Cancel();
		t.Cancel(); // do nothing
		::printf("\n");

	}
}


class RunnableImpl : public IRunnable
{
public:
	void Run()
	{
		Thread *t = Thread::CurrentThread();
		::printf("[%lu] start %s\n", t->ThreadId(), t->Name().c_str());
		// loop 10 sec
		for (int i = 0; i < 10; i++) {
			::printf("[%lu] RunnableImpl [%s]\n", t->ThreadId(), t->Name().c_str());
			Thread::Sleep(1);
		}
		::printf("[%lu] end %s\n", t->ThreadId(), t->Name().c_str());
	}
};

class ExtendThread : public Thread
{
public:
	ExtendThread()
		: Thread()
	{
		SetName("ExtendThread");
	}

	void Execute()
	{
		Thread *t = Thread::CurrentThread();
		::printf("[%lu] start %s\n", t->ThreadId(), t->Name().c_str());
		Thread::Sleep(1);
		::printf("[%lu] end %s\n", t->ThreadId(), t->Name().c_str());
	}
};

void test1()
{
	::printf("\ntest1() \n");
	Mutex mutex;
	RunnableImpl impl;

	std::vector<Thread *> threads;
	Thread t0(&impl, &mutex);
	Thread t1(&impl, &mutex);
	Thread *t2 = new Thread(&impl, &mutex);
	Thread *t3 = new ExtendThread();

	threads.push_back(&t0);
	threads.push_back(&t1);
	threads.push_back(t2);
	threads.push_back(t3);

	t0.SetName("RunnableImpl 0");
	t1.SetName("RunnableImpl 1");
	t2->SetName("RunnableImpl 2");

	printf("\njoin test\n");
	{
		printf("start threads\n");
		for (size_t i = 0; i < threads.size(); i++) {
			threads[i]->Start();
		}

		printf("join threads (wait for 10 secs)\n");
		for (size_t i = 0; i < threads.size(); i++) {
			threads[i]->Join();
		}
	}

	printf("\ncancel test\n");
	{
		printf("start threads\n");
		for (size_t i = 0; i < threads.size(); i++) {
			threads[i]->Start();
		}

		Thread::Sleep(1);
		printf("cancel threads\n");
		for (size_t i = 0; i < threads.size(); i++) {
			threads[i]->Cancel();
		}

		printf("join threads\n");
		for (size_t i = 0; i < threads.size(); i++) {
			threads[i]->Join();
		}
	}

	delete t2;
	delete t3;
}


class Test2Handler : public IRunnable
{
public:
	void Run()
	{
		Thread *t = Thread::CurrentThread();
		::printf("[%lu] start Test2Handler\n", t->ThreadId());
		Thread::Sleep(10);
	}
};

void test2()
{
	::printf("\ntest1() check thread active status\n");
	Thread *nullThread = Thread::CurrentThread();
	printf("main thread is not Thread instance. pointer:%p\n",nullThread);

	Test2Handler test2Handler;

	Thread t(&test2Handler, NULL);
	printf("1 tid %lu active:%d\n",t.ThreadId(), t.IsActive());
	t.Start();
	printf("2 tid %lu active:%d\n",t.ThreadId(), t.IsActive());
	t.Start(); // already started, so don't called Run()
	printf("3 tid %lu active:%d\n",t.ThreadId(), t.IsActive());
	Thread::Sleep(1);
	printf("4 tid %lu active:%d\n",t.ThreadId(), t.IsActive());
	t.Cancel();
	printf("5 tid %lu active:%d\n",t.ThreadId(), t.IsActive());
	t.Join();
	printf("6 tid %lu active:%d\n",t.ThreadId(), t.IsActive());
}

class Test3Handler : public IRunnable
{
public:
	void Run()
	{
		Thread *t = Thread::CurrentThread();
		int *sleepTime = reinterpret_cast<int *>(t->Parameter());
		::printf("[%lu] start Test3Handler sleep time:%d\n", t->ThreadId(), *sleepTime);
		Thread::Sleep(*sleepTime);
		delete sleepTime;
	}

	void Cleanup()
	{
		Thread *t = Thread::CurrentThread();
		::printf("[%lu] cleanup\n", t->ThreadId());
	}
};

void test3()
{
	::printf("\ntest3() delete\n");

	std::vector<Thread *> threads;
	Test3Handler test3Handler;
	for (int i = 0; i < 10; i++) {
		int *param = new int(i);
		Thread *t = new Thread(&test3Handler, param);
		t->Start();
		threads.push_back(t);
	}
	
	// stop thread before destroing Thread 
	for (int i = 0; i < 10; i++) {
		Thread *t = threads.at(i);
		t->Cancel();
		t->Join();
		delete t;
	}

}

class Test4Woker : public IRunnable
{
public:
	int mCount;
	bool mYield;

	void Run()
	{
		Thread *t = Thread::CurrentThread();
		const char *message = static_cast<std::string*>(t->Parameter())->c_str();
		while (true) {
			::printf("%s\n", message);
			mCount++;
			if (mYield) {
				Thread::Yield();
			}
		}
	}

};

void test4()
{
	::printf("\ntest4() yield\n");
	Test4Woker helloWorker;
	Test4Woker worldWorker;

	std::string hello = "hello";
	std::string world = "     world";
	Thread t1(&helloWorker, &hello);
	Thread t2(&worldWorker, &world);

	helloWorker.mYield = false;
	worldWorker.mYield = false;
	helloWorker.mCount = 0;
	worldWorker.mCount = 0;

	int helloCount = 0;
	int helloCountYield = 0;
	int worldCount = 0;
	int worldCountYield = 0;
	for (int i = 0; i < 2; i++) {
		t1.Start();
		t2.Start();
		Thread::Sleep(1);
		t1.Cancel();
		t2.Cancel();
		t1.Join();
		t2.Join();

		int hc = helloWorker.mCount;
		int wc = worldWorker.mCount;
		if (i == 0) {
			helloCount = hc;
			worldCount = wc;
			helloWorker.mYield = true;
			worldWorker.mYield = true;
			helloWorker.mCount = 0;
			worldWorker.mCount = 0;
		} else {
			helloCountYield = hc;
			worldCountYield = wc;
		}
	}

	printf("default hello:%d + world:%d = total:%d\n",helloCount, worldCount, helloCount + worldCount);
	printf("yield   hello:%d + world:%d = total:%d\n",helloCountYield, worldCountYield, helloCountYield + worldCountYield);

}

void test5()
{
	::printf("\ntest5() disable copy \n");
	Thread t1;
#if 0
	Thread t2(t1); // compile error (disable copy constructor)
	Thread t3;
	t3 = t1;       // compile error (disable copy)
#endif
}

int main(int argc, char *argv[]) {
	test0();
	test1();
	test2();
	test3();
	test4();
	test5();
	return 0;
}
