#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "MessageQueue.h"
#include "Thread.h"

using namespace PicoIPC;

void test0(bool isOwner)
{
	::printf("\ntest0 message queue check\n");

	::printf("isOwner:%s\n",(isOwner?"true":"false"));
	Error error = MessageQueue::Exist("/mq1");
	if (error) {
		::printf("not found: %s\n",error.Message().c_str());
	} else {
		::printf("found\n");
	}
}

void test1(bool isOwner, MessageQueue &mq, MessageQueue &mq_sync)
{
	::printf("\ntest1 message queue attribute check\n");

	::printf("isOwner:%s\n",(isOwner?"true":"false"));
	::printf("mq name %s\n", mq.Name().c_str());
	::printf("mq MaxMessageCount %ld\n", mq.MaxMessageCount());
	::printf("mq MaxMessageSize %ld\n", mq.MaxMessageSize());
	::printf("mq CurrentMessageCount %ld\n", mq.CurrentMessageCount());

	::printf("mq_sync name %s\n", mq_sync.Name().c_str());
	::printf("mq_sync MaxMessageCount %ld\n", mq_sync.MaxMessageCount());
	::printf("mq_sync MaxMessageSize %ld\n", mq_sync.MaxMessageSize());
	::printf("mq_sync CurrentMessageCount %ld\n", mq_sync.CurrentMessageCount());
}

void test2(bool isOwner, MessageQueue &mq)
{
	::printf("\ntest2 send/receive\n");

	if (isOwner) {
		::printf("pre CurrentMessageCount %ld\n", mq.CurrentMessageCount());
		long max = mq.MaxMessageCount();
		for (int i = 0; i < max; i++) {
			ByteBuffer bb;
			bb.Append("hello %d",i);
			Error err = mq.Send(bb);
			if (err) {
				::printf("err:%s\n", err.Message().c_str());
				::exit(1);
			}
		}
		::printf("post CurrentMessageCount %ld\n", mq.CurrentMessageCount());
	} else {
		::printf("wait 200 milli sec\n");
		Thread::MilliSleep(200);
		long count = mq.CurrentMessageCount();
		::printf("CurrentMessageCount %ld\n", count);
		for (int i = 0; i < count; i++) {
			ByteBuffer bb;
			Error err = mq.Receive(bb);
			if (err) {
				::printf("err:%s\n", err.Message().c_str());
				::exit(1);
			} else {
				bb.Print();
			}
		}
	}

}

void test3(bool isOwner, MessageQueue &mq)
{
	::printf("\ntest3 send/receive 2\n");

	::printf("isOwner:%s\n",(isOwner?"true":"false"));

	if (isOwner) {
		// send message 1
		{
			ByteBuffer bb;
			bb.Append("Hello");
			Error err = mq.Send(bb);
			if (err) {
				::printf("err:%s\n", err.Message().c_str());
				::exit(1);
			} else {
				std::string send;
				bb.Value(send);
				printf("send:%s\n", send.c_str());
			}
		}

		// send message 2
		{
			ByteBuffer bb;
			bb.Append("world");
			Error err = mq.Send(bb);
			if (err) {
				::printf("err:%s\n", err.Message().c_str());
				::exit(1);
			} else {
				std::string send;
				bb.Value(send);
				printf("send:%s\n", send.c_str());
			}
		}

		// send message 3
		{
			ByteBuffer bb;
			bb.Append("last message");
			Error err = mq.Send(bb);
			if (err) {
				::printf("err:%s\n", err.Message().c_str());
				::exit(1);
			} else {
				std::string send;
				bb.Value(send);
				printf("send:%s\n", send.c_str());
			}
		}

		// send message (overflow: error)
		{
			ByteBuffer bb;
			bb.Append("Over MaxMessageCount");
			Error err = mq.TimedSend(bb, 100); // error occured 
			if (err) {
				::printf("timeout:%s\n", err.Message().c_str());
			} else {
				::exit(1);
			}
		}

		// send message (overflow: wait)
		{
			ByteBuffer bb;
			bb.Append("Over MaxMessageCount");
			Error err = mq.Send(bb); // wait 
			if (err) {
				::printf("err:%s\n", err.Message().c_str());
				::exit(1);
			} else {
				std::string send;
				bb.Value(send);
				printf("send:%s\n", send.c_str());
			}
		}
	} else {
		::printf("wait 3 sec\n");
		Thread::Sleep(3);

		// receive message 1
		{
			ByteBuffer bb;
			Error err = mq.Receive(bb);
			if (err) {
				::printf("err:%s\n", err.Message().c_str());
				::exit(1);
			} else {
				std::string recv;
				bb.Value(recv);
				printf("recv:%s\n", recv.c_str());
			}
		}
		// receive message 2
		{
			ByteBuffer bb;
			Error err = mq.Receive(bb);
			if (err) {
				::printf("err:%s\n", err.Message().c_str());
				::exit(1);
			} else {
				std::string recv;
				bb.Value(recv);
				printf("recv:%s\n", recv.c_str());
			}
		}
		// receive message 3
		{
			ByteBuffer bb;
			Error err = mq.Receive(bb);
			if (err) {
				::printf("err:%s\n", err.Message().c_str());
				::exit(1);
			} else {
				std::string recv;
				bb.Value(recv);
				printf("recv:%s\n", recv.c_str());
			}
		}

		// receive message (overflow: recv message)
		{
			ByteBuffer bb;
			Error err = mq.Receive(bb);
			if (err) {
				::printf("err:%s\n", err.Message().c_str());
				::exit(1);
			} else {
				std::string recv;
				bb.Value(recv);
				printf("recv:%s\n", recv.c_str());
			}
		}

		// receive message (no message: timeout error)
		{
			ByteBuffer bb;
			Error err = mq.TimedReceive(bb, 100);
			if (err) {
				::printf("timeout:%s\n", err.Message().c_str());
			} else {
				::exit(1);
			}
		}
	}
}

