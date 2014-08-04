#ifndef lfb_H
#define lfb_H
#include <types.h>
extern void
graphics_install_bochs(u16 resolution_x, u16 resolution_y);
extern uintptr_t lfb_get_address(void);
#endif
