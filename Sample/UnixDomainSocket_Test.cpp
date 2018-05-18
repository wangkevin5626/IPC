#include <stdio.h>
#include <unistd.h>
#include "UnixDomainSocketClient.h"
#include "UnixDomainSocketServer.h"
#include "Thread.h"

using namespace PicoIPC;


class RequestReceiverImpl : public IRequestReceiver
{
public:
	void Received(ByteBuffer &request, ByteBuffer &response)
	{
		int a;
		int b;
		int c;
		request.Value(a);
		request.Value(b);
		request.Value(c);
		int result;
		switch (c) {
		case 0: result = a + b; break;
		case 1: result = a - b; break;
		case 2: result = a * b; break;
		default: result = a + b; break;
		}
		response.Append(result);
	}

	void ReceiveError(const Error &error)
	{
		::printf("receive error # %s\n",error.Message().c_str());
	}

	void ResponseError(const Error &error)
	{
		::printf("response error # %s\n",error.Message().c_str());
	}
};


static bool start_client_request = false;
class NotifyReceiverImpl : public INotifyReceiver
{
public:
	void ReceiveNotify(ByteBuffer &update)
	{
		std::string message;
		int  count;
		bool active;
		update.Value(message);
		update.Value(count);
		update.Value(active);
		printf("update message size:[%zu] count=%d active=%s\n", update.Size(), count, (active ? "true":"false"));
		start_client_request = active;
	}
};


class NofifyHandler : public IRunnable
{
public:
	void Run()
	{
		Thread *t = Thread::CurrentThread();
		::printf("[%lu] start server(update) wait for 10 sec]\n", t->ThreadId());
		Thread::Sleep(10);

		UnixDomainSocketServer *server = static_cast<UnixDomainSocketServer *>(t->Parameter());
		size_t size = 1024*1024*32; //32Mbyte
		char *buf = new char[size];
		memset(buf, ' ',size);
		int notify_count = 10;
		for (int i = 0; i < notify_count; i++) {
			ByteBuffer update;
			std::string message(buf, size);
			message[0] = 'y';
			message[1] = 'e';
			message[2] = 's';
			message[size - 1] = 'z';
			int  count = i;
			bool active = ((notify_count - 1) != i);
			update.Append(message);
			update.Append(count);
			update.Append(active);
			::printf("%d notify size:%zu\n", i, update.Size());
			Error err = server->Notify(update);
			if (err) {
				::printf("notify error # %s\n", err.Message().c_str());
			}
		}
		delete [] buf;
	}

	void Cleanup()
	{
		Thread *t = Thread::CurrentThread();
		::printf("[%lu] Cleanup NofifyHandler\n", t->ThreadId());
	}
};

class RequestHandler : public IRunnable
{
public:
	void Run()
	{
		Thread *t = Thread::CurrentThread();
		UnixDomainSocketClient *client = static_cast<UnixDomainSocketClient *>(t->Parameter());
		::printf("[%lu] wait for notify from server to start client request]\n", t->ThreadId());
		while (!start_client_request) {
			printf(".");
			fflush(stdout);
			Thread::MilliSleep(100);
		}
		::printf("\n## start client request\n");

		int i = 0;
		while (start_client_request) {
			ByteBuffer req;
			ByteBuffer res;
			int a = 5;
			int b = 7;
			int c = i++%3; // 0:+, 1:-, 2:*
			req.Append(a);
			req.Append(b);
			req.Append(c);
			Error err = client->SendReceive(req,res);
			if (err) {
				printf("error # %s\n", err.Message().c_str());
			} else {
				int d;
				res.Value(d);
				printf("%d result:[%d]\n", i,d);
			}
			// Thread::Yield();
		}
		::printf("[%lu] end client(request)]\n", t->ThreadId());
	}

	void Cleanup()
	{
		Thread *t = Thread::CurrentThread();
		::printf("[%lu] Cleanup RequestHandler\n", t->ThreadId());
	}
};

class ServerStopper : public IRunnable
{
public:
	void Run()
	{
		Thread *t = Thread::CurrentThread();
		UnixDomainSocketServer *server = static_cast<UnixDomainSocketServer *>(t->Parameter());
		Thread::Sleep(3);
		::printf("[%lu] start ServerStopper\n", t->ThreadId());
		server->Stop();
		::printf("[%lu] end ServerStopper\n", t->ThreadId());
	}
};

int main(int argc, char *argv[]) {
	::printf("[%lu] start main thread]\n", Thread::CurrentThreadId());
	if (argc > 1) {
		// server
		UnixDomainSocketServer server("/tmp/PicoIPC_uds");
		RequestReceiverImpl impl;

		server.SetLimitSize(0);
		server.SetReceiver(&impl);

		// PING test
		{
			Error err = server.Ping();
			if (err) {
				printf("ping error # %s\n", err.Message().c_str());
			} else {
				printf("ping ok\n");
			}
		}

#if 0
		ServerStopper stopper;
		Thread t_stop(&stopper, &server);
		t_stop.Start();
		bool isBlock = (argc == 2 ? false : true);
		if (isBlock) {
			::printf("start server [foreground]\n");
			server.Start(true);
			::printf("server is end\n");
			t_stop.Join();
		} else {
			::printf("start server [background]\n");
			server.Start(false);
			::printf("server is started\n");
			t_stop.Join();
			::printf("server is end\n");
		}
#else
		server.Start(false); // non block
		NofifyHandler nofify;
		Thread t_notify(&nofify, &server);
		t_notify.SetName("NofifyHandler");
		t_notify.Start();
		t_notify.Join();
#endif
		::printf("end server thread\n");
	} else {
		// client
		UnixDomainSocketClient client("/tmp/PicoIPC_uds");
		NotifyReceiverImpl impl;

		client.SetLimitSize(0);
		client.SetNotifyReceiver(&impl);
		
		// PING test
		{
			Error err = client.Ping();
			if (err) {
				printf("ping error # %s\n", err.Message().c_str());
			} else {
				printf("ping ok\n");
			}
		}

		RequestHandler request;
		Thread t1(&request, &client);
		t1.SetName("RequestHandler");
		t1.Start();
		t1.Join();
		::printf("end client thread\n");
	}
	return 0;
}
