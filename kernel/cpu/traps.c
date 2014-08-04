#include <types.h>
#include <printk.h>
static const char *exception_messages_[32] = {
	"Division by zero",				/* 0 */
	"Debug",
	"Non-maskable interrupt",
	"Breakpoint",
	"Detected overflow",
	"Out-of-bounds",				/* 5 */
	"Invalid opcode",
	"No coprocessor",
	"Double fault",
	"Coprocessor segment overrun",
	"Bad TSS",						/* 10 */
	"Segment not present",
	"Stack fault",
	"General protection fault",
	"Page fault",
	"Unknown interrupt",			/* 15 */
	"Coprocessor fault",
	"Alignment check",
	"Machine check",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved"
};
u32 isr_handler(u32 esp) {
	
	struct pt_regs *r = (struct pt_regs*)esp;
	if(r->int_no == 13)
	{
		 printk("\n\nGPF trap\n");
		 	
		
		 printk("\nEIP:    %04x\nEFLAGS: %08lx\n",
                  r->eip, r->eflags);
         printk("eax: %08lx   ebx: %08lx   ecx: %08lx   edx: %08lx\n",
                 r->eax, r->ebx, r->ecx, r->edx);
         printk("esi: %08lx   edi: %08lx   ebp: %08lx ",
                 r->esi, r->edi, r->ebp);
         printk("ds: %04x   es: %04x   ss: %04x\n",
                 r->ds, r->es , r->ss);
			 }
	if(r->int_no == 14)
	{
		unsigned int faulting_address;
  	__asm__ __volatile__ ("mov %%cr2, %0" : "=r" (faulting_address));
   
  // int present  = !(r->err_code & 0x1); 
   int rw = r->err_code & 0x2;           
   int us = r->err_code & 0x4;           
   int reserved = r->err_code & 0x8;     


   if(rw)
		printk("Read-only ");
   if(us)
		printk("User-mode ");
   if(reserved)
		printk("Reserved ");
 	printk("\nUnhandled exception: [%d] %s", r->int_no, exception_messages_[r->int_no]);
 	printk("\n faulting_address : %x", faulting_address);
 	for(;;);
		 printk("\n\nPagefault trap\n");	
		
		 printk("\nEIP:    %04x\nEFLAGS: %08lx\n",
                  r->eip, r->eflags);
         printk("eax: %08lx   ebx: %08lx   ecx: %08lx   edx: %08lx\n",
                 r->eax, r->ebx, r->ecx, r->edx);
         printk("esi: %08lx   edi: %08lx   ebp: %08lx ",
                 r->esi, r->edi, r->ebp);
         printk("ds: %04x   es: %04x   ss: %04x\n",
                 r->ds, r->es , r->ss);
	}
	for(;;);
	
return esp;
}
