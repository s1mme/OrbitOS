#ifndef __SYSTEM_H
#define __SYSTEM_H
#define _KERNEL_
#include <types.h>



#define asm __asm__
#define volatile __volatile__

extern unsigned int __irq_sem;

#define IRQ_OFF { asm volatile ("cli"); }
#define IRQ_RES { asm volatile ("sti"); }
#define PAUSE   { asm volatile ("hlt"); }

#define STOP while (1) { PAUSE; }


#define SIGNAL_RETURN 0xFFFFDEAF
#define THREAD_RETURN 0xFFFFB00F

extern void * code;
//extern void * end;

extern char * boot_arg; 
extern char * boot_arg_extra; 
void itoa(int n, char s[]);




extern void return_to_userspace(void);


extern int max(int,int);
extern int min(int,int);
extern int abs(int);
extern void swap(int *, int *);
extern void *memcpy(void *restrict dest, const void *restrict src, size_t count);
extern void *memmove(void *restrict dest, const void *restrict src, size_t count);
extern void *memset(void *dest, int val, size_t count);
extern unsigned short *memsetw(unsigned short *dest, unsigned short val, int count);

extern char * strdup(const char *str);
extern char * strcpy(char * dest, const char * src);
extern int atoi(const char *str);
extern unsigned char inportb(unsigned short _port);
extern void outportw(u16 port, u16 val);
extern u16 inportw(u16 port);
extern unsigned short inports(unsigned short _port);
extern void outports(unsigned short _port, unsigned short _data);
extern unsigned int inportl(unsigned short _port);
extern void outportl(unsigned short _port, unsigned int _data);
extern void outportsm(unsigned short port, unsigned char * data, unsigned long size);
extern void inportsm(unsigned short port, unsigned char * data, unsigned long size);
extern int strcmp(const char *a, const char *b);
extern char * strtok_r(char * str, const char * delim, char ** saveptr);
extern size_t lfind(const char * str, const char accept);
extern size_t rfind(const char * str, const char accept);
extern size_t strspn(const char * str, const char * accept);
extern char * strpbrk(const char * str, const char * accept);
void putch(char c);
extern u32 strlen(const char *str);
extern char* dirname(char *path);
extern  char* basename(char *path);
extern void sti(void);
#define assert(statement) ((statement) ? (void)0 : assert_failed(__FILE__, __LINE__, #statement))
#endif
