#ifndef vsprintf_H
#define vsprintf_H

extern unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base);
extern int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);

#endif
