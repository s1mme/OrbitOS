#include <timer.h>
#include <hw_irq.h>
#include <irq.h>
#include <types.h>
#include <system.h>
#include <printk.h>
#include <heapmngr.h>
u8 key_result;
u8 keyboard_layout_us[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	
  '9', '0', '-', '=', '\b',	
  '\t',			
  'q', 'w', 'e', 'r',	
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	
    '^',		
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
 '\'', '`',   0,		
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			
  'm', ':', '.', '/',   0,				
  '*',
    0,	
  ' ',	
    0,	
    0,	
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	
    0,	
    0,	
    0,	
    0,	
    0,	
  '-',
    0,	
    0,
    0,
  '+',
    0,	
    0,	
    0,	
    0,	
    0,	
    0,   0,   0,
    0,	
    0,	
    0,	
};

int cnt____;
u8 scan_code;
irqreturn_t keyboard_interrupt(u32 esp)
 {
    //cur_timer->mark_offset();

	scan_code = inportb(0x60);
	if(scan_code & 0x80)
	{
	
		return esp;
		
	}
	else
		key_result = keyboard_layout_us[scan_code];
		
	printk("%c", key_result);



    return esp;
 }


struct irqaction *keyb;

void _kbd_init_hook(void)
{
	
	keyb = malloc_(sizeof(struct irqaction));
	keyb->handler = keyboard_interrupt;
	keyb->flags = 0;
	keyb->mask = 0;
	keyb->name = "keyboard";
	keyb->dev_id = NULL;
	keyb->next = NULL;
	keyb->irq = 1;

	
	printk("irq: %d\n", keyb->irq);
	printk("name: %s\n", keyb->name);


	irq_desc[1].action = keyb;
	//irq_desc[1].action = &irq1;
	setup_irq(1, irq_desc[1].action ); 
		//irq_install_handler(1, keyboard_interrupt);	
}


int getch_char;
u8 getch_polling(void)
{

	 if(keyboard_layout_us[inportb(0x60)] != 0) 
		outportb(0x60,0xf4); 
     
     while(keyboard_layout_us[inportb(0x60)] == 0); 
	 getch_char = keyboard_layout_us[inportb(0x60)];
     outportb(0x60,0xf4); 
	 
     return key_result; 
	
}
