#include <stdlib.h>
#include <stdio.h>

/*
1、首先确定要hook的库名称和函数名称
2、获取目标库在内存中的module-base
	校验elf文件格式
	进行elf格式解析：
		-获取动态节的位置
		-在`.dynstr section`  中找到需要 hook 的 `symbol 对应的 index 值`
		-遍历所有的 `.relxxx section`（重定位 section），查找 `symbol index` 和 `symbol type` 都匹配的项
	将目标地址进行修改
3、刷新指令缓存
*/


__attribute__((constructor)) void hook_test(){
	printf("in constructor\n");
}

int main(int argc, char *argv[]){
	printf("hello world!\n");
	return 0;
}

