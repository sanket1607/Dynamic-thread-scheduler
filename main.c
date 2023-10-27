/**
 * Tony Givargis
 * Copyright (C), 2023
 * University of California, Irvine
 *
 * CS 238P - Operating Systems
 * main.c
 */

#include "system.h"
#include "scheduler.h"

static void
_thread_(void *arg)
{
	const char *name;
	int i;

	name = (const char *)arg;
	for (i=0; i<100; ++i) {
		printf("%s %d\n", name, i);
		us_sleep(20000);
		/*scheduler_yield();*/
	}
}

int
main(int argc, char *argv[])
{
	UNUSED(argc);
	UNUSED(argv);

	initializer();

	if (scheduler_create(_thread_, "best") ||
	    scheduler_create(_thread_, "the") ||
	    scheduler_create(_thread_, "is") ||
	    scheduler_create(_thread_, "systems") ||
	    scheduler_create(_thread_, "operating")) {
		TRACE(0);
		return -1;
	}
	scheduler_execute();
	destroyer();
	return 0;
}