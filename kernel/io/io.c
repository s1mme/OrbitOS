#include <types.h>
#include <timer.h>
void outb_p( unsigned char data, unsigned short port)
{	
sleep(1);
__asm__ __volatile__("outb %1, %0" : : "dN" (port), "a" (data));	
}


unsigned char inb_p(unsigned short port)
{
   unsigned char ret;
   __asm__ volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
   sleep(1);
   return ret;
}


void outw(unsigned short port, unsigned short val)
{
    __asm__ volatile ("out %%ax,%%dx" :: "a"(val), "d"(port));
}

void outb( unsigned char data, unsigned short port)
{	
__asm__ __volatile__("outb %1, %0" : : "dN" (port), "a" (data));	
}

unsigned char inb(unsigned short port)
{
   unsigned char ret;
   __asm__ volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}
u16 inw(u16 port)
{
    u16 ret_val;
    __asm__ volatile ("in %%dx,%%ax" : "=a" (ret_val) : "d"(port));
    return ret_val;
}
void insw(unsigned long addr, void *buffer, int count)
 {
         if (count) {
                 u16 *buf = buffer;
                 do {
                         u16 x = inw(addr);
                         *buf++ = x;
                 } while (--count);
         }
 }
 void outsw(unsigned long addr, const void *buffer, int count)
{
        if (count) {
                const u16 *buf = buffer;
                do {
                        outw(*buf++, addr);
                } while (--count);
        }
}
