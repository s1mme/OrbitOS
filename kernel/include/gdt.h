#ifndef GDT_H
#define GDT_H

#include <types.h>

struct gdt_entry
{
	u16 limit_low;
	u16 base_low;
	u8  base_middle;
	u8  access;
	u8  gran;
	u8  base_high;
}__attribute__((packed));

struct gdt_ptr
{
	u16 limit;
	u32 base;	
}__attribute__((packed));

extern void gdt_flush(void);
extern void gdt_install(void);

extern void set_kernel_stack(u32 stack);
 

struct tss_entry_struct
{
   u32 prev_tss;   
   u32 esp0;       
   u32 ss0;        
   u32 esp1;       
   u32 ss1;
   u32 esp2;
   u32 ss2;
   u32 cr3;
   u32 eip;
   u32 eflags;
   u32 eax;
   u32 ecx;
   u32 edx;
   u32 ebx;
   u32 esp;
   u32 ebp;
   u32 esi;
   u32 edi;
   u32 es;         
   u32 cs;         
   u32 ss;         
   u32 ds;         
   u32 fs;         
   u32 gs;         
   u32 ldt;        
   u16 trap;
   u16 iomap_base;
} __attribute__((packed));

typedef struct tss_entry_struct tss_entry_t; 
#endif
