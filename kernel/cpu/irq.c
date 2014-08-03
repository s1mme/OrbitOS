#include <hw_irq.h>
#include <irq.h>
#include <types.h>
#include <desc.h>
#include <idt.h>
#include <system.h>
#include <io.h>
#include <printk.h>
/* i8259A PIC registers */
#define PIC_MASTER_CMD          0x20
#define PIC_MASTER_IMR          0x21
#define PIC_MASTER_ISR          PIC_MASTER_CMD
#define PIC_MASTER_POLL         PIC_MASTER_ISR
#define PIC_MASTER_OCW3         PIC_MASTER_ISR
#define PIC_SLAVE_CMD           0xa0
#define PIC_SLAVE_IMR           0xa1

/* i8259A PIC related value */
#define PIC_CASCADE_IR          2
#define MASTER_ICW4_DEFAULT     0x01
#define SLAVE_ICW4_DEFAULT      0x01
#define PIC_ICW4_AEOI           2  
#define GDT_ENTRY_KERNEL_BASE   12
  

#define __KERNEL_CS 0x08

unsigned int cached_irq_mask = 0xffff;
#define __byte(x,y)             (((unsigned char *) &(y))[x])
#define cached_master_mask      (__byte(0, cached_irq_mask))
#define cached_slave_mask       (__byte(1, cached_irq_mask))
#define cached_A1	(__byte(1,cached_irq_mask))
#define cached_21	(__byte(0,cached_irq_mask))
irq_desc_t irq_desc[NR_IRQS]  = { [0 ... NR_IRQS-1] = { 0, 0, 0, 0, 0,0,0 }};

void mask_and_ack_8259A(unsigned int irq);
unsigned int startup_8259A_irq(unsigned int irq);

static void end_8259A_irq (unsigned int irq);
static struct hw_interrupt_type i8259A_irq_type = {
          "XT-PIC",
          startup_8259A_irq,
          NULL,		//shutdown_8259A_irq
          enable_8259A_irq,
          disable_8259A_irq,
          mask_and_ack_8259A,
          end_8259A_irq,
          NULL
  };
  enum { 
          GATE_INTERRUPT = 0xE, 
          GATE_TRAP = 0xF,        
          GATE_CALL = 0xC,
  }; 

void native_write_idt_entry(gate_desc *idt_, int entry, const gate_desc *gate)
 {
         memcpy(&idt_[entry], gate, sizeof(*gate));
 }
 
#define write_idt_entry(dt, entry, g)     native_write_idt_entry(dt, entry, g)
  
void pack_gate(gate_desc *gate, unsigned char type,
                               unsigned long base, unsigned dpl, unsigned flags,
                               unsigned short seg)
  {
          gate->a = (seg << 16) | (base & 0xffff);
          gate->b = (base & 0xffff0000) | (((0x80 | type | (dpl << 5)) & 0xff) << 8);
  }
  
  
  
void _set_gate(int gate, unsigned type, void *addr,
                              unsigned dpl, unsigned ist, unsigned seg)
 {
         gate_desc s;

         pack_gate(&s, type | 0x60, (unsigned long)addr, dpl, ist, seg);		//0x60 for usermode!
 
         write_idt_entry(idt, gate, &s);
    //     write_trace_idt_entry(gate, &s);
 }

#define set_intr_gate_(n, addr)                                          \
         do {                                                            \
                 _set_gate(n, GATE_INTERRUPT, (void *)addr, 0, 0,        \
                           0x08);                                 \
         } while (0)
  
void disable_8259A_irq(unsigned int irq)
  {
          unsigned int mask = 1 << irq;
  

          cached_irq_mask |= mask;
          if (irq & 8)
                  outb(cached_slave_mask, PIC_SLAVE_IMR);
          else
                 outb(cached_master_mask, PIC_MASTER_IMR);
         
 }
 
void enable_8259A_irq(unsigned int irq)
 {
         unsigned int mask = ~(1 << irq);
 
       
         cached_irq_mask &= mask;
         if (irq & 8)
                 outb(cached_slave_mask, PIC_SLAVE_IMR);
         else
                 outb(cached_master_mask, PIC_MASTER_IMR);
         
 }
unsigned int startup_8259A_irq(unsigned int irq)
  { 
          enable_8259A_irq(irq);
          return 0; /* never anything pending */
  }
  
static void end_8259A_irq (unsigned int irq)
{
	if (!(irq_desc[irq].status & (IRQ_DISABLED|IRQ_INPROGRESS)))
		enable_8259A_irq(irq);
} 
  
