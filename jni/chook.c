#include "chook.h"
#include "ch_utils.h"
#include "ch_elf.h"



/**
 * get module base from /proc/pid/maps
 *  pid  = -1, get self
 *  pid != -1, get target process
 *  module_name -> module name
 */
static void* ch_utils_get_module_base(pid_t pid, const char* module_name){
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


ch_hook_info_t* chook_register(const char *module_name, const char *symbol_name, void *new_func, void **old_func){
    ch_hook_info_t* info = NULL;

    if(NULL == module_name || NULL == symbol_name || NULL == new_func){
        return NULL;
    }
    
    if(NULL == (info = (ch_hook_info_t*)malloc(sizeof(ch_hook_info_t)))){
        return NULL;
    }
    if(NULL == (info->symbol_name = strdup(symbol_name))){
        free(info);
        return NULL;
    }
    
    info->module_name = module_name;
    info->new_func = new_func;
    info->old_func = old_func;

    return info;
}


int chook_hook(ch_hook_info_t* info){
    ch_elf_t ch_self;

    if(NULL == info){
        return -1;
    }
    if(NULL == info->module_name || NULL == info->symbol_name || NULL == info->new_func){
        return -1;
    }

    //get module base
    void* module_base = ch_utils_get_module_base(-1, info->module_name);
    if(NULL == module_base){
        LOGD("[-] get module_base failed!");
        return -1;
    }
    LOGD("[+] module_base:%p", module_base);


    //check elf header format
    if(ch_elf_check_elfheader((uintptr_t)module_base) < 0){
        LOGD("[-] elf header format error!");
        return -1;
    }
    LOGD("[+] elf header format is ok");


    //init elf
    if(ch_elf_init(&ch_self, (uintptr_t)module_base) < 0){
        LOGD("[-] elf init failed!");
        return -1;
    }
    LOGD("[+] elf init ok");


    //start hook

    
    return 0;
}


int chook_unhook(ch_hook_info_t* info){
    if(NULL == info){
        return -1;
    }
    if(NULL == info->module_name || NULL == info->symbol_name || NULL == info->new_func){
        return -1;
    }

    return 0;
}


void chook_destruct(ch_hook_info_t* info){
    if(NULL != info->symbol_name){
        free(info->symbol_name);
    }
    if(NULL != info){
        free(info);
        info = NULL;
    }
}