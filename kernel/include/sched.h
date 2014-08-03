#ifndef _I386_SCHED_H
#define _I386_SCHED_H
 #include <task.h>
#include <errno.h>
#include <printk.h>
#include <sched.h>
#include <system.h>
#include <resource.h>
 #define STACK_MAGIC     0xdeadbeef
  #define TASK_RUNNING            0
  #define TASK_INTERRUPTIBLE      1
  #define TASK_UNINTERRUPTIBLE    2
  #define TASK_ZOMBIE             3
  #define TASK_STOPPED            4
  #define TASK_SWAPPING           5
  
  /*
   * Scheduling policies
   */
  #define SCHED_OTHER             0
  #define SCHED_FIFO              1
  #define SCHED_RR                2
#define HZ 100
#define DEF_PRIORITY    (20*HZ/100)     /* 200 ms time slices */
 /*
  *  INIT_TASK is used to set up the first task table, touch at
  * your own risk!. Base=0, limit=0x1fffff (=2MB)
  */
 /* Prototype for an lcall7 syscall handler. */

typedef void (*lcall7_func)(struct pt_regs *);
void no_lcall7(struct pt_regs * regs);

/* Description of an execution domain - personality range supported,
 * lcall7 syscall handler, start up / shut down functions etc.
 * N.B. The name and lcall7 handler must be where they are since the
 * offset of the handler is hard coded in kernel/sys_call.S.
 */
struct exec_domain {
	const char *name;
	lcall7_func handler;
	unsigned char pers_low, pers_high;
	unsigned long * signal_map;
	unsigned long * signal_invmap;
	long *use_count;
	struct exec_domain *next;
}; 
  #define INIT_FILES  \
         1, \
         0,  \
          0,  \
         NULL,  \
 
  #define NOGROUP         (-1)
 #define INIT_TASK \
 /* state etc */ { 0,DEF_PRIORITY,DEF_PRIORITY,0,0,0,0, \
 /* debugregs */ { 0, },            \
 /* exec domain */&default_exec_domain,  \
 /* binfmt     NULL, \ */ \
 /* schedlink */ &init_task,&init_task, &init_task, &init_task, \
 /* stack */     0,(unsigned long) &init_kernel_stack, \
 /* ec,brk... */ 0,0,0,0,0, \
 /* pid etc.. */ 0,0,0,0,0, \
 /* suppl grps*/ {NOGROUP,}, \
 /* proc links*/ &init_task,&init_task,NULL,NULL,NULL,NULL, \
 /* uid etc */   0,0,0,0,0,0,0,0, \
 /* timeout */   0,SCHED_OTHER,0,0,0,0,0,0,0, \
 /* timer  */ /*{ NULL, NULL, 0, 0, it_real_fn }, */ \
 /* utime */     0,0,0,0,0, \
 /* flt */       0,0,0,0,0,0, \
 /* swp */       0,0,0,0,0, \
 /* rlimits */   INIT_RLIMITS, \
 /* math */      0, \
 /* comm */      "swapper",	\
 /* fs info *//*   0,NULL, */  \
 /* ipc */  /*     NULL, NULL,  */ \
 /* ldt */   /*    NULL, \ */    \
 /* tss */       {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, }, \
 /* fs */       /* &init_fs, \ */ \
 /* files */     0, \
 /* mm */       /* &init_mm, \ */ 	\
 /* signals */  /* &init_signals, \*/ 	\
 /* priv */      0, 0, \
 0, \
 0,0,0, \
 0,0,0,0 \
 }

#define INIT_TASK_2 \
 /* state etc */ { 0,DEF_PRIORITY,DEF_PRIORITY,0,0,0,0, \
 /* debugregs */ { 0, },            \
 /* exec domain */0,  \
 /* binfmt     NULL, \ */ \
 /* schedlink */ &init_task,&init_task, &init_task, &init_task, \
 /* stack */     0,(unsigned long) 0, \
 /* ec,brk... */ 0,0,0,0,0, \
 /* pid etc.. */ 0,0,0,0,0, \
 /* suppl grps*/ {NOGROUP,}, \
 /* proc links*/ &init_task,&init_task,NULL,NULL,NULL,NULL, \
 /* uid etc */   0,0,0,0,0,0,0,0, \
 /* timeout */   0,SCHED_OTHER,0,0,0,0,0,0,0, \
 /* timer  */ /*{ NULL, NULL, 0, 0, it_real_fn }, */ \
 /* utime */     0,0,0,0,0, \
 /* flt */       0,0,0,0,0,0, \
 /* swp */       0,0,0,0,0, \
 /* rlimits */   INIT_RLIMITS, \
 /* math */      0, \
 /* comm */      "new task!",	\
 /* fs info *//*   0,NULL, */  \
 /* ipc */  /*     NULL, NULL,  */ \
 /* ldt */   /*    NULL, \ */    \
 /* tss */       {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, }, \
 /* fs */       /* &init_fs, \ */ \
 /* files */     0, \
 /* mm */       /* &init_mm, \ */ 	\
 /* signals */  /* &init_signals, \*/ 	\
 /* priv */      0, 0, \
 0, \
 0,0,0, \
 0,0,0,0 \
 }
#define for_each_task(p) \
         for (p = &init_task ; (p = p->next_task) != &init_task ; )

#define SET_LINKS(p) do {  \
         (p)->next_task = &init_task; \
         (p)->prev_task = init_task.prev_task; \
         init_task.prev_task->next_task = (p); \
         init_task.prev_task = (p); \
         (p)->p_ysptr = NULL; \
         if (((p)->p_osptr = (p)->p_pptr->p_cptr) != NULL) \
                 (p)->p_osptr->p_ysptr = p; \
         (p)->p_pptr->p_cptr = p; \
         } while (0)

#define REMOVE_LINKS(p) do { unsigned long flags; \
         (p)->next_task->prev_task = (p)->prev_task; \
         (p)->prev_task->next_task = (p)->next_task; \
         if ((p)->p_osptr) \
                 (p)->p_osptr->p_ysptr = (p)->p_ysptr; \
         if ((p)->p_ysptr) \
                 (p)->p_ysptr->p_osptr = (p)->p_osptr; \
         else \
                 (p)->p_pptr->p_cptr = (p)->p_osptr; \
         } while (0)

extern  struct task_struct *current_set;
extern  struct task_struct init_task;
extern struct task_struct *task[NR_TASKS];
 #define current current_set
 
extern void wake_up_process(struct task_struct * p);
extern void sched_init(void);
extern void schedule(void);
extern void show_state(void);
#endif
