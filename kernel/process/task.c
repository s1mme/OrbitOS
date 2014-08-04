#include <task.h>
#include <errno.h>
#include <printk.h>
#include <sched.h>
#include <system.h>
#include <types.h>
#include <resource.h>
#include <heapmngr.h>
#include <mem.h>
extern u32 initial_esp;

  struct task_struct * task[NR_TASKS] = {&init_task, };
  int nr_tasks=1;
  int nr_running=1;
  unsigned long int total_forks=0;        // Handle normal Linux uptimes. 
  int last_pid=0;
  static int find_empty_process(void)
  {
          int i;
  
          if (nr_tasks >= NR_TASKS - MIN_TASKS_LEFT_FOR_ROOT) {
                  if (current->uid)
                          return -EAGAIN;
          }
          if (current->uid) {
                  long max_tasks = current->rlim[RLIMIT_NPROC].rlim_cur;
  
                  max_tasks--;    /* count the new process.. */
                  if (max_tasks < nr_tasks) {
                          struct task_struct *p;
                          for_each_task (p) {
                                  if (p->uid == current->uid)
                                          if (--max_tasks < 0)
                                                  return -EAGAIN;
                          }
                  }
          }
          for (i = 0 ; i < NR_TASKS ; i++) {
                  if (!task[i])
                          return i;
          }
          return -EAGAIN;
  }

  int get_pid(void)
  {
          struct task_struct *p;
        //  if (flags & CLONE_PID)
           //       return current->pid;
  repeat:
          if ((++last_pid) & 0xffff8000)
                  last_pid=1;
              for_each_task (p) {
                 if (p->pid == last_pid ||
                      p->pgrp == last_pid ||
                      p->session == last_pid)
                          goto repeat;
          }
          return last_pid;
  }

volatile struct task_struct *ready_queue;
void task_initialize(void)
{
	//ready_queue = (struct task_struct*)malloc(sizeof(struct task_struct));
	sched_init();
	current->page_directory = kernel_directory;
	//current->stack = initial_esp;
	wake_up_process(current);
}

typedef unsigned long uintptr_t;
u32 volatile pid = 0;

 int exit____(void)
{
	printk("exit\n");
    
    return 0;
}
 struct task_struct init_task_2 = INIT_TASK_2;
void create_thread(void (*entry)(void), int type)
{
         int nr;
         struct task_struct *p;
			
         p = (struct task_struct *) kmalloc(sizeof(struct task_struct));
        // if (!p)
          //       goto bad_fork;
         memset(p, 0, sizeof(struct task_struct));     
         
         nr = find_empty_process();
         if (nr < 0)
			printk("no empty process\n");

      //  *p = init_task_2;	
		 
         p->swappable = 0;
         
         p->stack = (u32)kvmalloc(KERNEL_STACK_SIZE)+KERNEL_STACK_SIZE;
         *(unsigned long *) p->kernel_stack_page = STACK_MAGIC;
         p->state = TASK_UNINTERRUPTIBLE;
         p->pid = get_pid();
         
         
         p->next_run = NULL; 
         
         p->prev_run = NULL; 
         
		 //p->p_pptr = p->p_opptr = current;
         p->p_cptr = NULL;      
		 p->signal = 0;
		 
         task[nr] = p;
         SET_LINKS(p);		
         nr_tasks++;

         struct pt_regs *kernel_stack = (struct pt_regs*)kmalloc(sizeof(struct pt_regs));
         u32 code_segment, data_segment;
		 u32 eflags = 0x0202;
		
		 if(type)	//usermode
		 {
			 code_segment = 0x1B, data_segment = 0x23;
		 }
		 else       //kernel
		 {
			 code_segment = 0x08, data_segment = 0x10;	 
		 }
			
	 	 kernel_stack->ss = data_segment; 
	 	 
	     kernel_stack->useresp = KERNEL_STACK_SIZE;
	
		 kernel_stack->eflags = eflags;
		 kernel_stack->cs = code_segment;
		 kernel_stack->eip = (u32)entry;
		 kernel_stack->err_code = 0;
		 kernel_stack->int_no = 0;
		 kernel_stack->eax = 0;					//argc??
		 kernel_stack->ecx = 0;			//(uintptr_t)argv; ?
		 kernel_stack->edx = 0;
		 kernel_stack->ebx = 0;
		 kernel_stack->ebp = 0;
		 kernel_stack->esi = 0;
		 kernel_stack->edi = 0;

	 	
		 kernel_stack->ds = data_segment;
		 kernel_stack->es = data_segment;
		 kernel_stack->fs = data_segment;
	     kernel_stack->gs = data_segment;
	
	     p->stack = (u32)kernel_stack; 		 
 
         p->swappable = 1;
         p->counter = (current->counter >>= 1);
         wake_up_process(p); //lagra processen i en kö
         ++total_forks;	
         printk("pid:: %d",p->pid);
       //  free_(kernel_stack);
       //  free_(p->kernel_stack_page);
	   //  free_(p);
}

