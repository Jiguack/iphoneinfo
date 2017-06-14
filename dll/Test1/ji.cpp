// Test1.cpp : Defines the entry point for the console application.
//

#include <iostream>

#include "dll.h"

void check() {
	int count = icount();
	if (count > 0)
		printf("{\'connected_num\':%d}", count);
	else
		printf("{\'connected_num\':0}");
}

void list() {
	printf("{\'list\':");
	ilist();
	printf("}");
}

void battery() {
	ibattery();
}

void info() {
	printf("{\'info\':0}");
}

void print_usage(char **argv)
{
	char* name = argv[0];
	printf("Usage: %s check\n", name);
	printf("Usage: %s list\n", name);
	printf("Usage: %s battery {id}\n", name);
	printf("Usage: %s info {id}\n", name);
}

int main(int argc, char **argv)
{
	if (argc <= 1) {
		print_usage(argv);
	}
	else if (!strcmp(argv[1], "check")) {
		check();
	}
	else if (!strcmp(argv[1], "list")) {
		list();
	}
	else if (!strcmp(argv[1], "battery")) {
		if (argc == 3)
			ibattery_id(argv[2]);
		else
			battery();
	}
	else if (!strcmp(argv[1], "info")) {
		if (argc == 3)
			iinfo_id(argv[2]);
		else
			iinfo();
	}
	else {
		print_usage(argv);
	}

	return 0;
}
