#include <stdlib.h>
#include <stdio.h>

#include "chook.h"


int new_puts(const char* fmt)
{
	printf("hook puts!");
	return 0;
}


__attribute__((constructor)) void hook_test()
{
	void* old_func;
	chook_register("test", "puts", new_puts, &old_func);
	chook_hook();
	puts("hello world!");
	chook_unhook();
	puts("hello world!");
}


int main(int argc, char *argv[])
{
	puts("hook start!");
	return 0;
}

