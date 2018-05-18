#include <iostream>
#include <string>
#include <cmath>
#include <stdlib.h>

#include "SharedMemoryContext.h"
#include "SharedLock.h"
#include "MySharedData.h"
#include "UnixDomainSocketClient.h"
#include "Thread.h"
#include "MessageQueue.h"

using namespace PicoIPC;

static bool axis_on = false;
static int      user_argc;
static char   **user_argv;

class NotifyReceiver : public INotifyReceiver
{
public:
    void ReceiveNotify(ByteBuffer &update)
    {
        printf("notify\n");
//		update.Print();
        int id;
        update.Value(id);
        if (id == 1) {
            update.Value(axis_on);
            printf("axis %s\n", axis_on ? "on":"off");
        }
    }
};

class AxisAngleWoker : public IRunnable
{
public:
    void Run()
    {
        Thread *t = Thread::CurrentThread();
        MessageQueue *mq = static_cast<MessageQueue*>(t->Parameter());
        ::srand(::time(NULL));
        int counter = 0; 
        while (true) {
            if (!axis_on) {
                Thread::MilliSleep(100);
                continue;
            }

            ByteBuffer bb;
            bb.Append(counter++%100);
            for (int j = 0; j < 33; j++) {
                bb.Append(static_cast<double>(rand())/RAND_MAX*2.0-1.0);
            }
            mq->TimedSend(bb, 10);
            Thread::MilliSleep(10);
        }
    }
};

void help()
{
    std::cout << "[show help]" << std::endl;
    std::cout << "-- input ([n] : short cut command) --" << std::endl;
    std::cout << "[q]uit     : quit" << std::endl;
    std::cout << "[p]ing     : ping" << std::endl;
    std::cout << "req [10]   : upload program" << std::endl;
    std::cout << "req [11]   : download program" << std::endl;
    std::cout << "req [12]   : program list" << std::endl;
    std::cout << "[h]elp     : this message" << std::endl;
}

void ping(UnixDomainSocketClient &client)
{
    Error err = client.Ping();
    if (err) {
        printf("ping error\n");
    } else {
        printf("ping ok\n");
    }
}

void request_upload(UnixDomainSocketClient &client)
{
    ByteBuffer req;
    ByteBuffer res;
    int id = 10;
    req.Append(id);

    Error err = client.SendReceive(req,res);
    if (err) {
        printf("error # %s\n", err.Message().c_str());
    } else {
        std::string response;
        res.Value(response);
        printf("response:[%s]\n", response.c_str());
    }
}

void request_download(UnixDomainSocketClient &client)
{
    ByteBuffer req;
    ByteBuffer res;
    int id = 11;
    req.Append(id);

    Error err = client.SendReceive(req,res);
    if (err) {
        printf("error # %s\n", err.Message().c_str());
    } else {
        std::string response;
        res.Value(response);
        printf("response:[%s]\n", response.c_str());
    }
}

void request_programlist(UnixDomainSocketClient &client)
{
    ByteBuffer req;
    ByteBuffer res;
    int id = 12;
    req.Append(id);

    Error err = client.SendReceive(req,res);
    if (err) {
        printf("error # %s\n", err.Message().c_str());
    } else {
        std::string response;
        res.Value(response);
        printf("response:[%s]\n", response.c_str());
    }
}

// named shared memories and named semaphores are managed controller process side
int main(int argc, char *argv[])
{
    user_argc = argc;
    user_argv = argv;
    UnixDomainSocketClient client("/tmp/PicoIPC_uds");
    NotifyReceiver notifyReceiver;

    SharedMemoryContext context;
    SharedMemory *shm;
    MessageQueue mq("/mq1", 1000, 400);


    // initialize
    {
        client.SetNotifyReceiver(&notifyReceiver);
        shm = context.Bind<SharedArea1>("/panda_shared_memory", true);

        // lock
        {
            SharedLock<SharedArea1> l(shm); // locked
            l->ver.SetSystemVersion("4.20.1"); // same as	::strcpy(l->sh1.ver.system_version,"4.20.1");
            l->sys.power_on++;
            l->sys.time = 456;
            l->sys.serve_on = true;
            // automatically unlocked by SharedLock's destructor
        }
    }
    
    AxisAngleWoker worker;
    Thread t(&worker, &mq);
    t.Start();

    // main loop
    {
        /*help();
        while (true) {
            std::string line;
            std::getline(std::cin, line);
            if (line == "quit" || line == "q") {
                t.Cancel();
                t.Join();
                break;
            }
            else if (line == "ping" || line == "p") {
                ping(client);
            }
            else if (line == "help" || line == "h") {
                help();
            }
            else if (line == "req 10" || line == "10") {
                request_upload(client);
            }
            else if (line == "req 11" || line == "11") {
                request_download(client);
            }
            else if (line == "req 12" || line == "12") {
                request_programlist(client);
            }
            else {
                std::cout << "input 'help' to message" << std::endl;
            }         
        }*/
        while(true){
            if(user_argc>=2){
                if(!strcmp("quit",user_argv[1]) || !strcmp("q",user_argv[1])){
                    printf("quit the console\n");
                    /*t.Cancel();
                    t.Join();*/
                    break;
                }
		if(!strcmp("iot",user_argv[1])){
                    printf("iot registered\n");
                    break;
                }
                if(!strcmp("2",user_argv[1])){
                    printf("get system time\n");
                    break;
                }
		            
            }
        }
    }

    return 0;
}
