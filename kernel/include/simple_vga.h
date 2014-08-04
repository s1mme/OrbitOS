#ifndef simple_H
#define simple_H
extern void update_cursor(void);
extern int stdio_read__(int fd, char *buf, int length);
extern void putch(char c);
extern void init_video_term(void);
#endif
