#ifndef CH_UTILS_H
#define CH_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifndef MAX_LENGTH
#define MAX_LENGTH 1024
#endif //MAX_LENGTH

#ifdef __cplusplus
extern "C" {
#endif

/**
 * get moduleBase from /proc/pid/maps
 *  pid  = -1, get self
 *  pid != -1, get target process
 *  module_name -> module name
 */
void* ch_utils_get_module_base(pid_t pid, const char* module_name){
    FILE* fp = NULL;
    void* address = NULL;
    char path[MAX_LENGTH] = {0};
    char buff[MAX_LENGTH] = {0};

    if(module_name == NULL){
        return NULL;
    }

    if(pid < 0){
        snprintf(path, sizeof(path), "/proc/self/maps");
    }
    else{
        snprintf(path, sizeof(path), "/proc/%d/maps", pid);
    }

    fp = fopen(path, "r");
    if(fp == NULL){
        perror("[-] fopen");
        return  NULL;
    }

    while(fgets(buff, sizeof(buff), fp)){
        if (strstr(buff, module_name) != NULL) {
            sscanf(buff, "%p", &address);
            break;
        }
    }
    fclose(fp);
    return address;
}


#ifdef __cplusplus
}
#endif

#endif //CH_UTILS_H