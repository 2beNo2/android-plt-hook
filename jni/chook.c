#include "chook.h"
#include "ch_utils.h"
#include "ch_elf.h"


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
    if(NULL == info){
        return -1;
    }
    if(NULL == info->module_name || NULL == info->symbol_name || NULL == info->new_func){
        return -1;
    }

    //get module base
    void* module_base = ch_utils_get_module_base(-1, info->module_name);
    if(NULL == module_base){
        LOGD("[-] get module_base failed");
        return -1;
    }
    LOGD("[+] module_base:%p", module_base);

    //check elf header format

    //init elf

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