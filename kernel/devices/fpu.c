#include <system.h>
#include <types.h>
#include <irq.h>
void
set_fpu_cw(const u16 cw) {
	asm volatile("fldcw %0" :: "m"(cw));
}

void enable_fpu(void) {
	asm volatile ("clts");
	size_t t;
	asm volatile ("mov %%cr4, %0" : "=r"(t));
	t |= 3 << 9;
	asm volatile ("mov %0, %%cr4" :: "r"(t));
}

void disable_fpu(void) {
	size_t t;
	asm volatile ("mov %%cr0, %0" : "=r"(t));
	t |= 1 << 3;
	asm volatile ("mov %0, %%cr0" :: "r"(t));
}


u8 saves[512] __attribute__((aligned(16)));


void init_fpu(void) {
	asm volatile ("fninit");
	set_fpu_cw(0x37F);
}

void invalid_op(struct pt_regs * r) {

	enable_fpu();

}

void switch_fpu(void) {
	disable_fpu();
}

/* Enable the FPU context handling */
void auto_fpu(void) {
	 setup_irq(6, (struct irqaction *)&invalid_op ); 
     setup_irq(7, (struct irqaction *)&invalid_op ); 
}
