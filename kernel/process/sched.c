#include <task.h>
#include <errno.h>
#include <printk.h>
#include <sched.h>
#include <system.h>
#include <resource.h>
#include <gdt.h>
struct tq_struct {
	struct tq_struct *next;		/* linked list of active bh's */
	int sync;			/* must be initialized to zero */
	void (*routine)(void *);	/* function to call */
	void *data;			/* argument to function */
};

typedef struct tq_struct * task_queue;

#define DECLARE_TASK_QUEUE(q)  task_queue q = NULL


#define tas(ptr) (xchg((ptr),1))

struct __xchg_dummy { unsigned long a[100]; };
#define __xg(x) ((struct __xchg_dummy *)(x))

static unsigned long __xchg(unsigned long x, void * ptr, int size)
{
	switch (size) {
		case 1:
			__asm__("xchgb %b0,%1"
				:"=q" (x)
				:"m" (*__xg(ptr)), "0" (x)
				:"memory");
			break;
		case 2:
			__asm__("xchgw %w0,%1"
				:"=r" (x)
				:"m" (*__xg(ptr)), "0" (x)
				:"memory");
			break;
		case 4:
			__asm__("xchgl %0,%1"
				:"=r" (x)
				:"m" (*__xg(ptr)), "0" (x)
				:"memory");
			break;
	}
	return x;
}
#define xchg(ptr,x) ((__typeof__(*(ptr)))__xchg((unsigned long)(x),(ptr),sizeof(*(ptr))))
 void run_task_queue(task_queue *list)
{
	struct tq_struct *p;

	p = xchg(list,NULL);
	while (p) {
		void *arg;
		void (*f) (void *);
		struct tq_struct *save_p;
		arg    = p -> data;
		f      = p -> routine;
		save_p = p;
		p      = p -> next;
		save_p -> sync = 0;
		(*f)(arg);
	}
}
DECLARE_TASK_QUEUE(tq_scheduler);
 unsigned long ident_map[32] = {
	0,	1,	2,	3,	4,	5,	6,	7,
	8,	9,	10,	11,	12,	13,	14,	15,
	16,	17,	18,	19,	20,	21,	22,	23,
	24,	25,	26,	27,	28,	29,	30,	31
};

struct exec_domain default_exec_domain = {
	"Orbitos",	/* name */
	0,	/* lcall7 causes a seg fault. */
	0, 0xff,	/* All personalities. */
	ident_map,	/* Identity map signals. */
	ident_map,	/*  - both ways. */
	NULL,		/* No usage counter. */
	NULL		/* Nothing after this in the list. */
};

 unsigned long init_kernel_stack[1024] = { STACK_MAGIC, };
 struct task_struct init_task = INIT_TASK;

 int need_resched = 0;
 static void add_to_runqueue(struct task_struct * p)
 {

 #if 1   /* sanity tests */
         if (p->next_run || p->prev_run) {
                 printk("task already on run-queue\n");
                 return;
         }
 #endif
         if (p->policy != SCHED_OTHER || p->counter > current->counter + 3)
                 need_resched = 1;
         nr_running++;
         (p->prev_run = init_task.prev_run)->next_run = p;
         p->next_run = &init_task;
         init_task.prev_run = p;

}
 void del_from_runqueue(struct task_struct * p)
{
	struct task_struct *next = p->next_run;
	struct task_struct *prev = p->prev_run;

#if 1	/* sanity tests */
	if (!next || !prev) {
		printk("task not on run-queue\n");
		return;
	}
#endif
	if (p == &init_task) {
		static int nr = 0;
		if (nr < 5) {
			nr++;
			printk("idle task may not sleep\n");
		}
		return;
	}
	nr_running--;
	next->prev_run = prev;
	prev->next_run = next;
	p->next_run = NULL;
	p->prev_run = NULL;
}
void move_last_runqueue(struct task_struct * p)
{
	struct task_struct *next = p->next_run;
	struct task_struct *prev = p->prev_run;

 	/* remove from list */
	next->prev_run = prev;
	prev->next_run = next;
	/* add back to list */
	p->next_run = &init_task;
	prev = init_task.prev_run;
	init_task.prev_run = p;
	p->prev_run = prev;
	prev->next_run = p;

}