void test4(bool isOwner, MessageQueue &mq)
{
	::printf("\ntest4 send/receive 4\n");

	::printf("isOwner:%s\n",(isOwner?"true":"false"));
	::printf("mq_axis name %s\n", mq.Name().c_str());
	::printf("mq_axis MaxMessageCount %ld\n", mq.MaxMessageCount());
	::printf("mq_axis MaxMessageSize %ld\n", mq.MaxMessageSize());
	::printf("mq_axis CurrentMessageCount %ld\n", mq.CurrentMessageCount());

	if (isOwner) {
		::srand(::time(NULL));
		// send message (about 10 sec)
		::printf("[start send]\n");
		for (int i = 0; i < 1000; i++) {
			ByteBuffer bb;
			//double in range -1.0 to 1.0
			bb.Append(i);
			for (int j = 0; j < 10; j++) {
				bb.Append(static_cast<double>(rand())/RAND_MAX*2.0-1.0);
			}
			Error err = mq.TimedSend(bb, 1000);
			if (err) {
				::printf("err:%s\n", err.Message().c_str());
				::printf("stopped %d\n",i);
				break;
			}
			Thread::MilliSleep(10);
		}
		::printf("[end send]\n");
	} else {
		::printf("[start receive]\n");
		do {
			ByteBuffer bb;
			Error err = mq.TimedReceive(bb, 1000);
			//Error err = mq.Receive(bb);
			if (err) {
				::printf("stopped\n");
				break;
			}
			int no;
			double a[10];
			bb.Value(no);
			for (int i = 0; i < 10; i++) {
				bb.Value(a[i]);
			}
#if 1
			::printf(".");
			::fflush(stdout);
#else
			::printf("%4d %1.4f %1.4f %1.4f %1.4f %1.4f %1.4f %1.4f %1.4f %1.4f %1.4f\n"
					, no , a[0] , a[1] , a[2] , a[3] , a[4] , a[5] , a[6] , a[7] , a[8] , a[9]);
#endif
		} while (true);
		::printf("[end receive]\n");
	}
}

void test5(bool isOwner, MessageQueue &mq)
{
	::printf("\ntest5 send/receive 5\n");

	if (isOwner) {
		::srand(::time(NULL));
		// send message (sampling10000, sleep 1 msec)
		::printf("[start send]\n");
		for (int i = 0; i < 10000; i++) {
			ByteBuffer bb;
			//double in range -1.0 to 1.0
			bb.Append(i);
			for (int j = 0; j < 10; j++) {
				bb.Append(static_cast<double>(rand())/RAND_MAX*2.0-1.0);
			}
			Error err = mq.TimedSend(bb, 1000);
			if (err) {
				::printf("err:%s\n", err.Message().c_str());
				::printf("stopped %d\n",i);
				break;
			}
			Thread::MilliSleep(1);
		}
		::printf("[end send]\n");
	} else {
		printf("[start receive]\n");
		int sum = 0;
		do {
			Thread::MilliSleep(1000);
			std::vector<ByteBuffer> list;
			Error err = mq.Receive(list);
			if (err) {
				::printf("err:%s\n", err.Message().c_str());
				break;
			}
#if 1
			printf("receved list size=%d\n",list.size());
#else
			for (size_t i = 0; i < list.size(); i++) {
				int no;
				double a[10];
				ByteBuffer &bb = list[i];
				bb.Value(no);
				for (int i = 0; i < 10; i++) {
					bb.Value(a[i]);
				}
				::printf("%4d %1.4f %1.4f %1.4f %1.4f %1.4f %1.4f %1.4f %1.4f %1.4f %1.4f\n"
						, no , a[0] , a[1] , a[2] , a[3] , a[4] , a[5] , a[6] , a[7] , a[8] , a[9]);
			}
#endif
			sum += list.size();
		} while (sum != 10000);
		printf("[end receive]\n");
	}
}

