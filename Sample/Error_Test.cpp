#include <stdio.h>
#include "Error.h"

using namespace PicoIPC;

void test1()
{
	Error err = Error::createNoError();
	if (err) {
		printf("error has [%s]\n", err.Message().c_str());
	} else {
		printf("no error [%s]\n", err.Message().c_str());
	}
}

void test2()
{
	Error err = Error::createError("File Not Found:%s count:%d", "/file/path", 3);
	if (err) {
		printf("error has [%s]\n", err.Message().c_str());
	} else {
		printf("no error [%s]\n", err.Message().c_str());
	}
}

int main(int argc, char *argv[]) {
	test1();
	test2();

	return 0;
}