void wake_up_process(struct task_struct * p)
 {
     
         p->state = TASK_RUNNING;
         if (!p->next_run)
         {
			
                 add_to_runqueue(p);
			 }
 }
 struct task_struct *current_set;

     static int goodness(struct task_struct * p, struct task_struct * prev)
{
	int weight;
	/*
	 * Realtime process, select the first one on the
	 * runqueue (taking priorities within processes
	 * into account).
	 */
	if (p->policy != SCHED_OTHER)
		return 1000 + p->rt_priority;
	/*
	 * Give the process a first-approximation goodness value
	 * according to the number of clock-ticks it has left.
	 *
	 * Don't do any other calculations if the time slice is
	 * over..
	 */
	weight = p->counter;
	if (weight) {
		/* .. and a slight advantage to the current process */
		if (p == prev)
			weight += 1;
	}

	return weight;
}
 
void sched_init(void)
{	
	 current_set =&init_task; 
}
#define idle_task (&init_task)
#define SYMBOL_NAME_STR(X) #X
#define SYMBOL_NAME(X) X

/*/*
 *  'schedule()' is the scheduler function. It's a very simple and nice
 * scheduler: it's not perfect, but certainly works for most things.
 *
 * The goto is "interesting".
 *
 *   NOTE!!  Task 0 is the 'idle' task, which gets called when no other
 * tasks can run. It can not be killed, and it cannot sleep. The 'state'
 * information in task[0] is never used.
 */
 int intr_count = 0;
 void schedule(void)
{
	int c;
	struct task_struct * p;
	struct task_struct * prev, * next;



// check alarm, wake up any interruptible tasks that have got a signal 

//	sti();

	if (intr_count)
		goto scheduling_in_interrupt;

	/*if (bh_active & bh_mask) {
		intr_count = 1;
		do_bottom_half();
		intr_count = 0;
	}*/

	run_task_queue(&tq_scheduler);

	need_resched = 0;
	prev = current;
	
	// move an exhausted RR process to be last.. 
	if (!prev->counter && prev->policy == SCHED_RR) {
		prev->counter = prev->priority;
		move_last_runqueue(prev);
	}
	switch (prev->state) {
		case TASK_INTERRUPTIBLE:
			if (prev->signal & ~prev->blocked)
				goto makerunnable;

			//if (timeout && (timeout <= jiffies)) {
				//prev->timeout = 0;
			//	timeout = 0;
		makerunnable:
		
				prev->state = TASK_RUNNING;
				break;
			//}
		default:
			del_from_runqueue(prev);
			
		case TASK_RUNNING:;
	}
	p = init_task.next_run;

	c = -1000;
	next = idle_task;
	while (p != &init_task) {
			
		int weight = goodness(p, prev);
		if (weight > c)
			c = weight, next = p;
		p = p->next_run;
	}

	// if all runnable processes have "counter == 0", re-calculate counters 
	if (!c) {
		
		for_each_task(p)
			p->counter = (p->counter >> 1) + p->priority;
	}


	if (prev != next) {
	
		size_t t;
	__asm__ volatile ("mov %%cr0, %0" : "=r"(t));
	t |= 1 << 3;
	__asm__ volatile ("mov %0, %%cr0" :: "r"(t)); 
	set_kernel_stack(current->kernel_stack_page+KERNEL_STACK_SIZE); //update the tss entry when we changes stack!
}
	return;

scheduling_in_interrupt:
	printk("Aiee: scheduling in interrupt %p\n",
		__builtin_return_address(0));
}

void show_task(int nr,struct task_struct * p)
{
	printk("%-8s %3d ", p->comm, (p == current) ? -nr : nr);
}

void show_state(void)
{
	
	int i;
	for (i=0 ; i<NR_TASKS ; i++)
		if (task[i])
			show_task(i,task[i]);
}
