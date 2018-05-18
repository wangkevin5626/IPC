#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include "ByteBuffer.h"

using namespace PicoIPC;

struct A
{
	int a;
	double b;
	bool c;
	std::string d;
	std::vector<int> e;
	std::map<int, std::string> f;
	char g;
	char h[16];
};

void test1()
{
	printf("\ntest1()\n");

	// set initial data to struct A
	A a;
	{
		a.a = 123;
		a.b = 987.654;
		a.c = true;
		a.d = "hello world";
		for (int i = 0; i < 3; i++) {
			a.e.push_back(i);
		}
		for (int i = 0; i < 5; i++) {
			char buf[256];
			::snprintf(buf,255,"PPAP %d",i);
			std::string v(buf);
			a.f.insert(std::make_pair(i,v));
		}
		a.g = 'c';
		strcpy(a.h,"::strcpy()");
	}

	// copy struct A to ByteBuffer (Serialize)
	ByteBuffer f;
	{
		f.Append(a.a);
		f.Append(a.b);
		f.Append(a.c);
		f.Append(a.d);
		f.Append(a.e); // Directly Append() when std::vector element is primitive type
		f.Append(a.f); // Directly Append() when std::map element(key,value) is primitive type
		f.Append(a.g);
		f.Append(a.h);
	}
	f.Print();

	// copy ByteBuffer to struct A (Deserialize)
	A z;
	{
		f.SetPosition(0);
		f.Value(z.a);
		f.Value(z.b);
		f.Value(z.c);
		f.Value(z.d);
		f.Value(z.e);
		f.Value(z.f);
		f.Value(z.g);
		f.Value(z.h);
	}
	printf("%d %f '%s' '%s'\n",z.a, z.b, (z.c ? "true":"false"), z.d.c_str());
	for (int i = 0; i < 3; i++) {
		printf("%d\n", z.e[i]);
	}
	for (int i = 0; i < 5; i++) {
		printf("%d %s\n", i, z.f[i].c_str());
	}
	printf("'%c' [%s]\n", z.g, z.h);
}

struct B1
{
	int a;
	double b;
	bool c;
};

struct B2
{
	int x;
	size_t y;
	std::string z;
};

struct C
{
	std::vector<B1> v;
	std::map<int, B2> m;
};

void test2()
{
	printf("\ntest2()\n");

	// set initial data to struct C
	C c;
	{
		std::vector<B1> vector;
		for (int i = 0; i < 5; i++) {
			B1 b1;
			b1.a = i;
			b1.b = i + 0.1234;
			b1.c = i%2;
			vector.push_back(b1);
		}

		char buf[16];
		std::map<int, B2> map;
		for (int i = 0; i < 10; i++) {
			B2 b2;
			b2.x = i;
			b2.y = 10 - i;
			sprintf(buf, "hello %d", i);
			b2.z = std::string(buf);
			map.insert(std::make_pair(i,b2));
		}
		c.v = vector;
		c.m = map;
	}

	// print original data
	{
		printf("--- original data ---\n");
		const std::vector<B1> &vector = c.v;
		int size = vector.size();
		for (int i = 0; i < size; i++) {
			B1 b1 = vector[i];
			printf("B1[%d] a=%d b=%f c=%s\n",i, b1.a, b1.b, (b1.c?"true":"false"));
		}

		const std::map<int, B2> &map = c.m;
		size = map.size();
		std::map<int,B2>::const_iterator ite = map.begin();
		std::map<int,B2>::const_iterator end = map.end();
		for (; ite != end; ite++) {
			int key			= ite->first;
			const B2 &value = ite->second;
			printf("B2[%d] x=%d y=%d z=%s\n",key, value.x, value.y, value.z.c_str());
		}
	}


	// copy struct C to ByteBuffer (Serialize)
	ByteBuffer a;
	{
		// If the element of std::vector is a class or structure, you need to manually append it
		const std::vector<B1> &v = c.v;
		int size = v.size();
		a.Append(size);
		for (int i = 0; i < size; i++) {
			const B1 &b1 = v.at(i);
			a.Append(b1.a);
			a.Append(b1.b);
			a.Append(b1.c);
		}

		// If the element(key,value) of std::map is a class or structure, you need to manually append it
		const std::map<int, B2> &m = c.m;
		size = m.size();
		a.Append(size);
		std::map<int,B2>::const_iterator ite = m.begin();
		std::map<int,B2>::const_iterator end = m.end();
		for (; ite != end; ite++) {
			int key			= ite->first;
			const B2 &value = ite->second;
			a.Append(key);
			a.Append(value.x);
			a.Append(value.y);
			a.Append(value.z);
		}
	}
	printf("--- ByteBuffer dump data ---\n");
	a.Print();

	// copy ByteBuffer to struct C (Deserialize)
	C de;
	{
		a.SetPosition(0);
		std::vector<B1> vector;
		int size = 0;
		a.Value(size);
		for (int i = 0; i < size; i++) {
			B1 b1;
			a.Value(b1.a);
			a.Value(b1.b);
			a.Value(b1.c);
			vector.push_back(b1);
		}

		std::map<int, B2> map;
		size = 0;
		a.Value(size);
		for (int i = 0; i < size; i++) {
			int key;
			B2 value;
			a.Value(key);
			a.Value(value.x);
			a.Value(value.y);
			a.Value(value.z);
			map.insert(std::make_pair(key,value));
		}
		de.v = vector;
		de.m = map;
	}

	// print deserialized data
	{
		printf("--- deserialized data ---\n");
		const std::vector<B1> &vector = de.v;
		int size = vector.size();
		for (int i = 0; i < size; i++) {
			B1 b1 = vector[i];
			printf("B1[%d] a=%d b=%f c=%s\n",i, b1.a, b1.b, (b1.c?"true":"false"));
		}

		const std::map<int, B2> &map = de.m;
		size = map.size();
		std::map<int,B2>::const_iterator ite = map.begin();
		std::map<int,B2>::const_iterator end = map.end();
		for (; ite != end; ite++) {
			int key			= ite->first;
			const B2 &value = ite->second;
			printf("B2[%d] x=%d y=%d z=%s\n",key, value.x, value.y, value.z.c_str());
		}
	}

}

