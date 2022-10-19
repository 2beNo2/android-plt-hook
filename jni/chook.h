#ifndef CHOOK_H
#define CHOOK_H 1

/*
1、首先确定要hook的库名称和函数名称
2、获取目标库在内存中的module-base
	校验elf文件格式
	进行elf格式解析：
		-获取动态节的位置，解析获取elf的各种section
	进行plt hook：
		-在`.dynstr section`  中找到需要 hook 的 `symbol 对应的 index 值`
		-遍历所有的 `.relxxx section`（重定位 section），查找 `symbol index` 和 `symbol type` 都匹配的项
	将目标地址进行修改
3、刷新指令缓存
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct{
    const char* module_name;
    char *symbol_name;
    void *new_func;
    void **old_func;
} ch_hook_info_t;

ch_hook_info_t* chook_register(const char *module_name, const char *symbol_name, void *new_func, void **old_func);
int  chook_hook(ch_hook_info_t* info);
int  chook_unhook(ch_hook_info_t* info);
void chook_destruct(ch_hook_info_t* info);

#ifdef __cplusplus
}
#endif

#endif //CHOOK_H