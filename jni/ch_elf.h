#ifndef CH_ELF_H
#define CH_ELF_H

#include <stdint.h>
#include <elf.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__LP64__)
#define ElfW(type) Elf64_ ## type
#else
#define ElfW(type) Elf32_ ## type
#endif

typedef struct{
 

} ch_elf_t;

int  ch_elf_check_elfheader(uintptr_t base_addr);
int  ch_elf_init(ch_elf_t *self, uintptr_t base_addr, const char *pathname, uint32_t file_sz);


#ifdef __cplusplus
}
#endif

#endif //CH_ELF_H