void test6(bool isOwner, MessageQueue &mq)
{
	::printf("\ntest6 clear\n");

	if (isOwner) {
		::printf("pre CurrentMessageCount %ld\n", mq.CurrentMessageCount());
		for (int i = 0; i < 5; i++) {
			ByteBuffer bb;
			bb.Append(i);
			Error err = mq.Send(bb);
			if (err) {
				::printf("err:%s\n", err.Message().c_str());
				::printf("stopped %d\n",i);
				break;
			}
		}
		::printf("post CurrentMessageCount %ld\n", mq.CurrentMessageCount());
		mq.Clear();
		::printf("clear CurrentMessageCount %ld\n", mq.CurrentMessageCount());

		for (int i = 0; i < 3; i++) {
			ByteBuffer bb;
			bb.Append(i);
			Error err = mq.Send(bb);
			if (err) {
				::printf("err:%s\n", err.Message().c_str());
				::printf("stopped %d\n",i);
				break;
			}
		}
	} else {
		Thread::MilliSleep(1000);
		::printf("pre CurrentMessageCount %ld\n", mq.CurrentMessageCount());
		mq.Clear();
		::printf("clear CurrentMessageCount %ld\n", mq.CurrentMessageCount());
	}
}

class NotifyMessageImpl : public INotifyMessage
{
	// implements
	void FirstMessageArrived(MessageQueue &mq)
	{
		::printf("NotifyMessageImpl::ReceiveMessage q:%s\n",mq.Name().c_str());
		std::vector<ByteBuffer> list;
		Error err = mq.Receive(list);
		if (err) {
			::printf("err:%s\n", err.Message().c_str());
		} else {
			printf("receved list size=%d\n",list.size());
			for (size_t i = 0; i < list.size(); i++) {
				ByteBuffer &bb = list[i];
				int a;
				int b;
				char c;
				bb.Value(a);
				bb.Value(b);
				bb.Value(c);
				::printf("receive :%d %d %c\n",a,b,c);
			}
		}
	}

};

void test7(bool isOwner, MessageQueue &mq)
{
	::printf("\ntest7 notify interface\n");

	if (isOwner) {
		mq.Clear();
		for (int i = 0; i < 3; i++) {
			Thread::Sleep(2);
			::printf("pre CurrentMessageCount %ld\n", mq.CurrentMessageCount());
			for (int j = 0; j < 5; j++) {
				ByteBuffer bb;
				bb.Append(i);
				bb.Append(j);
				bb.Append(static_cast<char>('a' + j));
				printf("send %d %d\n",i,j);
				mq.Send(bb);
			}
			::printf("post CurrentMessageCount %ld\n", mq.CurrentMessageCount());
		}
	} else {
		NotifyMessageImpl notification;
		mq.SetNotifyMessage(&notification); // notify on
		Thread::Sleep(5); 
		mq.SetNotifyMessage(NULL); // notify off
		Thread::Sleep(5); 
	}
}

void sync(bool isOwner, MessageQueue &mq_sync)
{
	if (isOwner) {
		ByteBuffer bb;
		mq_sync.Receive(bb); // wait receive any message (size 0)
	} else {
		ByteBuffer bb;
		mq_sync.Send(bb);    // send message (size 0)
	}
}

int main(int argc, char *argv[]) {
	bool isOwner = (argc > 1);

	MessageQueue mq = (isOwner ? MessageQueue("/mq1", 3, 64) : MessageQueue("/mq1"));
	MessageQueue mq_sync = (isOwner ? MessageQueue("/mq_sync", 1, 1) : MessageQueue("/mq_sync"));
	MessageQueue mq_axis = (isOwner ? MessageQueue("/mq_axis", 500, 100) : MessageQueue("/mq_axis"));

	test0(isOwner);              sync(isOwner, mq_sync);
	test1(isOwner, mq, mq_sync); sync(isOwner, mq_sync);
	test2(isOwner, mq);          sync(isOwner, mq_sync);
	test3(isOwner, mq);          sync(isOwner, mq_sync);
	test4(isOwner, mq_axis);     sync(isOwner, mq_sync);
	test5(isOwner, mq_axis);     sync(isOwner, mq_sync);
	test6(isOwner, mq_axis);     sync(isOwner, mq_sync);
	sync(isOwner, mq_sync);
	test7(isOwner, mq_axis);     sync(isOwner, mq_sync);
	return 0;
}
