#include <string.h>
#include <sys/mman.h>

#include "ch_utils.h"
#include "ch_elf.h"


int ch_elf_check_elfheader(uintptr_t base_addr){
    ElfW(Ehdr) *ehdr = (ElfW(Ehdr)*)base_addr;

    //check magic
    if(0 != memcmp(ehdr->e_ident, ELFMAG, SELFMAG)) return -1;

    //check class (64/32)
#if defined(__LP64__)
    if(ELFCLASS64 != ehdr->e_ident[EI_CLASS]) return -1;
#else
    if(ELFCLASS32 != ehdr->e_ident[EI_CLASS]) return -1;
#endif

    //check endian (little/big)
    if(ELFDATA2LSB != ehdr->e_ident[EI_DATA]) return -1;

    //check version
    if(EV_CURRENT != ehdr->e_ident[EI_VERSION]) return -1;

    //check type
    if(ET_EXEC != ehdr->e_type && ET_DYN != ehdr->e_type) return -1;

    //check machine
#if defined(__arm__)
    if(EM_ARM != ehdr->e_machine) return -1;
#elif defined(__aarch64__)
    if(EM_AARCH64 != ehdr->e_machine) return -1;
#elif defined(__i386__)
    if(EM_386 != ehdr->e_machine) return -1;
#elif defined(__x86_64__)
    if(EM_X86_64 != ehdr->e_machine) return -1;
#else
    return -1;
#endif

    //check version
    if(EV_CURRENT != ehdr->e_version) return -1;
    return 0;
}


static ElfW(Phdr) *ch_elf_get_segment_by_type(ch_elf_t *self, ElfW(Word) type){
    ElfW(Phdr) *phdr = NULL;
    for(phdr = self->phdr; phdr < self->phdr + self->ehdr->e_phnum; phdr++){
        if(phdr->p_type == type){
            return phdr;
        }
    }
    return NULL;
}


static ElfW(Phdr) *ch_elf_get_segment_by_type_and_offset(ch_elf_t *self, ElfW(Word) type, ElfW(Off) offset){
    ElfW(Phdr) *phdr = NULL;
    for(phdr = self->phdr; phdr < self->phdr + self->ehdr->e_phnum; phdr++){
        if(phdr->p_type == type && phdr->p_offset == offset){
            return phdr;
        }
    }
    return NULL;
}


//ELF hash func
static uint32_t ch_elf_hash(const uint8_t *name){
    uint32_t h = 0, g;
    while (*name) {
        h = (h << 4) + *name++;
        g = h & 0xf0000000;
        h ^= g;
        h ^= g >> 24;
    }
    return h;
}


static uint32_t ch_elf_hash_lookup(ch_elf_t *self, const char *symbol_name){
    uint32_t mod;
    const char *symbol_cur;
    uint32_t symbol_hash = ch_elf_hash((uint8_t *)symbol_name);
    mod = self->hash_bucket[symbol_hash % self->hash_bucket_cnt];

    for(; mod != 0; mod = self->hash_chain[mod]){
        symbol_cur = self->dynstr_tab + self->dynsym_tab[mod].st_name;
        if(0 == strcmp(symbol_cur, symbol_name)){
            LOGD("[+] SYMBOL_STR :%s", symbol_cur);
            LOGD("[+] SYMBOL_IDX :%d", mod);
            return mod;
        }
    }
    return -1;
}


static void* ch_elf_get_vaddr_from_re(ch_elf_t *self, uint32_t symbol_idx){
    /* 
    typedef struct elf32_rel { //修正的大小固定为4个字节
        Elf32_Addr r_offset;   // 重定位后数据存放的地址
        Elf32_Word r_info;     // 低8位调整类型  高24位表示动态符号表的索引
            #define ELF32_R_SYM(x) ((x) >> 8)    //--> 动态符号表的索引  
            #define ELF32_R_TYPE(x) ((x) & 0xff) //--> 调整类型，不同指令集有不同的调整类型 
    } Elf32_Rel;
    */

    ElfW(Rel) *replt_begin   = (ElfW(Rel) *)self->relplt;
    ElfW(Addr) replt_sz      = self->relplt_sz;
    ElfW(Rel) *replt_end     = replt_begin + replt_sz / sizeof(ElfW(Rel));

    for(; replt_begin <= replt_end; replt_begin++){
        if((replt_begin->r_info >> 8) == symbol_idx){
            LOGD("[+] r_offset :%p", replt_begin->r_offset);
            return (void*)replt_begin->r_offset;
        }
    }

    return NULL;
}


//GNU hash func
static uint32_t ch_elf_gnu_hash(const uint8_t *name){
    uint32_t h = 5381;
    while(*name != 0){
        h += (h << 5) + *name++;
    }
    return h;
}


void ch_elf_flush_instruction_cache(uintptr_t addr)
{
    __builtin___clear_cache((void *)PAGE_START(addr), (void *)PAGE_END(addr));
}


