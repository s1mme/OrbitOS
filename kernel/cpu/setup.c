#include <desc.h>
#include <gdt.h>
#include <idt.h>

void cpu_init (void)
{
		 gdt_install();
		 idt_install();
}
