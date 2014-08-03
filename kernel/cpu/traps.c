#include <types.h>
#include <printk.h>

u32 isr_handler(u32 esp) {
	
	struct pt_regs *r = (struct pt_regs*)esp;
	if(r->int_no == 13)
		 printk("\n\nGPF trap\n");
	if(r->int_no == 14)
		 printk("\n\nPagefault trap\n");	
		
		 printk("\nEIP:    %04x\nEFLAGS: %08lx\n",
                  r->eip, r->eflags);
         printk("eax: %08lx   ebx: %08lx   ecx: %08lx   edx: %08lx\n",
                 r->eax, r->ebx, r->ecx, r->edx);
         printk("esi: %08lx   edi: %08lx   ebp: %08lx ",
                 r->esi, r->edi, r->ebp);
         printk("ds: %04x   es: %04x   ss: %04x\n",
                 r->ds, r->es , r->ss);
	for(;;);
	
return esp;
}