volatile bool task_switching = true;
int cnterr = 0;
u32 _task_switch(u32 esp)
{
	if(!current) return esp;
	current->stack = esp;
	struct task_struct* oldTask = current; 
	current = current->next_run;
	if(current == oldTask)
		return esp;
//	 current->state = TASK_RUNNING;
    current_directory = current->page_directory;
    struct pt_regs *r = (struct pt_regs *)esp;
	current->eip = r->eip;
	
	
	schedule();
	
	return current->stack;
}


 int do_fork(void)
 {

		 __asm__ __volatile__("cli");
         int nr;
         int error = -EINVAL;		//används tillsammans med goto
         struct task_struct *p;
							
         error = -ENOMEM;
         p = (struct task_struct *) malloc_(sizeof(*p));
         if (!p)
                 goto bad_fork;
                      
         error = -EAGAIN;
         
         nr = find_empty_process();
       //  if (nr < 0)
       //          goto bad_fork_free_stack;
 
         *p = *current;		//fundersam över denna *löst* använder swapper som current 
		 struct pt_regs *regs = (struct pt_regs *)(current->stack + sizeof(struct pt_regs));	
	           
         p->swappable = 0;
        
         p->kernel_stack_page = (u32)malloc_a(KERNEL_STACK_SIZE)+KERNEL_STACK_SIZE;
         *(unsigned long *) p->kernel_stack_page = STACK_MAGIC;
         p->state = TASK_UNINTERRUPTIBLE;
         p->pid = get_pid();
         p->next_run = NULL; //används av add_to_runqueue via wake_up_process 
         p->prev_run = NULL; 
         p->p_pptr = p->p_opptr = NULL; //används av SET_LINKS
         p->p_cptr = current;      
		 p->signal = 0;
		 
         task[nr] = p;
        // SET_LINKS(p);		//skapa länken för processen
         nr_tasks++;
 
         error = -ENOMEM;
      /*           TODO's copy :O            */   
      /* copy all the process information */
      //   if (copy_files(clone_flags, p))
      //           goto bad_fork_cleanup;
      //   if (copy_fs(clone_flags, p))
      //           goto bad_fork_cleanup_files;
      //    if (copy_sighand(clone_flags, p))
      //            goto bad_fork_cleanup_fs;
      //  if (copy_mm(p))						//klona adress fältet !
      //        goto bad_fork_cleanup_sighand;		 
#ifdef DEBUGGING		 
		 printk("\nEIP:    %04x:[<%08lx>]\nEFLAGS: %08lx\n",
                  regs->eip, regs->eflags);
         printk("eax: %08lx   ebx: %08lx   ecx: %08lx   edx: %08lx\n",
                 regs->eax, regs->ebx, regs->ecx, regs->edx);
         printk("esi: %08lx   edi: %08lx   ebp: %08lx ",
                 regs->esi, regs->edi, regs->ebp);
         printk("ds: %04x   es: %04x   ss: %04x\n",
                 regs->ds, regs->es , regs->ss);
#endif          		
         struct pt_regs *kernel_stack = (struct pt_regs*)malloc_(sizeof(struct pt_regs));
         u32 code_segment, data_segment;
		 u32 eflags = 0x0202;
   
	 	 kernel_stack->ss = 0x23; 
	     kernel_stack->useresp = regs->useresp;
		 code_segment = 0x1b; 
		
		 kernel_stack->eflags = eflags;
		 kernel_stack->cs = code_segment;
		 kernel_stack->eip = regs->eip;
		 kernel_stack->err_code = 0;
		 kernel_stack->int_no = 0;
		 kernel_stack->eax = 0;					//argc; return value??
		 kernel_stack->ecx = regs->ecx;			//(uintptr_t)argv; ?
		 kernel_stack->edx = regs->edx;
		 kernel_stack->ebx = regs->ebx;
		 kernel_stack->ebp = regs->ebp;
		 kernel_stack->esi = regs->esi;
		 kernel_stack->edi = regs->edi;

		 data_segment = 0x23;
	 	
		 kernel_stack->ds = data_segment;
		 kernel_stack->es = data_segment;
		 kernel_stack->fs = data_segment;
	     kernel_stack->gs = data_segment;
	
	     p->stack = (u32)kernel_stack; 
		 p->eip = regs->eip;
		 p->ebp = regs->ebp;
		
		// p->esp = (u32)kernel_stack;
         p->swappable = 1;
         p->counter = (current->counter >>= 1);

         page_directory_t *directory = clone_directory(kernel_directory); 
	     p->page_directory = directory;
	     
         switch_page_directory(p->page_directory);
        
         wake_up_process(p); //lagra processen i en kö
		__asm__ __volatile__("sti");
         ++total_forks;	
         return p->pid;		
 /*           TODO's to clean :O        */
 //bad_fork_cleanup_sighand:
 //        exit_sighand(p);
 //bad_fork_cleanup_fs:
 //        exit_fs(p);
 //bad_fork_cleanup_files:
 //        exit_files(p);
 //bad_fork_cleanup:
  //       task[nr] = NULL;
        // REMOVE_LINKS(p);
  //       nr_tasks--;
 //bad_fork_free_stack:
//         free_(new_stack);
// bad_fork_free_p:
 //        free_(p);
 bad_fork:
         return error;
 }