void test3()
{
	printf("\ntest3()\n");

	// set initial data to struct A
	A a;
	{
		a.a = -123;
		a.b = 56789.123;
		a.c = true;
		a.d = "test message";
		for (int i = 5; i < 10; i++) {
			a.e.push_back(i);
		}
		for (int i = 0; i < 3; i++) {
			char buf[256];
			::snprintf(buf,255,"PPAP %d",i);
			std::string v(buf);
			a.f.insert(std::make_pair(i,v));
		}
		a.g = 'Z';
		strcpy(a.h,"::strcpy()");
	}

	// copy struct A to ByteBuffer (Serialize)
	ByteBuffer f;
	{
		f.Append(a.a);
		f.Append(a.b);
		f.Append(a.c);
		f.Append(a.d);
		f.Append(a.e); // Directly Append() when std::vector element is primitive type
		f.Append(a.f); // Directly Append() when std::map element(key,value) is primitive type
		f.Append(a.g);
		f.Append(a.h);
	}

	// ByteBuffer to raw data (get serialized data)
	const std::string &raw_data = f.Data();

	// raw data to ByteBuffer
	ByteBuffer f2(raw_data);

	// test
	{
		if (f.Dump() == f2.Dump()) {
			printf("[same]\n");
		} else {
			printf("[difference]\n");
		}
	}

	// deserialize from f2
	{
		A z;
		//f2.SetPosition(0);
		f2.Value(z.a);
		f2.Value(z.b);
		f2.Value(z.c);
		f2.Value(z.d);
		f2.Value(z.e);
		f2.Value(z.f);
		f2.Value(z.g);
		f2.Value(z.h);

		printf("deserialize data\n");
		printf("%d\n",z.a);
		printf("%f\n",z.b);
		printf("'%s'\n",(z.c ? "true":"false"));
		printf("'%s'\n", z.d.c_str());
		for (unsigned int i = 0; i < z.e.size(); i++) {
			printf("%d\n", z.e[i]);
		}
		for (unsigned int i = 0; i < z.f.size(); i++) {
			printf("%d %s\n", i, z.f[i].c_str());
		}
		printf("'%c'\n", z.g);
		printf("[%s]\n", z.h);
	}
}

int main(int argc, char *argv[]) {
	test1();
	test2();
	test3();
	return 0;
}
