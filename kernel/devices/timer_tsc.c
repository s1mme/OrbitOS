 #include <timer.h>
 #include <hw_irq.h>
 #include <irq.h>
 #include <types.h>
 #include <io.h>
 #include <printk.h>
 #include <system.h>
 #include <errno.h>
#define boot_cpu_has(bit)       test_bit(bit, boot_cpu_data.x86_capability)
#define cpu_has_tsc             boot_cpu_has(X86_FEATURE_TSC)
 #define LATCH (11932182/1000)
 #define HZ 100
  #define CALIBRATE_TIME  (5 * 1000020/HZ)
  #define CALIBRATE_LATCH	(5 * LATCH)

  #define rdtscl(low) \
       __asm__ __volatile__("rdtsc" : "=a" (low) : : "edx")
  #define rdtsc(low,high) \
     __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))
    void rep_nop(void)
 {
         __asm__ __volatile__("rep;nop": : :"memory");
 }
static void delay_tsc(unsigned long loops)
 {
         unsigned long bclock, now;
         
         rdtscl(bclock);
         do
         {
                 rep_nop();
                 rdtscl(now);
         } while ((now-bclock) < loops);
 }
/* tsc timer_opts struct */
 struct timer_opts timer_tsc = {
         .name = "tsc",
        .mark_offset = NULL, /* mark_offset_tsc todo */
        .get_offset = NULL, /* get_offset_tsc todo */
        .monotonic_clock = NULL, /* monotonic_clock_tsc todo */
         .delay = delay_tsc,
 };
 

 int test_bit(int nr, const unsigned long * addr)
 {
         int     mask;
 
         addr += nr >> 5;
         mask = 1 << (nr & 0x1f);
         return ((mask & *addr) != 0);
  }

#define CALIBRATE_LATCH (5 * LATCH)
  
 void mach_prepare_counter(void)
  {
         /* Set the Gate high, disable speaker */
          outb((inb(0x61) & ~0x02) | 0x01, 0x61);
  
          /*
           * Now let's take care of CTC channel 2
           *
           * Set the Gate high, program CTC channel 2 for mode 0,
           * (interrupt on terminal count mode), binary count,
           * load 5 * LATCH count, (LSB and MSB) to begin countdown.
           *
           * Some devices need a delay here.
           */
          outb(0xb0, 0x43);                       /* binary, mode 0, LSB/MSB, Ch 2 */
          outb_p(CALIBRATE_LATCH & 0xff, 0x42);   /* LSB of count */
          outb_p(CALIBRATE_LATCH >> 8, 0x42);       /* MSB of count */
  }
  
 void mach_countup(unsigned long *count_p)
  {
          unsigned long count = 0;
          do {
                  count++;
          } while ((inb_p(0x61) & 0x20) == 0);
          *count_p = count;
  }
  
  unsigned long calibrate_tsc(void)
  {
         mach_prepare_counter();
  
          {
                  unsigned long startlow, starthigh;
                  unsigned long endlow, endhigh;
                  unsigned long count;
  
                  rdtsc(startlow,starthigh);
                  mach_countup(&count);
                  rdtsc(endlow,endhigh);
  
  
                  /* Error: ECTCNEVERSET */
                  if (count <= 1)
                          goto bad_ctc;
  
                  /* 64-bit subtract - gcc just messes up with long longs */
                  __asm__ __volatile__("subl %2,%0\n\t"
                          "sbbl %3,%1"
                          :"=a" (endlow), "=d" (endhigh)
                          :"g" (startlow), "g" (starthigh),
                           "" (endlow), "1" (endhigh));
  
                  /* Error: ECPUTOOFAST */
                  if (endhigh)
                          goto bad_ctc;
  
                  /* Error: ECPUTOOSLOW */
                  if (endlow <= CALIBRATE_TIME)
                          goto bad_ctc;
  
                  __asm__ __volatile__("divl %2"
                          :"=a" (endlow), "=d" (endhigh)
                          :"r" (endlow), "" (0), "1" (CALIBRATE_TIME));
  
                  return endlow;
          }
  
          /*
           * The CTC wasn't reliable: we got a hit on the very first read,
           * or the CPU was so fast/slow that the quotient wouldn't fit in
           * 32 bits..
           */
  bad_ctc:
          return 0;
  }
unsigned long cpu_khz;
int init_tsc(int test)
{
//	if (cpu_has_tsc) {
                 unsigned long tsc_quotient;
                 int d;
	 while(d < 20)
	 {
                         tsc_quotient = calibrate_tsc();
                 

                 if (tsc_quotient) {
                        // fast_gettimeoffset_quotient = tsc_quotient;
                        // use_tsc = 1;
                         /*
                          *      We could be more selective here I suspect
                          *      and just enable this for the next intel chips ?
                          */
                         /* report CPU clock rate in Hz.
                          * The formula is (10^6 * 2^32) / (2^32 * 1 / (clocks/us)) =
                          * clock/second. Our precision is about 100 ppm.
                          */
                               unsigned long eax=0, edx=1000;
                                 __asm__ __volatile__("divl %2"
                                 :"=a" (cpu_khz), "=d" (edx)
                                 :"r" (tsc_quotient),
                                 "" (eax), "1" (edx));
                                 printk("Detected %lu.%03lu MHz processor.", cpu_khz / 1000, cpu_khz % 1000);
							
                         //set_cyc2ns_scale(cpu_khz/1000);
                         return cpu_khz;
                 }
                 d++;
       }
         return -ENODEV;
 }
 struct init_timer_opts  timer_tsc_init = {
         .init = &init_tsc,
        .opts = &timer_tsc,
 };
