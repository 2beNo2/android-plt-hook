#include <stdlib.h>
#include <stdio.h>

#include "chook.h"



int new_puts(const char* fmt){
	//printf("in new_printf\n");  // 会递归
	printf("hook puts!");
	return 0;
}


__attribute__((constructor)) void hook_test(){
	void* old_func;
	ch_hook_info_t* info = chook_register("test", "puts", new_puts, &old_func);
	if(NULL == info){
		return;
	}

	chook_hook(info);
	chook_destruct(info);
}


int main(int argc, char *argv[]){
	puts("hello world!");
	return 0;
}

