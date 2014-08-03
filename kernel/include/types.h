#ifndef TYPES_H_
#define TYPES_H_
typedef unsigned long 	cpumask_t;
#define NR_IRQS 16
#define SA_INTERRUPT	0x20000000
#define asmlinkage CPP_ASMLINKAGE __attribute__((regparm(0)))
#define __sti()			__asm__ __volatile__("sti": : :"memory")
#define __cli() 		__asm__ __volatile__("cli": : :"memory")

typedef char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16;
typedef int s32;
typedef unsigned int u32;
typedef long long s64;
typedef unsigned long long u64;


void outportb(u16 port, u8 data);
#ifndef __bool_true_false_are_defined
  typedef _Bool             bool;
  #define true  1
  #define false 0
  #define __bool_true_false_are_defined 1
#endif

#define va_start(v,l) __builtin_va_start(v,l)
#define va_arg(v,l)   __builtin_va_arg(v,l)
#define va_end(v)     __builtin_va_end(v)
#define va_copy(d,s)  __builtin_va_copy(d,s)
typedef __builtin_va_list va_list;
extern void panic_assert(const char *file, u32 line, const char *desc);
#define ASSERT(b) ((b) ? (void)0 : panic_assert(__FILE__, __LINE__, #b))
/*struct pt_regs {
	long ebx;
	long ecx;
	long edx;
	long esi;
	long edi;
	long ebp;
	long eax;
	int  xds;
	int  xes;
	long orig_eax;
	long eip;
	int  xcs;
	long eflags;
	long esp;
	int  xss;
}__attribute__((packed)); */

typedef int size_t;
struct pt_regs {
	unsigned int gs, fs, es, ds;
	unsigned int edi, esi, ebp,  ebx, edx, ecx, eax;
	unsigned int int_no, err_code;
	unsigned int eip, cs, eflags, useresp, ss;
}__attribute__((packed));


#define NULL ((void*)0)
#define	EFAULT		14	/* Bad address */
#define   ENOSYS          38

 #define	EINVAL		22	/* Invalid argument */
#define ASSERT(b) ((b) ? (void)0 : panic_assert(__FILE__, __LINE__, #b))
#define BIT(n) (1<<(n))
#endif 
