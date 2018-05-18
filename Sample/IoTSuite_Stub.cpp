#include <iostream>
#include <fstream>
#include <string>

#include "SharedMemoryContext.h"
#include "SharedLock.h"
#include "MySharedData.h"
#include "UnixDomainSocketServer.h"
#include "Thread.h"
#include "MessageQueue.h"

using namespace PicoIPC;

class RequestReceiverImpl : public IRequestReceiver
{
public:
	void Received(ByteBuffer &request, ByteBuffer &response)
	{
		::printf("receive data\n");
		int id;
		request.Value(id);
		if (id == 10) {
			response.Append("program upload response");
		}
		else if (id == 11) {
			response.Append("program download response");
		}
		else if (id == 12) {
			response.Append("program list response");
		}
		else {
			response.Append("???");
		}
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

void help()
{
	std::cout << "[show help]" << std::endl;
	std::cout << "-- input ([n] : short cut command) --" << std::endl;
	std::cout << "[q]uit     : quit" << std::endl;
	std::cout << "[p]ing     : ping" << std::endl;
	std::cout << "sh[m]      : show shared memory" << std::endl;
	std::cout << "axis [on]  : notify axis on to controller" << std::endl;
	std::cout << "axis [off] : notify axis off to controller" << std::endl;
	std::cout << "[h]elp     : this message" << std::endl;
}

void ping(UnixDomainSocketServer &server)
{
	Error err = server.Ping();
	if (err) {
		printf("ping error\n");
	} else {
		printf("ping ok\n");
	}
}

void show_shared_memory(SharedMemory *shm)
{
	std::cout << "[show shared memory]" << std::endl;
	// lock
	SharedLock<SharedArea1> l(shm);
	::printf("system info\n");
	::printf("%s\n",l->ver.SystemVersion().c_str());
	::printf("power on:%d\n",l->sys.power_on);
	::printf("%d\n",l->sys.time);
	::printf("\n");
}

class AxisLogger : public IRunnable
{
public:
	AxisLogger(MessageQueue *mq)
		: mMQ(mq)
		, mIsActive(false)
	{
		mFile.open("axis_list.dat", std::fstream::out | std::fstream::app);
	}

	void Run()
	{
		int counter;
		double axis;
		ByteBuffer bb;
		while (true) {
			if (mIsActive) {
#if 0
				Error e = mMQ->TimedReceive(bb, 500);
				if (!e) {
					bb.Value(counter);
					mFile << counter;
					for (int j = 0; j < 33; j++) {
						bb.Value(axis);
						mFile << " " << axis;
					}
					mFile << std::endl;
				} else {
					printf("err:%s\n",e.Message().c_str());
				}
				Thread::MilliSleep(5);
#else
				std::vector<ByteBuffer> list;
				Error e = mMQ->Receive(list);
				if (!e) {
					for (size_t i = 0; i < list.size(); i++) {
						ByteBuffer &b = list[i];
						b.Value(counter);
						mFile << counter;
						for (int j = 0; j < 33; j++) {
							b.Value(axis);
							mFile << " " << axis;
						}
						mFile << std::endl;
					}
				} else {
					printf("err:%s\n",e.Message().c_str());
				}
				Thread::MilliSleep(100);
#endif
			} else {
				mMutex.Lock();
				mMutex.ConditionWait();
				mMutex.Unlock();
			}
		}
	}

	void Cleanup()
	{
		mFile.close();
	}

	void on()
	{
		printf("write to axis_list.dat\n");

		mMutex.Lock();
		mIsActive = true;
        mMutex.ConditionSignal();
        mMutex.Unlock();
	}

	void off()
	{
		mMutex.Lock();
		mIsActive = false;
        mMutex.Unlock();
	}

private:
	MessageQueue *mMQ;
	bool mIsActive;
	std::fstream mFile;
	Mutex  mMutex;
};


void notify_axis_on(UnixDomainSocketServer &server, AxisLogger &logger)
{
	std::cout << "[notify axis on]" << std::endl;
	ByteBuffer bb;
	int id = 1;
	bool v = true;
	bb.Append(id);
	bb.Append(v);
	server.Notify(bb);

	logger.on();
}

void notify_axis_off(UnixDomainSocketServer &server, AxisLogger &logger)
{
	std::cout << "[notify axis off]" << std::endl;
	ByteBuffer bb;
	int id = 1;
	bool v = false;
	bb.Append(id);
	bb.Append(v);
	server.Notify(bb);

	logger.off();
}

int main(int argc, char *argv[])
{
	// IoT suite side

	UnixDomainSocketServer server("/tmp/PicoIPC_uds");
	RequestReceiverImpl receiver;

	SharedMemoryContext context;
	SharedMemory *shm;
	MessageQueue mq("/mq1");
	AxisLogger logger(&mq);
	Thread t(&logger, NULL);


	// initialize
	{
		server.SetReceiver(&receiver);
		server.Start(false); // non block
		shm = context.Bind<SharedArea1>("/panda_shared_memory", false);
		if (shm == NULL) {
			std::cout << "shared memory not found" << std::endl;
			return 1;
		}
		t.Start();
	}
	
	// main loop
	{
		help();
		while (true) {
			std::string line;
			std::getline(std::cin, line);
			if (line == "quit" || line == "q") {
				break;
			}
			else if (line == "ping" || line == "p") {
				ping(server);
			}
			else if (line == "help" || line == "h") {
				help();
			}
			else if (line == "shm" || line == "m") {
				show_shared_memory(shm);
			}
			else if (line == "axis on" || line == "on") {
				notify_axis_on(server, logger);
			}
			else if (line == "axis off" || line == "off") {
				notify_axis_off(server, logger);
			}
			else {
				std::cout << "input 'help' to message" << std::endl;
			}
		}
	}

	// finalize
	{
		t.Cancel();
		t.Join();
	}

	return 0;
}
