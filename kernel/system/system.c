#include <printk.h>
#include <heapmngr.h>
#include <types.h>

 char * strcat(char * dest,const char * src)
{
int d0, d1, d2, d3;
__asm__ __volatile__(
	"repne\n\t"
	"scasb\n\t"
	"decl %1\n"
	"1:\tlodsb\n\t"
	"stosb\n\t"
	"testb %%al,%%al\n\t"
	"jne 1b"
	: "=&S" (d0), "=&D" (d1), "=&a" (d2), "=&c" (d3)
	: "0" (src), "1" (dest), "2" (0), "3" (0xffffffffu):"memory");
return dest;
}

int memcmp(const void *lhs, const void *rhs, size_t count) {
const u8 *us1 = (u8 *)lhs;
const u8 *us2 = (u8 *)rhs;
while (count-- != 0) {
if (*us1 != *us2)
return (*us1 < *us2) ? -1 : 1;
us1++, us2++;
}

return 0;
}

void *memcpy(void *dest,const void *src,size_t n) { 
  u32 num_dwords = n/4;
  u32 num_bytes = n%4;
  u32 *dest32 = (u32*)dest;
  u32 *src32 = (u32*)src;
  u8 *dest8 = ((u8*)dest)+num_dwords*4;
  u8 *src8 = ((u8*)src)+num_dwords*4;
  u32 i;

  for (i=0;i<num_dwords;i++) {
    dest32[i] = src32[i];
  }
  for (i=0;i<num_bytes;i++) {
    dest8[i] = src8[i];
  }
  return dest;
}


int max(int a, int b) {
	return (a > b) ? a : b;
}

int min(int a, int b) {
	return (a > b) ? b : a;
}

int abs(int a) {
	return (a >= 0) ? a : -a;
}

void swap(int *a, int *b) {
	int t = *a;
	*a = *b;
	*b = t;
}

void * memmove(void * restrict dest, const void * restrict src, size_t count) {
	size_t i;
	unsigned char *a = dest;
	const unsigned char *b = src;
	if (src < dest) {
		for ( i = count; i > 0; --i) {
			a[i-1] = b[i-1];
		}
	} else {
		for ( i = 0; i < count; ++i) {
			a[i] = b[i];
		}
	}
	return dest;
}

int strcmp(const char * a, const char * b) {
	u32 i = 0;
	while (1) {
		if (a[i] < b[i]) {
			return -1;
		} else if (a[i] > b[i]) {
			return 1;
		} else {
			if (a[i] == '\0') {
				return 0;
			}
			++i;
		}
	}
}


void * memset(void * b, int val, size_t count) {
	__asm__ __volatile__ ("cld; rep stosb" : "+c" (count), "+D" (b) : "a" (val) : "memory");
	return b;
}

unsigned short * memsetw(unsigned short * dest, unsigned short val, int count) {
	int i = 0;
	for ( ; i < count; ++i ) {
		dest[i] = val;
	}
	return dest;
}

u32 strlen(const char *str) {
	int i = 0;
	while (str[i] != (char)0) {
		++i;
	}
	return i;
}

char * strdup(const char *str) {
	int len = strlen(str);
	char * out = malloc_(sizeof(char) * (len+1));
    
	memcpy(out, str, len+1);
    
	return out;
}

char * strcpy(char * dest, const char * src) {
	int len = strlen(src);
	memcpy(dest, src, len+1);
	return dest;
}


int atoi(const char * str) {
	u32 len = strlen(str);
	u32 out = 0;
	u32 i;
	u32 pow = 1;
	for (i = len; i > 0; --i) {
		out += (str[i-1] - 48) * pow;
		pow *= 10;
	}
	return out;
}



void outportsm(unsigned short port, unsigned char * data, unsigned long size) {
	__asm__ __volatile__ ("rep outsw" : "+S" (data), "+c" (size) : "d" (port));
}


void inportsm(unsigned short port, unsigned char * data, unsigned long size) {
	__asm__ __volatile__ ("rep insw" : "+D" (data), "+c" (size) : "d" (port) : "memory");
}

