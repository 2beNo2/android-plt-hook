#ifndef CHOOK_H
#define CHOOK_H

#include <elf.h>
#include "ch_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct{
    const char* pathname;
    char *symbol;
    void *new_func;
    void **old_func;
} ch_hook_info_t;


#ifdef __cplusplus
}
#endif

#endif //CHOOK_H