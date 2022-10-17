#include <stdlib.h>
#include <stdio.h>

#include "chook.h"



void new_printf(const char* fmt, ...){
	//printf("in new_printf\n");  // 会递归
	puts("in new_printf");
}


__attribute__((constructor)) void hook_test(){
	printf("in constructor\n");
	void* old_func;
	ch_hook_info_t* info = chook_register("libc.so", "printf", new_printf, &old_func);
	if(NULL == info){
		return;
	}

	chook_hook(info);

	chook_destruct(info);
}


int main(int argc, char *argv[]){
	printf("hello world!\n");
	return 0;
}

