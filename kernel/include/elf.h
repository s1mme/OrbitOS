#ifndef ELF_H
#define ELF_H
#include <types.h>
#include <fs.h>

typedef struct
{
    unsigned char  ident[16];
    unsigned short type;
    unsigned short machine;
    unsigned int version;
    unsigned int entry;
    unsigned int phoff;
    unsigned int shoff;
    unsigned int flags;
    unsigned short ehsize;
    unsigned short phentrysize;
    unsigned short phnum;
    unsigned short shentrysize;
    unsigned short shnum;
    unsigned short shstrndx;
} elf_header_t;


typedef struct
{
    unsigned int type;
    unsigned int offset;
    unsigned int vaddr;
    unsigned int paddr;
    unsigned int filesz;
    unsigned int memsz;
    unsigned int flags;
    unsigned int align;
} program_header_t;
 int sys_sbrk(int size);
 int execve(char *name, char **argv, char **env);
 
 
 
 
/*
 * Unless otherwise stated, the definitions herein
 * are sourced from the Portable Formats Specification,
 * version 1.1 - ELF: Executable and Linkable Format
 */

/*
 * ELF Magic Signature
 */
#define ELFMAG0   0x7f
#define ELFMAG1   'E'
#define ELFMAG2   'L'
#define ELFMAG3   'F'
#define EI_NIDENT 16

/*
 * ELF Datatypes
 */
typedef u32 Elf32_Word;
typedef u32 Elf32_Addr;
typedef u32 Elf32_Off;
typedef u32 Elf32_Sword;
typedef u16 Elf32_Half;

/*
 * ELF Header
 */
typedef struct {
	unsigned char e_ident[EI_NIDENT];
	Elf32_Half    e_type;
	Elf32_Half    e_machine;
	Elf32_Word    e_version;
	Elf32_Addr    e_entry;
	Elf32_Off     e_phoff;
	Elf32_Off     e_shoff;
	Elf32_Word    e_flags;
	Elf32_Half    e_ehsize;
	Elf32_Half    e_phentsize;
	Elf32_Half    e_phnum;
	Elf32_Half    e_shentsize;
	Elf32_Half    e_shnum;
	Elf32_Half    e_shstrndx;
} Elf32_Header;

/*
 * e_type
 */

#define ET_NONE   0     /* No file type */
#define ET_REL    1     /* Relocatable file */
#define ET_EXEC   2     /* Executable file */
#define ET_DYN    3     /* Shared object file */
#define ET_CORE   4     /* Core file */
#define ET_LOPROC 0xff0 /* [Processor Specific] */
#define ET_HIPROC 0xfff /* [Processor Specific] */

/*
 * Machine types
 */
#define EM_NONE  0
#define EM_386   3

#define EV_NONE    0
#define EV_CURRENT 1

/** Program Header */
typedef struct {
	Elf32_Word p_type;
	Elf32_Off  p_offset;
	Elf32_Addr p_vaddr;
	Elf32_Addr p_paddr;
	Elf32_Word p_filesz;
	Elf32_Word p_flags;
	Elf32_Word p_align;
} Elf32_Phdr;

/* p_type values */
#define PT_NULL    0 /* Unused, skip me */
#define PT_LOAD    1 /* Loadable segment */
#define PT_DYNAMIC 2 /* Dynamic linking information */
#define PT_INTERP  3 /* Interpreter (null-terminated string, pathname) */
#define PT_NOTE    4 /* Auxillary information */
#define PT_SHLIB   5 /* Reserved. */
#define PT_PHDR    6 /* Oh, it's me. Hello! Back-reference to the header table itself */
#define PT_LOPROC  0x70000000
#define PT_HIPROC  0x7FFFFFFF


/** Section Header */
typedef struct {
	Elf32_Word sh_name;
	Elf32_Word sh_type;
	Elf32_Word sh_flags;
	Elf32_Addr sh_addr;
	Elf32_Off  sh_offset;
	Elf32_Word sh_size;
	Elf32_Word sh_link;
	Elf32_Word sh_info;
	Elf32_Word sh_addralign;
	Elf32_Word sh_entsize;
} Elf32_Shdr;

typedef struct {
	u32  id;
	uintptr_t ptr;
} Elf32_auxv;

/* sh_type values */
#define SHT_NONE     0
#define SHT_PROGBITS 1
#define SHT_SYMTAB   2
#define SHT_STRTAB   3
#define SHT_NOBITS   8
extern int load_elf( char *path, char *argv[], char *envp[]);
#endif