int ch_elf_init(ch_elf_t *self, uintptr_t base_addr){
    ElfW(Phdr) *phdr0         = NULL;
    ElfW(Phdr) *dynamic_Phdr  = NULL;
    ElfW(Dyn)  *dyn           = NULL;
    ElfW(Dyn)  *dyn_end       = NULL;
    uint32_t   *hash          = NULL;

    if(0 == base_addr) return -1;
    
    memset(self, 0, sizeof(ch_elf_t));
    self->base_addr   = (ElfW(Addr))base_addr;
    self->ehdr        = (ElfW(Ehdr)*)base_addr;
    self->phdr        = (ElfW(Phdr)*)(base_addr + self->ehdr->e_phoff);

    //find the first load-segment with offset 0
    phdr0 = ch_elf_get_segment_by_type_and_offset(self, PT_LOAD, 0);
    if(NULL == phdr0) return -1;

    //save load bias addr
    if(self->base_addr < phdr0->p_vaddr) return -1;
    self->bias_addr = self->base_addr - phdr0->p_vaddr;

    //find dynamic segment
    dynamic_Phdr = ch_elf_get_segment_by_type(self, PT_DYNAMIC);
    if(NULL == dynamic_Phdr) return -1;

    //save dynamic phdr
    self->dynamic     = (ElfW(Dyn)*)(self->bias_addr + dynamic_Phdr->p_vaddr);
    self->dynamic_sz  = dynamic_Phdr->p_memsz;
    dyn     = self->dynamic;
    dyn_end = self->dynamic + (self->dynamic_sz / sizeof(ElfW(Dyn)));
    LOGD("[+] (DYNAMIC) :0x%l"PRIxPTR"", dynamic_Phdr->p_vaddr);

    for(; dyn < dyn_end; dyn++){
        switch(dyn->d_tag)
        {
        case DT_NULL:
            {
                dyn = dyn_end;
                break;
            }
        case DT_STRTAB:
            {
                self->dynstr_tab = (const char *)(self->bias_addr + dyn->d_un.d_ptr);
                LOGD("[+] (STRTAB) :0x%x", dyn->d_un.d_ptr);
                break;
            }
        case DT_SYMTAB:
            {
                self->dynsym_tab = (ElfW(Sym)*)(self->bias_addr + dyn->d_un.d_ptr);
                LOGD("[+] (SYMTAB) :0x%x", dyn->d_un.d_ptr);
                break;
            }
        case DT_PLTREL:
            {
                //use rel or rela?
                self->is_use_rela = (dyn->d_un.d_val == DT_RELA ? 1 : 0);
                break;
            }
        case DT_JMPREL:
            {
                self->relplt = self->bias_addr + dyn->d_un.d_ptr;
                LOGD("[+] (JMPREL) :0x%x", dyn->d_un.d_ptr);
                break;
            }
        case DT_PLTRELSZ:
            {
                self->relplt_sz = dyn->d_un.d_val;
                LOGD("[+] (PLTRELSZ) :0x%x", dyn->d_un.d_val);
                break;
            }
        case DT_REL:
        case DT_RELA:
            {
                self->reldyn = self->bias_addr + dyn->d_un.d_ptr;
                LOGD("[+] (REL/RELA) :0x%x", dyn->d_un.d_ptr);
                break;
            }
        case DT_RELSZ:
        case DT_RELASZ:
            {
                self->reldyn_sz = dyn->d_un.d_val;
                LOGD("[+] (RELSZ/RELASZ) :0x%x", dyn->d_un.d_val);
                break;
            }
        case DT_HASH:
            {
                self->hash = self->bias_addr + dyn->d_un.d_ptr;
                hash = (uint32_t*)self->hash;
                self->hash_bucket_cnt  = hash[0];
                self->hash_chain_cnt   = hash[1];
                self->hash_bucket      = &hash[2];
                self->hash_chain       = &(self->hash_bucket[self->hash_bucket_cnt]);
                LOGD("[+] (HASH) :0x%x", dyn->d_un.d_ptr);
                break;
            }
        case DT_GNU_HASH:
            {
                self->gnu_hash = self->bias_addr + dyn->d_un.d_ptr;
                hash = (uint32_t *)(self->gnu_hash);
                self->gnu_bucket_cnt  = hash[0];
                self->symoffset       = hash[1];
                self->bloom_sz        = hash[2];
                self->bloom_shift     = hash[3];
                self->bloom           = (ElfW(Addr) *)(&hash[4]);
                self->gnu_bucket      = (uint32_t *)(&(self->bloom[self->bloom_sz]));
                self->gnu_chain       = (uint32_t *)(&(self->gnu_bucket[self->gnu_bucket_cnt]));
                LOGD("[+] (GNU_HASH) :0x%x", dyn->d_un.d_ptr);
                break;
            }
        default:
            break;
        }
    }

    return 0;
}


int ch_elf_hook(ch_elf_t *self, const char *symbol_name, void *new_func, void **old_func){
    uint32_t symbol_idx = -1;
    uintptr_t re_addr = NULL;

    //获取符号在dynsym表中的序号
    symbol_idx = ch_elf_hash_lookup(self, symbol_name);
    if(-1 == symbol_idx){
        LOGD("[-] FIND SYMBOL_IDX ERROR!");
        return -1;
    }
    
    //从重定位表中找到符号在内存中的地址
    re_addr = self->bias_addr + ch_elf_get_vaddr_from_re(self, symbol_idx);
    if(NULL == re_addr){
        return -1;
    }
    LOGD("[+] re_addr :%p", re_addr);

    //应该先获取目标地址的内存权限，保存下来

    //修改内存权限
    mprotect(PAGE_START(re_addr), PAGE_SIZE, PROT_READ | PROT_WRITE);

    //替换掉目标地址保存的函数地址
    *(void **)re_addr = new_func;

    //刷新处理器缓存
    ch_elf_flush_instruction_cache(re_addr);

    return 0;
}



