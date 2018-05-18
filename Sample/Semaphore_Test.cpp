#include <stdio.h>
#include <unistd.h>
#include "Semaphore.h"

using namespace PicoIPC;

bool test0()
{
	::printf("\ntest0 semaphore check\n");
	Error error = Semaphore::Exist("/sem1");
	if (error) {
		::printf("not found: %s\n",error.Message().c_str());
	} else {
		::printf("found\n");
	}
	return !error;
}

void test1(Semaphore &sem1, Semaphore &sem2)
{
	::printf("\ntest1 \n");
	::printf("sem 1 locked...");
	::fflush(stdout);
	sem1.Wait();
	printf("done\n");

	::sleep(3);

	::printf("sem 2 locked...");
	::fflush(stdout);
	sem2.Wait();
	::printf("done\n");

	::sleep(3);

	::printf("sem 1 unlocked...");
	::fflush(stdout);
	sem1.Post();
	::printf("done\n");

	::sleep(3);

	::printf("sem 2 unlocked...");
	::fflush(stdout);
	sem2.Post();
	::printf("done\n");
}

void test2(Semaphore &sem1, Semaphore &sem2)
{
	::printf("\ntest2 \n");
	::printf("sem 1 locked...");
	::fflush(stdout);
	sem1.Wait();
	::printf("done\n");

	::sleep(3);

	::printf("sem 2 locked...");
	::fflush(stdout);
	sem2.Wait();
	::printf("done\n");

	::sleep(3);

	::printf("sem 2 unlocked...");
	::fflush(stdout);
	sem2.Post();
	::printf("done\n");

	::sleep(3);

	::printf("sem 1 unlocked...");
	::fflush(stdout);
	sem1.Post();
	::printf("done\n");
}

int main(int argc, char *argv[]) {
	bool isOwner = (argc > 1);
	if (!isOwner && !test0()) {
		return 0;
	}

	Semaphore sem1("/sem1", isOwner);
	Semaphore sem2("/sem2", isOwner);
	
	test1(sem1, sem2);
	test2(sem1, sem2);

	return 0;
}
