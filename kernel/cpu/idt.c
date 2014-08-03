#include <types.h>
#include <idt.h>
#include <system.h>
gate_desc idt[256];
struct idt_ptr idtpointer;
extern void idt_flush(void);
void idt_install(void)
{
	idtpointer.limit = (sizeof( gate_desc) * 256) - 1;
	idtpointer.base = (u32)&idt;	
	memset(&idt, 0, sizeof( gate_desc) * 256);
	idt_flush();
}