void init_8259A(int t)
 {
 
         outb(0xff, PIC_MASTER_IMR);     /* mask all of 8259A-1 */
         outb(0xff, PIC_SLAVE_IMR);      /* mask all of 8259A-2 */

         /*
          * outb_p - this has to work on a wide range of PC hardware.
          */
         outb_p(0x11, PIC_MASTER_CMD);   /* ICW1: select 8259A-1 init */
         outb_p(0x20 + 0, PIC_MASTER_IMR);       /* ICW2: 8259A-1 IR0-7 mapped to 0x20-0x27 */
         outb_p(1U << PIC_CASCADE_IR, PIC_MASTER_IMR);   /* 8259A-1 (the master) has a slave on IR2 */

         outb_p(MASTER_ICW4_DEFAULT, PIC_MASTER_IMR);

         outb_p(0x11, PIC_SLAVE_CMD);    /* ICW1: select 8259A-2 init */
         outb_p(0x20 + 8, PIC_SLAVE_IMR);        /* ICW2: 8259A-2 IR0-7 mapped to 0x28-0x2f */
         outb_p(PIC_CASCADE_IR, PIC_SLAVE_IMR);  /* 8259A-2 is a slave on master's IR2 */
         outb_p(SLAVE_ICW4_DEFAULT, PIC_SLAVE_IMR); /* (slave's support for AEOI in flat mode is to be investigated) */
 
 }


void init_ISA_irqs (void)
 {
         int i;

         init_8259A(1);
 
         for (i = 0; i < NR_IRQS; i++) {
                 irq_desc[i].status = IRQ_DISABLED;
                 irq_desc[i].action = NULL;
                 irq_desc[i].depth = 1;
 
                 if (i < 16) {
                         /*
                          * 16 old-style INTA-cycle interrupts:
                          */
                         irq_desc[i].handler = &i8259A_irq_type;
                 } else {
                         /*
                          * 'high' PCI IRQs filled in on demand
                          */
                         // irq_desc[i].handler = &no_irq_type;
                 }
         }
 }

/*
  * Internal function to register an irqaction.
  */
 int setup_irq(unsigned int irq, struct irqaction * new)
 {
         struct irq_desc *desc = irq_desc + irq;
         struct irqaction *old, **p;
         int shared = 0;

         p = &desc->action;
         if ((old = *p) != NULL) {
				printk("Added new interrupt to the irq queue\n");

                 do {
                         p = &old->next;
                         old = *p;
                 } while (old);        
         }

         *p = new;
			
         if (!shared) {
                 desc->depth = 0;
                 desc->status &= ~(IRQ_DISABLED | IRQ_AUTODETECT |
                                   IRQ_WAITING | IRQ_INPROGRESS);
                          
         }
         return 0;
 }
 //void null_handler(struct pt_regs *r) { /*for(;;); printk("null handler called"); */}
/* struct irqaction irq2  = { null_handler, 0, 0, "null", 0, 0,2};
 struct irqaction irq3  = { null_handler, 0, 0, "null", 0, 0,3};
 struct irqaction irq4  = { null_handler, 0, 0, "null", 0, 0,4};
 struct irqaction irq5  = { null_handler, 0, 0, "null", 0, 0,5};
 struct irqaction irq6  = { null_handler, 0, 0, "null", 0, 0,6};
 struct irqaction irq7  = { null_handler, 0, 0, "null", 0, 0,7};
 struct irqaction irq8  = { null_handler, 0, 0, "null", 0, 0,8}; 
  */
 
extern void irq1(void);
extern void irq0(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);


extern void irq14(void);
extern void irq15(void);
extern void irq16(void);
#define NR_IRQS 16
#define IRQ(x,y) \
	irq##x

#define IRQLIST_16(x) \
	IRQ(x,0) , IRQ(x,1), IRQ(x,2), IRQ(x,3), \
	IRQ(x,4), IRQ(x,5), IRQ(x,6), IRQ(x,7), \
	IRQ(x,8), IRQ(x,9), IRQ(x,a), IRQ(x,b), \
	IRQ(x,c), IRQ(x,d), IRQ(x,e), IRQ(x,f)

