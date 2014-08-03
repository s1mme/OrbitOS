#ifndef io_H
#define io_H
#include <types.h>
extern void outb_p( unsigned char data, unsigned short port);
extern unsigned char inb_p(unsigned short port);
extern void outw(unsigned short port, unsigned short val);
extern void outb( unsigned char data, unsigned short port);
extern unsigned char inb(unsigned short port);
extern u16 inw(u16 port);
extern void insw(unsigned long addr, void *buffer, int count);
extern  void outsw(unsigned long addr, const void *buffer, int count);
#endif
