#ifndef serial_H
#define serial_H

extern void serial_string(int device, char * out);
extern void serial_init_hook_(void);
extern void serial_install(void);
#endif