void outportb(u16 port, u8 data)
{	
__asm__ __volatile__("outb %1, %0" : : "dN" (port), "a" (data));	
}

u8 inportb(u16 port)
{
   u8 ret;
   __asm__ volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}
void outports(u16 port, u16 val)
{
    __asm__ volatile ("out %%ax,%%dx" :: "a"(val), "d"(port));
}
void outportw(u16 port, u16 val)
{
    __asm__ volatile ("out %%ax,%%dx" :: "a"(val), "d"(port));
}

void outportl(u16 port, u32 val)
{
    __asm__ volatile ("outl %%eax,%%dx" : : "a"(val), "d"(port));
}

u16 inports(u16 port)
{
    u16 ret_val;
    __asm__ volatile ("in %%dx,%%ax" : "=a" (ret_val) : "d"(port));
    return ret_val;
}

u16 inportw(u16 port)
{
    u16 ret_val;
    __asm__ volatile ("in %%dx,%%ax" : "=a" (ret_val) : "d"(port));
    return ret_val;
}
u32 inportl(u16 port)
{
    u32 ret_val;
    __asm__ volatile ("in %%dx,%%eax" : "=a" (ret_val) : "d"(port));
    return ret_val;
}


char * strstr(const char * haystack, const char * needle) {
	const char * out = NULL;
	const char * ptr;
	const char * acc;
	const char * p;
	size_t s = strlen(needle);
	for (ptr = haystack; *ptr != '\0'; ++ptr) {
		size_t accept = 0;
		out = ptr;
		p = ptr;
		for (acc = needle; (*acc != '\0') && (*p != '\0'); ++acc) {
			if (*p == *acc) {
				accept++;
				p++;
			} else {
				break;
			}
		}
		if (accept == s) {
			return (char *)out;
		}
	}
	return NULL;
}
int strncmp(const char *s1, const char *s2, size_t n) {
        unsigned char uc1, uc2;
        if (n == 0 || s1 == NULL || s2 == NULL)
                return 0;
        /* Loop, comparing bytes. */
        while (n-- > 0 && *s1 == *s2) {
                if (n == 0 || *s1 == '\0')
                        return 0;
                s1++, s2++;
        }

        uc1 = (*(unsigned char *) s1);
        uc2 = (*(unsigned char *) s2);

        return ((uc1 < uc2) ? -1 : (uc1 > uc2));
}
void sti(void)
{
	__asm__ __volatile__("sti");
}

void cli(void)
{
	__asm__ __volatile__("cli");
}
 /* reverse:  reverse string s in place */
 void reverse(char s[])
 {
     int i, j;
     char c;
 
     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
 }

 /* itoa:  convert n to characters in s */
 void itoa(int n, char s[])
 {
     int i, sign;
 
     if ((sign = n) < 0)  /* record sign */
         n = -n;          /* make n positive */
     i = 0;
     do {       /* generate digits in reverse order */
         s[i++] = n % 10 + '0';   /* get next digit */
     } while ((n /= 10) > 0);     /* delete it */
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
 }
 
 const char* basename(char *path)
{
	char *p;
	if( path == NULL || *path == '\0' )
		return ".";
	p = path + strlen(path) - 1;
	while( *p == '/' ) {
		if( p == path )
			return path;
		*p-- = '\0';
	}
	while( p >= path && *p != '/' )
		p--;
	return p + 1;
}

const char* dirname(char *path)
{
	char *p;
	if( path == NULL || *path == '\0' )
		return ".";
	p = path + strlen(path) - 1;
	while( *p == '/' ) {
		if( p == path )
			return path;
		*p-- = '\0';
	}
	while( p >= path && *p != '/' )
		p--;
	return
		p < path ? "." :
		p == path ? "/" :
		(*p = '\0', path);
}
void assert_failed(const char *file, u32 line, const char *desc) {

	printk("Kernel Assertion Failed: %s\n", desc);
	printk("File: %s\n", file);
	printk("Line: %d\n", line);
	for(;;);
}
