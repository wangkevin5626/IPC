#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "SharedMemoryContext.h"
#include "SharedMemory.h"
#include "SharedLock.h"

using namespace PicoIPC;

struct SMTest
{
    int a;
    int b;
    char c[10];
};

void test1(bool isOwner, SharedMemory *shm1)
{
    ::printf("\ntest1\n");
    // auto lock
    {
        if (isOwner) {
            SharedLock<SMTest> l(shm1); // locked
            l->a++;
            l->b = 456;
            ::strcpy(l->c,"Hello");
            ::printf("%d, %d %s\n",l->a, l->b, l->c);
            ::sleep(5);
            l->a++;
            l->b = 123;
            ::strcpy(l->c,"World");
            ::printf("%d, %d %s\n",l->a, l->b, l->c);
        } else {
            SharedLock<SMTest> l(shm1); // locked
            ::printf("%d, %d %s\n",l->a, l->b, l->c);
            ::sleep(5);
            ::printf("%d, %d %s\n",l->a, l->b, l->c);
        }
    }
}

void test2(bool isOwner, SharedMemory *shm1)
{
    ::printf("\ntest2\n");
    ::sleep(5);
    // manual lock
    {
        if (isOwner) {
            shm1->Wait(); // locked
            SMTest *l = shm1->Data<SMTest>();
            l->a++;
            l->b = 456;
            ::strcpy(l->c,"Hello");
            ::printf("%d, %d %s\n",l->a, l->b, l->c);
            ::sleep(5);
            l->a++;
            l->b = 123;
            ::strcpy(l->c,"World");
            ::printf("%d, %d %s\n",l->a, l->b, l->c);
            shm1->Post(); // unlocked
        } else {
            shm1->Wait(); // locked
            SMTest *l = shm1->Data<SMTest>();
            ::printf("%d, %d %s\n",l->a, l->b, l->c);
            ::sleep(5);
            ::printf("%d, %d %s\n",l->a, l->b, l->c);
            shm1->Post(); // unlocked
        }
    }
}

int main(int argc, char *argv[]) {
    bool isOwner = (argc > 1);
	SharedMemoryContext context;
	SharedMemory *shm1 = context.Bind<SMTest>("/shm1", isOwner);

    test1(isOwner, shm1);
    test2(isOwner, shm1);

    return 0;
}
