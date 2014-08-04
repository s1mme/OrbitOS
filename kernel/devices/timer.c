#include <timer.h>
#include <hw_irq.h>
#include <irq.h>
#include <types.h>
#include <system.h>
#include <io.h>
#include <task.h>
#include <processor.h>
#include <printk.h>
#include <heapmngr.h>
#include <simple_vga.h>
#include <console.h>
#define HZ 100

#define LATCH (11932182/1000)

struct cpuinfo_x86 boot_cpu_data;
#define current_cpu_data boot_cpu_data

 /* Waits for a given number of “loops” */
void delay_pit(unsigned long loops)
  {
          int d0;
          __asm__ __volatile__(
                  "\tjmp 1f\n"
                  ".align 16\n"
                  "1:\tjmp 2f\n"
                  ".align 16\n"
                  "2:\tdecl %0\n\tjns 2b"
                  :"=&a" (d0)
                  :"" (loops));
  }
struct timer_opts *cur_timer = NULL; //&timer_none;
  /* tsc timer_opts struct */



static void mark_offset_pit(void)
{
         /* nothing needed */
}
void udelay(unsigned long usecs)
{
		unsigned long loops;
		loops = (usecs*HZ*current_cpu_data.loops_per_jiffy)/1000000;
		cur_timer->delay(loops);
}
void ndelay(unsigned long nsecs)
{
		unsigned long loops;
		loops = (nsecs*HZ*current_cpu_data.loops_per_jiffy)/1000000000;
		cur_timer->delay(loops);
} 



/*PIT of IBM-compat-
ible PCs to issue timer interrupts on the IRQ0 at a (roughly) 1000-Hz frequency—
that is, once every 1 millisecond*/

int setup_pit_timer(int hz)
{
	u32 divisor = 1193180 / hz; //divisor must fit into 16 bits; PIT (programable interrupt timer)

    outportb(0x43, 0x34); // 0x34 -> Mode 2 : Rate Generator // idea of +gjm+

    // Send divisor
    outportb(0x40, (u8)(divisor      & 0xFF)); // low  byte
    outportb(0x40, (u8)((divisor>>8) & 0xFF)); // high byte
    return 0;
}
struct irqaction *timer;
/*static struct irqaction fpu_irq = { math_error_irq, 0, CPU_MASK_NONE, "fpu", NULL, NULL }; */

void time_init_hook_(void)
{

	timer = malloc_(sizeof(struct irqaction));
	timer->handler = timer_interrupt;
	timer->flags = 0;
	timer->mask = 0;
	timer->name = "timer";
	timer->dev_id = NULL;
	timer->next = NULL;
	timer->irq = 0;

	
	printk("irq: %d\n", timer->irq);
	printk("name: %s\n", timer->name);


	irq_desc[0].action = timer;

	setup_irq(0, irq_desc[0].action );
		
}


unsigned int timer_ticks = 0;
int cnt = 1;
irqreturn_t timer_interrupt(u32 esp)
 {

	// desc->handler->end(regs->int_no);
    //cur_timer->mark_offset();
	 int *test = (int *)0xb8000;
	 *test = cnt;
	 cnt++; 
	 
	 timer_ticks++;	
	
	 task_switching = 1;
	// update_cursor();
     return esp;
 }
 unsigned int gettickcount_(void)
{
	return timer_ticks;
}
 struct timer_opts timer_pit = {
         .name = "pit",
         .mark_offset = mark_offset_pit, 
         .get_offset = NULL, //get_offset_pit
         .monotonic_clock = NULL, //monotonic_clock_pit
         .delay = delay_pit,
 };
 
 struct init_timer_opts timer_pit_init = {
       .init = &setup_pit_timer, //init_pit 
       .opts = &timer_pit,
 };
 
static struct init_timer_opts* timers[] = {

#ifdef CONFIG_X86_PM_TIMER
         &timer_pmtmr_init,
#endif
          &timer_tsc_init,
          &timer_pit_init,
          NULL,
  };
struct timer_opts* select_timer(void)
  {
          int i = 0;
          
         /* find most preferred working timer */
         while (timers[i]) {
                 if (timers[i]->init)
                         if (timers[i]->init(1) == 0)
                                 return timers[i]->opts;
                 ++i;
         }
                 
     //    panic("select_timer: Cannot find a suitable timer\n");
         return NULL;
 }
void time_init(void)
{
	setup_pit_timer(100);
	//cur_timer = select_timer();
		//printk(KERN_INFO "Using %s for high-res timesource\n",cur_timer->name);
	time_init_hook_();

}
int wait_loop0 = 100;
int wait_loop1 = 60;
void
sleep( int seconds )
{   // this function needs to be finetuned for the specific microprocessor
    int i, j, k;
    for(i = 0; i < seconds; i++)
    {
        for(j = 0; j < wait_loop0; j++)
        {
            for(k = 0; k < wait_loop1; k++)
            {   // waste function, volatile makes sure it is not being optimized out by compiler
                int volatile t = 120 * j * i + k;
                t = t + 5;
            }
        }
    }
}
