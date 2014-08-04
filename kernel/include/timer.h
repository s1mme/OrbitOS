#ifndef _ASM_TIMER_H
#define _ASM_TIMER_H
struct timer_opts {
          const char* name;
          void (*mark_offset)(void);
          unsigned long (*get_offset)(void);
          unsigned long long (*monotonic_clock)(void);
          void (*delay)(unsigned long);
  };
  
  struct init_timer_opts {
          int (*init)(int);
          struct timer_opts *opts;
  };
extern  struct init_timer_opts  timer_tsc_init;
extern void sleep( int seconds );
extern  unsigned int gettickcount_(void);
#endif
