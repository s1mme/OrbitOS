#include <types.h>
#include <gdt.h>
#include <system.h>
#define NUM_OF_GDT 6

extern void tss_flush(void);
struct gdt_ptr gdtpointer;
tss_entry_t tss_entry;
struct gdt_entry gdt[NUM_OF_GDT];

void gdt_set_gate(int num, u32 base, u32 limit, u8 access, u8 gran)
{
	gdt[num].base_low = (base & 0xffff);
	gdt[num].base_middle = (base >> 16) & 0xff;
	gdt[num].base_high = (base >> 24) & 0xff;

	gdt[num].limit_low = (limit & 0xffff);
	gdt[num].gran = ((limit >> 16) & 0x0f);

	gdt[num].gran |= (gran & 0xf0);
	gdt[num].access = access;
}

void write_tss(s32 num, u16 ss0, u32 esp0)
{
	u32 base = (u32) &tss_entry;
	u32 limit = base + sizeof(tss_entry);
	gdt_set_gate(num,base,limit,0xE9,0x00);
	memset(&tss_entry, 0 ,sizeof(tss_entry));
	tss_entry.ss0 = ss0;
	tss_entry.esp0 = esp0;
	tss_entry.cs = 0x0b;
	tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs = 0x13;
	
}
void gdt_install(void)
{
	gdtpointer.limit = (sizeof(struct gdt_entry) * NUM_OF_GDT) - 1;
	gdtpointer.base = (u32)&gdt;
	
	gdt_set_gate(0,0,0,0,0);
	
	gdt_set_gate(1,0,0xffffffff,0x9a, 0xcf);
	 
	gdt_set_gate(2,0,0xffffffff,0x92, 0xcf);
	gdt_set_gate(3,0,0xffffffff,0xfa, 0xcf); /*usermode*/
	 
	gdt_set_gate(4,0,0xffffffff,0xf2, 0xcf); /*usermode*/
	write_tss(5,0x10,0x0);
	
	gdt_flush();
	tss_flush();
}

void set_kernel_stack(u32 stack)
{
	tss_entry.esp0 = stack;		
}




