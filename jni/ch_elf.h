#ifndef CH_ELF_H
#define CH_ELF_H

#include <stdint.h>
#include <elf.h>
#include <link.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    ElfW(Addr)  base_addr;
    ElfW(Addr)  bias_addr;
    ElfW(Ehdr)  *ehdr;
    ElfW(Phdr)  *phdr;
    ElfW(Dyn)   *dynamic; 
    ElfW(Word)  dynamic_sz;

    const char *dynstr_tab; //.dynstr (string-table)
    ElfW(Sym)  *dynsym_tab; //.dynsym (symbol-index to string-table's offset)

    int         is_use_rela;
    ElfW(Addr)  relplt; //.rel.plt or .rela.plt
    ElfW(Word)  relplt_sz;
    ElfW(Addr)  reldyn; //.rel.dyn or .rela.dyn
    ElfW(Word)  reldyn_sz;
    ElfW(Addr)  relandroid; //android compressed rel or rela
    ElfW(Word)  relandroid_sz;

    //ELF hash
    ElfW(Addr)  hash;
    uint32_t    hash_bucket_cnt;
    uint32_t    hash_chain_cnt;
    uint32_t    *hash_bucket;
    uint32_t    *hash_chain;

    //Elf GNU hash
    ElfW(Addr)  gnu_hash;
    uint32_t    gnu_bucket_cnt;
    uint32_t    symoffset;
    uint32_t    bloom_sz;
    uint32_t    bloom_shift;
    ElfW(Addr)  *bloom;
    uint32_t    *gnu_bucket;
    uint32_t    *gnu_chain;

    ElfW(Addr)  re_addr; //save symbol vaddr
} ch_elf_t;

int ch_elf_check_elfheader(uintptr_t base_addr);
int ch_elf_init(ch_elf_t *self, uintptr_t base_addr);
int ch_elf_hook(ch_elf_t *self, const char *symbol_name, void *new_func, void **old_func);
int ch_elf_unhook(ch_elf_t *self, void **old_func);

#ifdef __cplusplus
}
#endif

#endif //CH_ELF_H