void (*interrupt[16])(void) = {
	irq0, irq1, irq2, irq3, irq4, irq5, irq6, irq7, irq8, irq9 ,irq10, irq11, irq12, irq13, irq14, irq15,
};

 #define NR_VECTORS 256
 #define SYSCALL_VECTOR          0x80
 #define FIRST_EXTERNAL_VECTOR   0x20

 void init_IRQ(void)
 {
         int i;
 
         /* all the set up before the call gates are initialised */
        init_ISA_irqs();


         /*
          * Cover the whole vector space, no vector can escape
          * us. (some of these will be overridden and become
          * 'special' SMP interrupts)
          */
       for (i = 0; i < (NR_VECTORS - FIRST_EXTERNAL_VECTOR); i++) {
                 int vector = FIRST_EXTERNAL_VECTOR + i;
                 if (i >= NR_IRQS)
                         break;
                 if (vector != SYSCALL_VECTOR) 
                         set_intr_gate_(vector, interrupt[i]);
         }
	  //_set_gate(14, GATE_INTERRUPT, ata_interrupt_handler,0,0,0x08);
    
       //  if (boot_cpu_data.hard_math && !cpu_has_fpu)
       //        setup_irq(FPU_IRQ, &fpu_irq);  
 }

void mask_and_ack_8259A(unsigned int irq)
{
	unsigned int irqmask = 1 << irq;


	/*
	 * Lightweight spurious IRQ detection. We do not want
	 * to overdo spurious IRQ handling - it's usually a sign
	 * of hardware problems, so we only do the checks we can
	 * do without slowing down good hardware unnecesserily.
	 *
	 * Note that IRQ7 and IRQ15 (the two spurious IRQs
	 * usually resulting from the 8259A-1|2 PICs) occur
	 * even if the IRQ is masked in the 8259A. Thus we
	 * can check spurious 8259A IRQs without doing the
	 * quite slow i8259A_irq_real() call for every IRQ.
	 * This does not cover 100% of spurious interrupts,
	 * but should be enough to warn the user that there
	 * is something bad going on ...
	 */
	if (cached_irq_mask & irqmask)
		goto spurious_8259A_irq;
	cached_irq_mask |= irqmask;

handle_real_irq:
	if (irq & 8) {
		inb(0xA1);		/* DUMMY - (do we need this?) */
		outb(cached_A1,0xA1);
		outb(0x60+(irq&7),0xA0);/* 'Specific EOI' to slave */
		outb(0x62,0x20);	/* 'Specific EOI' to master-IRQ2 */
	} else {
		inb(0x21);		/* DUMMY - (do we need this?) */
		outb(cached_21,0x21);
		outb(0x60+irq,0x20);	/* 'Specific EOI' to master */
	}
	
	return;

spurious_8259A_irq:
	
	{
		static int spurious_irq_mask;
		/*
		 * At this point we can be sure the IRQ is spurious,
		 * lets ACK and report it. [once per IRQ]
		 */
		if (!(spurious_irq_mask & irqmask)) {
		//	printk("spurious 8259A interrupt: IRQ%d.\n", irq);
			spurious_irq_mask |= irqmask;
		}

		goto handle_real_irq;
	}
}


void irq_ack(size_t irq_no) {
	if (irq_no >= 8) {
		outportb(0xA0, 0x20);
	}
	outportb(0x20, 0x20);
}

#include <task.h>
 unsigned int do_IRQ(u32 esp)
{	
		struct pt_regs *r = (struct pt_regs *)esp;
		if(task_switching)
			 esp = _task_switch(esp);
			 task_switching = 0;

		irq_desc_t *desc = irq_desc +  r->int_no - 32;
		esp = desc->action->handler((u32)esp);
		irq_ack(r->int_no - 32);

		return esp;
}
static unsigned char cache_21 = 0xff;
static unsigned char cache_A1 = 0xff;
 void unmask_irq(unsigned int irq_nr)
{
        unsigned char mask;

        mask = ~(1 << (irq_nr & 7));
        if (irq_nr < 8) {
                cache_21 &= mask;
                outb(cache_21,0x21);
        } else {
                cache_A1 &= mask;
                outb(cache_A1,0xA1);
        }
}
void mask_irq(unsigned int irq_nr)
{
        unsigned char mask;

        mask = 1 << (irq_nr & 7);
        if (irq_nr < 8) {
                cache_21 |= mask;
                outb(cache_21,0x21);
        } else {
                cache_A1 |= mask;
                outb(cache_A1,0xA1);
        }
}
void disable_irq(unsigned int irq_nr)
{

        mask_irq(irq_nr);

}

void enable_irq(unsigned int irq_nr)
{

  
        unmask_irq(irq_nr);

}
