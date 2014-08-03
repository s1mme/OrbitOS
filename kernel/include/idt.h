#ifndef _ASM_IDT_H
#define _ASM_IDT_H
#include <types.h>
struct gate_struct {          
          u16 offset_low;
          u16 segment; 
          unsigned ist : 3, zero0 : 5, type : 5, dpl : 2, p : 1;
          u16 offset_middle;
          u32 offset_high;
          u32 zero1; 
  } __attribute__((packed));


/* 8 byte segment descriptor */
struct desc_struct {
    union {
        struct {
            unsigned int a;
            unsigned int b;
        };
        struct {
            u16 limit0;
            u16 base0;
            unsigned base1: 8, type: 4, s: 1, dpl: 2, p: 1;
            unsigned limit: 4, avl: 1, l: 1, d: 1, g: 1, base2: 8;
        };
    };
} __attribute__((packed));
typedef struct desc_struct gate_desc;


struct idt_ptr
{
	u16 limit;
	u32 base; 
}__attribute__((packed));
extern gate_desc idt[256];
extern void idt_install(void);
#endif
