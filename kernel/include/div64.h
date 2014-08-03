#ifndef __I386_DIV64
#define __I386_DIV64

#define do_div(n,base) ({ \
	unsigned long __upper, __low, __high, __mod; \
	__asm__ volatile("":"=a" (__low), "=d" (__high):"A" (n)); \
	__upper = __high; \
	if (__high) { \
		__upper = __high % (base); \
		__high = __high / (base); \
	} \
	__asm__ volatile("divl %2":"=a" (__low), "=d" (__mod):"rm" (base), "0" (__low), "1" (__upper)); \
	__asm__ volatile("":"=A" (n):"a" (__low),"d" (__high)); \
	__mod; \
})

#endif
