#include "pado.h"

typedef unsigned short Elf32_Half;
typedef unsigned int   Elf32_Word;
typedef unsigned int   Elf32_Addr;
typedef unsigned int   Elf32_Off;
#define EI_NIDENT 16

typedef struct {
  unsigned char e_ident[EI_NIDENT];
  Elf32_Half e_type;
  Elf32_Half e_machine;
  Elf32_Word e_version;
  Elf32_Addr e_entry;
  Elf32_Off  e_phoff;
  Elf32_Off  e_shoff;
  Elf32_Word e_flags;
  Elf32_Half e_ehsize;
  Elf32_Half e_phentsize;
  Elf32_Half e_phnum;
  Elf32_Half e_shentsize;
  Elf32_Half e_shnum;
  Elf32_Half e_shstrndx;
} Elf32_Ehdr;

typedef struct {
  Elf32_Word p_type;
  Elf32_Off  p_offset;
  Elf32_Addr p_vaddr;
  Elf32_Addr p_paddr;
  Elf32_Word p_filesz;
  Elf32_Word p_memsz;
  Elf32_Word p_flags;
  Elf32_Word p_align;
} Elf32_Phdr;

void pado_elf_exec_from_buffer(char* buffer, int size) {
    Elf32_Ehdr* ehdr = (Elf32_Ehdr*)buffer;
    if(ehdr->e_ident[0]!=0x7F || ehdr->e_ident[1]!='E' || ehdr->e_ident[2]!='L' || ehdr->e_ident[3]!='F'){
         pado_print("Invalid ELF header\n");
         return;
    }
    Elf32_Phdr* phdr = (Elf32_Phdr*)(buffer + ehdr->e_phoff);
    int i;
    for(i = 0; i < ehdr->e_phnum; i++){
         if(phdr[i].p_type == 1){  /* PT_LOAD */
              char* dest = (char*)phdr[i].p_vaddr;
              char* src = buffer + phdr[i].p_offset;
              int j;
              for(j = 0; j < phdr[i].p_filesz; j++){
                   dest[j] = src[j];
              }
              for(; j < phdr[i].p_memsz; j++){
                   dest[j] = 0;
              }
         }
    }
    void (*entry)() = (void (*)())(ehdr->e_entry);
    entry();
}
