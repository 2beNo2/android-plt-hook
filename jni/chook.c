#include "chook.h"
#include "ch_utils.h"
#include "ch_elf.h"


#define MAX_ITEM_COUNT 256

typedef struct
{
    const char *module_name;
    const char *symbol_name;
    void *new_func;
    void **old_func;
    ch_elf_t ch_elf;
} ch_hook_info_t;

typedef struct 
{
	ch_hook_info_t item[MAX_ITEM_COUNT];
	int size;
}pltHookInfo;

static pltHookInfo g_info = {0};


static void* chook_get_module_base(pid_t pid, const char* module_name)
{
    FILE* fp = NULL;
    void* base_addr = NULL;
    char perm[5];
    char path[MAX_LENGTH] = {0};
    char buff[MAX_LENGTH] = {0};

    if(NULL == module_name){
        return NULL;
    }

    if(pid < 0){
        snprintf(path, sizeof(path), "/proc/self/maps");
    }
    else{
        snprintf(path, sizeof(path), "/proc/%d/maps", pid);
    }

    fp = fopen(path, "r");
    if(NULL == fp){
        perror("[-] fopen");
        return NULL;
    }

    while(fgets(buff, sizeof(buff), fp)){
        if(sscanf(buff, "%p-%*p %4s", &base_addr, perm) != 2) 
            continue;
        
        // do not touch the shared memory
        if (perm[3] != 'p') continue;

        // Ignore permission PROT_NONE maps
        if (perm[0] == '-' && perm[1] == '-' && perm[2] == '-') continue;

        if (strstr(buff, module_name) != NULL) {
            break;
        }
    }

    fclose(fp);
    return base_addr;
}


static ch_hook_info_t *chook_add_item() 
{
	if (g_info.size >= MAX_ITEM_COUNT) {
		return NULL;
	}
	++g_info.size;
	return &g_info.item[g_info.size];
}


static void chook_del_item(int pos)
{
    memcpy(&g_info.item[pos], &g_info.item[pos+1], sizeof(ch_hook_info_t) * (256 - pos));
    memset(&g_info.item[g_info.size], 0, sizeof(ch_hook_info_t));
	--g_info.size;
}


int chook_register(const char *module_name, const char *symbol_name, void *new_func, void **old_func)
{
    if(NULL == module_name || NULL == symbol_name || NULL == new_func){
        return -1;
    }
    
    ch_hook_info_t* item = chook_add_item();
    if(NULL == item){
        LOGD("[-] add hook item failed!");
        return -1;
    }

    item->symbol_name = symbol_name;
    item->module_name = module_name;
    item->new_func = new_func;
    item->old_func = old_func;
    return 0;
}


int chook_hook()
{
    ch_hook_info_t *info = &g_info.item[g_info.size];

    if(NULL == info->module_name || NULL == info->symbol_name || NULL == info->new_func){
        return -1;
    }

    //get module base
    void* module_base = chook_get_module_base(-1, info->module_name);
    if(NULL == module_base){
        LOGD("[-] get module base failed!");
        return -1;
    }

    //check elf header format
    if(ch_elf_check_elfheader((uintptr_t)module_base) < 0){
        LOGD("[-] elf header format error!");
        return -1;
    }

    //init elf
    if(ch_elf_init(&info->ch_elf, (uintptr_t)module_base) < 0){
        LOGD("[-] elf init failed!");
        return -1;
    }

    //start hook
    if(ch_elf_hook(&info->ch_elf, info->symbol_name, info->new_func, info->old_func) < 0){
        LOGD("[-] elf plt hook failed!");
        return -1;
    }

    return 0;
}


int chook_unhook()
{
    ch_hook_info_t *info = &g_info.item[g_info.size];

    if(NULL == info->module_name || NULL == info->symbol_name || NULL == info->new_func){
        return -1;
    }

    ch_elf_unhook(&info->ch_elf, info->old_func);
    
    return 0;
}

