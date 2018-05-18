#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "UnixDomainSocketClient.h"
#include "UnixDomainSocketServer.h"
#include "Thread.h"

using namespace PicoIPC;

class RequestReceiver : public IRequestReceiver
{
public:
	void Received(ByteBuffer &request, ByteBuffer &response)
	{
		time_t t = ::time(NULL);
		response.Append("time:[%s]", ::ctime(&t));
	}
};

class StopServerHandler : public IRunnable
{
public:
	void Run()
	{
		Thread *t = Thread::CurrentThread();
		printf("server_stop()\n");
		UnixDomainSocketServer *server = static_cast<UnixDomainSocketServer*>(t->Parameter());

		Thread::Sleep(10); // wait for client start

		printf("the server will stop after 10 sec\n");
		ByteBuffer notify;
		notify.Append("the server will stop after 10 sec");
		server->Notify(notify);
		Thread::Sleep(10);
		server->Stop();
	}
};

class NotifyReceiver : public INotifyReceiver
{
public:
	void ReceiveNotify(ByteBuffer &update)
	{
		printf("notify\n");
		update.Print();
	}
};

int main(int argc, char *argv[]) {
	::printf("[%lu] start main thread]\n", Thread::CurrentThreadId());
	if (argc > 1) {
		// server
		UnixDomainSocketServer server("/tmp/PicoIPC_uds"); 
		RequestReceiver receiver;
		server.SetReceiver(&receiver);

		StopServerHandler stopper;
		Thread t1(&stopper, &server);
		t1.Start();

		server.Start(true); // block
		t1.Join();
	} else {
		// client
		// client is non block only
		UnixDomainSocketClient client("/tmp/PicoIPC_uds");
		NotifyReceiver notifyReceiver;
		client.SetNotifyReceiver(&notifyReceiver);

		ByteBuffer req;
		ByteBuffer res;

		Error err = client.SendReceive(req,res);
		if (err) {
			printf("response error :%s\n",err.Message().c_str());
		} else {
			res.Print();
			std::string currentTime;
			res.Value(currentTime);
			printf("%s\n",currentTime.c_str());
		}
		Thread::Sleep(10);
		printf("end client\n");
	}

	printf("ending ... sleep 3 sec\n");
	Thread::Sleep(3);
	return 0;
}
