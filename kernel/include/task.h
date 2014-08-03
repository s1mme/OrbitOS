#ifndef _TASK_H
#define _TASK_H
#define NR_TASKS        1024     /* Max 4092, or 4090 w/APM configured on x86 */
#include <mem.h>
#include <types.h>
#define MAX_TASKS_PER_USER (NR_TASKS/2)
#define MIN_TASKS_LEFT_FOR_ROOT 4
struct mm_struct {
	
	/*
         struct vm_area_struct *mmap;            // list of VMAs 
         struct vm_area_struct *mmap_avl;        // tree of VMAs 
         struct vm_area_struct *mmap_cache;      // last find_vma result 
         pgd_t * pgd;
         atomic_t count;
         int map_count;                          */// number of VMAs 
      //   struct semaphore mmap_sem;

         unsigned long context;

         unsigned long start_code, end_code, start_data, end_data;
         unsigned long start_brk, brk, start_stack;
         unsigned long arg_start, arg_end, env_start, env_end;
         unsigned long rss, total_vm, locked_vm;
         unsigned long def_flags;
         unsigned long cpu_vm_mask;
         unsigned long swap_cnt; // number of pages to swap on next pass 
         unsigned long swap_address; 
         /*
          * This is an architecture-specific pointer: the portable
          * part of Linux does not know about any segments.
          */
         void * segments;
 };


struct thread_struct {
         unsigned short  back_link,__blh;
         unsigned long   esp0;
         unsigned short  ss0,__ss0h;
         unsigned long   esp1;
         unsigned short  ss1,__ss1h;
         unsigned long   esp2;
         unsigned short  ss2,__ss2h;
         unsigned long   cr3;
         unsigned long   eip;
         unsigned long   eflags;
         unsigned long   eax,ecx,edx,ebx;
         unsigned long   esp;
         unsigned long   ebp;
         unsigned long   esi;
         unsigned long   edi;
         unsigned short  es, __esh;
         unsigned short  cs, __csh;
         unsigned short  ss, __ssh;
         unsigned short  ds, __dsh;
         unsigned short  fs, __fsh;
         unsigned short  gs, __gsh;
         unsigned short  ldt, __ldth;
         unsigned short  trace, bitmap;
   //      unsigned long   io_bitmap[IO_BITMAP_SIZE+1];
         unsigned long   tr;
         unsigned long   cr2, trap_no, error_code;
 /* floating point info */
      //   union i387_union i387;
 /* virtual 86 mode info */
       //  struct vm86_struct * vm86_info;
         unsigned long screen_bitmap;
         unsigned long v86flags, v86mask, v86mode;
 };
 struct wait_queue {
          struct task_struct * task;
          struct wait_queue * next;
  };
  
  #define WAIT_QUEUE_HEAD(x) ((struct wait_queue *)((x)-1))
  
  static inline void init_waitqueue(struct wait_queue **q)
  {
          *q = WAIT_QUEUE_HEAD(q);
  }
#define RLIM_NLIMITS	10

struct rlimit {
	long	rlim_cur;
	long	rlim_max;
};
#define NGROUPS		32
#define NR_OPEN 32
#define __NFDBITS       (8 * sizeof(unsigned long))
  #define __FD_SETSIZE    1024
  #define __FDSET_LONGS   (__FD_SETSIZE/__NFDBITS)
  

  
  typedef struct {
          unsigned long fds_bits [__FDSET_LONGS];
  } __kernel_fd_set;

typedef __kernel_fd_set         fd_set;              
/* Open file table structure */
 struct files_struct {
         int count;
         fd_set close_on_exec;
         fd_set open_fds;
         struct file * fd[NR_OPEN];
 };
 struct task_struct {
 /* these are hardcoded - don't touch */
         volatile long state;    /* -1 unrunnable, 0 runnable, >0 stopped */
         long counter;
         long priority;
         unsigned long signal;
         unsigned long blocked;  /* bitmap of masked signals */
         unsigned long flags;    /* per process flags, defined below */
         int errno;
         long debugreg[8];  /* Hardware debugging registers */
         struct exec_domain *exec_domain;
 /* various fields */
  //       struct linux_binfmt *binfmt;
         struct task_struct *next_task, *prev_task;
         struct task_struct *next_run,  *prev_run;
         unsigned long saved_kernel_stack;
         unsigned long kernel_stack_page;
         int exit_code, exit_signal;
         /* ??? */
         unsigned long personality;
         int dumpable:1;
         int did_exec:1;
         /* shouldn't this be pid_t? */
         int pid;
         int pgrp;
         int tty_old_pgrp;
         int session;
         /* boolean value for session group leader */
         int leader;
         int     groups[NGROUPS];
         /* 
          * pointers to (original) parent process, youngest child, younger sibling,
          * older sibling, respectively.  (p->father can be replaced with 
          * p->p_pptr->pid)
          */
         struct task_struct *p_opptr, *p_pptr, *p_cptr, *p_ysptr, *p_osptr;
         struct wait_queue *wait_chldexit;       /* for wait4() */
         unsigned short uid,euid,suid,fsuid;
         unsigned short gid,egid,sgid,fsgid;
         unsigned long timeout, policy, rt_priority;
         unsigned long it_real_value, it_prof_value, it_virt_value;
         unsigned long it_real_incr, it_prof_incr, it_virt_incr;
      //   struct timer_list real_timer;
         long utime, stime, cutime, cstime, start_time;
 /* mm fault and swap info: this can arguably be seen as either mm-specific or thread-specific */
         unsigned long min_flt, maj_flt, nswap, cmin_flt, cmaj_flt, cnswap;
         int swappable:1;
         unsigned long swap_address;
         unsigned long old_maj_flt;      /* old value of maj_flt */
         unsigned long dec_flt;          /* page fault count of the last time */
         unsigned long swap_cnt;         /* number of pages to swap on next pass */
 /* limits */
         struct rlimit rlim[RLIM_NLIMITS];
         unsigned short used_math;
         char comm[16];
 /* file system info */
   //      int link_count;
     //    struct tty_struct *tty; /* NULL if no tty */
 /* ipc stuff */
     //    struct sem_undo *semundo;
       //  struct sem_queue *semsleeping;
 /* ldt for this task - used by Wine.  If NULL, default_ldt is used */
       //  struct desc_struct *ldt;
 /* tss for this task */
     struct thread_struct tss;
 /* filesystem information */
       //  struct fs_struct *fs;
 /* open file information */
        struct files_struct *files;
 /* memory management info */
        // struct mm_struct *mm;
 /* signal handlers */
       //  struct signal_struct *sig;
 /* privileged execution counters, for exit_signal permission checking */
         int priv, ppriv;
         int stack;
		
		 int eip;
		 int ebp;
		 int esp;
		 int heap_global;
		 int heap_actual_global;
		 int file_size;
		  page_directory_t *page_directory;
 };

extern int nr_running;
extern volatile bool task_switching;
extern void task_initialize(void);
extern u32 _task_switch(u32 esp);
extern int get_pid(void);
#define KERNEL_STACK_SIZE  2*8192
extern void create_thread(void (*entry)(void), int type);
extern  int do_fork(void);
#endif
