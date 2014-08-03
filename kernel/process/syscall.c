#include <types.h>
#include <task.h>
#include <fs.h>
#include <ext2.h>
#include <sched.h>
#include <errno.h>
#include <stat.h>
#include <elf.h>
#include <irq.h>
#include <keyboard.h>

static int write_(int fd, char * ptr, int len) {

		for (u32 i = 0; i < (u32)len; ++i) {
			printk("%c", ptr[i]);
		}
		
		return len;
}

int restart(void) {printk("REBOOTING"); /* outportb(0x64, 0xFE); */ return 0;}	
int sys_signal(u32 signum, uintptr_t handler) {
	printk("signum %d handler %d\n",signum, handler);
if (signum > 38) {
return 1;
}

//uintptr_t old = current_process->signals.functions[signum];
//current_process->signals.functions[signum] = handler;
return (int)1;
}
#define S_IFCHR  0020000
int fstat(int fd, struct stat* st) {
    printk ( "SYSCALL : fstat(%d, %p)\n", fd, st);
    if ( st == NULL ) return -1;
    st->st_mode = S_IFCHR; // | S_IWUSR;
                           //    st->st_blksize = 1;
    return 0;
}   

int ioctl(int fildes, int request, void *arg)
{
return 0;	
}                
static uintptr_t syscalls[] = {
    (uintptr_t)&restart,              /* 0 */
	(uintptr_t)&write_,
	(uintptr_t)&sys_open,
	(uintptr_t)&sys_read,
	(uintptr_t)&write_,               /* 4 */
	(uintptr_t)&sys_close,		
	(uintptr_t)&restart, 			 
	(uintptr_t)&restart,
	(uintptr_t)&restart,              /* 8 */
	(uintptr_t)&get_pid,		
	(uintptr_t)&sys_sbrk,
	(uintptr_t)&restart,
	(uintptr_t)&restart,              /* 12 */
	(uintptr_t)&restart,
	(uintptr_t)&sys_lseek,
	(uintptr_t)&fstat,
	(uintptr_t)&restart,			  /* 16 */
	(uintptr_t)&restart,             
	(uintptr_t)&restart,
	(uintptr_t)&restart,
	(uintptr_t)&restart,              /* 20 */
	(uintptr_t)&restart,
	(uintptr_t)&restart,
	(uintptr_t)&restart,             
	(uintptr_t)&restart,			  /* 24 */
	(uintptr_t)&restart,
	(uintptr_t)&restart,              
	(uintptr_t)&restart,
	(uintptr_t)&restart,			  /* 28 */
	(uintptr_t)&restart,              
	(uintptr_t)&restart,
	(uintptr_t)&restart,
	(uintptr_t)&restart,              /* 32 */
	(uintptr_t)&restart,
	(uintptr_t)&restart,
	(uintptr_t)&restart,              
	(uintptr_t)&restart,			  /* 36 */
	(uintptr_t)&restart,
	(uintptr_t)&restart,              
	(uintptr_t)&restart,
	(uintptr_t)&restart,			  /* 40 */
	(uintptr_t)&restart,              
	(uintptr_t)&restart,
	(uintptr_t)&restart,
	(uintptr_t)&restart,              /* 44 */
	(uintptr_t)&restart,
	(uintptr_t)&restart,
	(uintptr_t)&restart,              
	(uintptr_t)&restart,			  /* 48 */
	(uintptr_t)&restart,
	(uintptr_t)&restart,              
	(uintptr_t)&restart,
	(uintptr_t)&restart,			  /* 52 */
};
 extern void _isr127(void);
 void syscall_handler(u32 esp)
{
	struct pt_regs *r = (struct pt_regs *)esp;
/*
printk("r->eax %d\n",r->eax);
printk("r->ecx %d\n",r->ecx);
printk("r->edx %d\n",r->edx);
printk("r->ebx %d\n",r->ebx);
printk("r->ebp %d\n",r->ebp);
printk("r->esi %d\n",r->esi);
printk("r->edi %d\n",r->edi);
printk("r->eip %x\n",r->eip); */
    if (r->eax >= sizeof(syscalls)/sizeof(*syscalls))
        return;

  	
    void* addr = (void*)syscalls[r->eax];

   int ret;
   asm volatile (" \
     push %1; \
     push %2; \
     push %3; \
     push %4; \
     push %5; \
     call *%6; \
     pop %%ebx; \
     pop %%ebx; \
     pop %%ebx; \
     pop %%ebx; \
     pop %%ebx; \
   " : "=a" (ret) : "r" (r->edi), "r" (r->esi), "r" (r->edx), "r" (r->ecx), "r" (r->ebx), "r" (addr));
   r->eax = ret;
        	

}

void syscalls_install(void) {
		
	_set_gate(0x7F, 0xF, _isr127,0,0,0x08);
	//_syscall_init_hook();
}

