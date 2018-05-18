#include <stdio.h>
#include <unistd.h>
#include <vector>
#include "Mutex.h"
#include "Thread.h"

using namespace PicoIPC;

class SleepHandler : public IRunnable
{
public:
    void Run()
    {
        Thread *t = Thread::CurrentThread();
        Mutex *mutex = static_cast<Mutex *>(t->Parameter());
        ::printf("[%lu] wait lock\n", t->ThreadId());
        mutex->Lock();
        ::printf("[%lu] locked\n", t->ThreadId());
        Thread::Sleep(3);
        ::printf("[%lu] wakeup\n", t->ThreadId());
        mutex->Unlock();
    }
};

class Test2CondWaitHandler : public IRunnable
{
public:
    void Run()
    {
        Thread *t = Thread::CurrentThread();
        ::printf("[%lu] start cond_wait_handler\n", t->ThreadId());
        Mutex *mutex = static_cast<Mutex *>(t->Parameter());
        ::printf("[%lu] wait lock\n", t->ThreadId());
        mutex->Lock();
        ::printf("[%lu] locked\n", t->ThreadId());
        ::printf("[%lu] conditionWait\n", t->ThreadId());
        mutex->ConditionWait();
        ::printf("[%lu] wakeup\n", t->ThreadId());
        mutex->Unlock();
    }
};

class Test2CondSignalHandler : public IRunnable
{
public:
    void Run()
    {
        Thread::Sleep(3);
        Thread *t = Thread::CurrentThread();
        ::printf("[%lu] start cond_signal_handler\n", t->ThreadId());
        Mutex *mutex = static_cast<Mutex *>(t->Parameter());
        ::printf("[%lu] wait lock\n", t->ThreadId());
        mutex->Lock();
        ::printf("[%lu] locked\n", t->ThreadId());
        ::printf("[%lu] conditionSignal\n", t->ThreadId());
        mutex->ConditionSignal();
        mutex->Unlock();
    }
};

class Test2CondBroadcastHandler : public IRunnable
{
public:
    void Run()
    {
        Thread::Sleep(3);
        Thread *t = Thread::CurrentThread();
        ::printf("[%lu] start cond_brodcast_handler\n", t->ThreadId());
        Mutex *mutex = static_cast<Mutex *>(t->Parameter());
        ::printf("[%lu] wait lock\n", t->ThreadId());
        mutex->Lock();
        ::printf("[%lu] locked\n", t->ThreadId());
        ::printf("[%lu] conditionBroadcast\n", t->ThreadId());
        mutex->ConditionBroadcast();
        mutex->Unlock();
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
            mutex->Lock();
            ::printf("[%lu] %s\n", t->ThreadId(), t->Name().c_str());
            Thread::MilliSleep(100);

            // A deadlock will occur if Thread::Cancel() is done before Mutex::Unlock()
            // Therefore, Mutex::Unlock() with the ICleanupFunc function or IRunnable::cleanup()
            mutex->Unlock();

            // the calling thread to relinquish the CPU
            Thread::Yield();
        }
    }

    void Cleanup()
    {
        Thread *t = Thread::CurrentThread();
        Mutex *mutex = static_cast<Mutex *>(t->Parameter());
        ::printf("[%lu] %s unlock\n", t->ThreadId(), t->Name().c_str());
        mutex->Unlock();
    }
};


void test1()
{
    printf("\ntest1() lock/unlock test\n");
    Mutex mutex;
    SleepHandler handler;

    Thread t1(&handler, &mutex);
    Thread t2(&handler, &mutex);

    t1.Start();
    t2.Start();

    t1.Join();
    t2.Join();
}

void test2()
{
    printf("\ntest2() wait/signal test\n");
    Mutex mutex;
    Test2CondWaitHandler cond_wait_handler;
    Test2CondSignalHandler cond_signal_handler;
    
    Thread t1(&cond_wait_handler, &mutex);
    Thread t2(&cond_signal_handler, &mutex);

    t1.Start();
    t2.Start();

    t1.Join();
    t2.Join();
}

void test3()
{
    printf("\ntest3() wait/broadcast test\n");
    Mutex mutex;
    Test2CondWaitHandler cond_wait_handler;
    Test2CondBroadcastHandler cond_brodcast_handler;
    
    Thread t1(&cond_wait_handler, &mutex);
    Thread t2(&cond_wait_handler, &mutex);
    Thread t3(&cond_brodcast_handler, &mutex);

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
    LoopHandler loop_handler;
    
    std::vector<Thread *> ts;
    int thread_count = 10;
    for (int i = 0; i < thread_count; i++) {
        Thread *t = new Thread(&loop_handler, &mutex);
        char c[32];
        sprintf(c, "No.%d", i);
        t->SetName(c);
        ts.push_back(t);
    }

    printf("start all threads\n");
    for (int i = 0; i < thread_count; i++) {
        ts[i]->Start();
    }

    Thread::Sleep(2);
    printf("cancel all threads\n");
    for (int i = 0; i < thread_count; i++) {
        ts[i]->Cancel();
    }

    printf("join all threads\n");
    for (int i = 0; i < thread_count; i++) {
        ts[i]->Join();
    }

    printf("delete all threads\n");
    for (int i = 0; i < thread_count; i++) {
        delete ts[i];
    }

    // main thread can Mutex::lock() if all threads have no mutex
    printf("test lock\n");
    mutex.Lock();
    printf("ok\n");
    mutex.Unlock();
    ts.clear();
}

int main(int argc, char *argv[]) {
    test1();
    test2();
    test3();
    test4();

    return 0;
}
