#include "ch_utils.h"


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
        if (perm[0] == '-' && perm[1] == '-' && perm[2] == '-')
            continue;

        if (strstr(buff, module_name) != NULL) {
            break;
        }
    }

    fclose(fp);
    return base_addr;
}
