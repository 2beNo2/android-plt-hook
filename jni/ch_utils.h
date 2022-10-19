#ifndef CH_UTILS_H
#define CH_UTILS_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifndef MAX_LENGTH
#define MAX_LENGTH 1024
#endif //MAX_LENGTH

#include <android/log.h>

#define CH_DEBUG
#define TAG "CH_HOOK"

#ifdef CH_DEBUG
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#else
#define LOGD(...)
#endif // CH_DEBUG

#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,  TAG, __VA_ARGS__)


#ifdef __cplusplus
extern "C" {
#endif


/**
 * get module base from /proc/pid/maps
 *  pid  = -1, get self
 *  pid != -1, get target process
 *  module_name -> module name
 */
void* ch_utils_get_module_base(pid_t pid, const char* module_name){
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

#ifdef __cplusplus
}
#endif


#endif //CH_UTILS_H