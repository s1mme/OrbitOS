#ifndef _ASM_IRQ_H
#define _ASM_IRQ_H
 #include <types.h>
typedef unsigned int irqreturn_t;
#define IRQ_INPROGRESS  1       /* IRQ handler active - do not enter! */
  #define IRQ_DISABLED    2       /* IRQ disabled - do not enter! */
  #define IRQ_PENDING     4       /* IRQ pending - replay on enable */
  #define IRQ_REPLAY      8       /* IRQ has been replayed but not acked yet */
  #define IRQ_AUTODETECT  16      /* IRQ is being autodetected */
  #define IRQ_WAITING     32      /* IRQ not yet seen - for autodetection */
  #define IRQ_LEVEL       64      /* IRQ level triggered */
  #define IRQ_MASKED      128     /* IRQ masked - shouldn't be seen again */
  #define IRQ_PER_CPU     256     /* IRQ is per CPU */
#define IRQ_HANDLED     (1)
struct irqaction {
          irqreturn_t (*handler)(u32 esp);
          unsigned long flags;
          cpumask_t mask;
          const char *name;
          void *dev_id;
          struct irqaction *next;
          int irq;
//         struct proc_dir_entry *dir;
  };


/*
   * Interrupt controller descriptor. This is all we need
   * to describe about the low-level hardware. 
   */
  struct hw_interrupt_type {
          const char * typename;
          unsigned int (*startup)(unsigned int irq);
          void (*shutdown)(unsigned int irq);
          void (*enable)(unsigned int irq);
          void (*disable)(unsigned int irq);
          void (*ack)(unsigned int irq);
          void (*end)(unsigned int irq);
          void (*set_affinity)(unsigned int irq, cpumask_t dest);
  };
  
  typedef struct hw_interrupt_type  hw_irq_controller;
  
  typedef struct irq_desc {
          hw_irq_controller *handler;
          void *handler_data;
          struct irqaction *action;       /* IRQ action list */
          unsigned int status;            /* IRQ status */
          unsigned int depth;             /* nested irq disables */
          unsigned int irq_count;         /* For detecting broken interrupts */
          unsigned int irqs_unhandled;
       //   spinlock_t lock;
  }  irq_desc_t;
  irqreturn_t keyboard_interrupt(u32 esp);

irqreturn_t timer_interrupt(u32 esp);
  extern irq_desc_t irq_desc [NR_IRQS];
  typedef void (*irq_handler_t) (struct pt_regs *);
void irq_install_handler(int irq, irq_handler_t handler);
void irq_ack(size_t irq_no);
extern u32 irq_handler(u32 esp);
void irq_install___(void);
void time_init(void);
extern  void init_IRQ(void);
extern int setup_irq(unsigned int irq, struct irqaction * new);
extern void _set_gate(int gate, unsigned type, void *addr,
                              unsigned dpl, unsigned ist, unsigned seg);

#endif
