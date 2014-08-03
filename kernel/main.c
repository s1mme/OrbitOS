#include <types.h>
#include <multiboot.h>
#include <setup.h>
#include <mem.h>
#include <heapmngr.h>
#include <console.h>
#include <irq.h>
#include <keyboard.h>
#include <fpu.h>
#include <task.h>
#include <syscall.h>
#include <initrd.h>
#include <pci.h>
#include <hardisk.h>
#include <ext2.h>
#include <fs.h>
#include <printk.h>
#include <sched.h>
void test_task(void)
{
	printk("teeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeest");
	//outportb(0x64, 0xFE);
	for(;;);
}
fs_node_t *fs_root_initrd;
void start_kernel(struct multiboot *mbp, unsigned int magic,u32 esp)
 {
	 u32 initrd_location = *((u32*)mbp->mods_addr);
	 u32 initrd_end = *(u32*)(mbp->mods_addr+4);

	 __asm__ __volatile__("cli");
	 cpu_init();
	 placement_pointer = initrd_end;
	 vmmngr_initialize(mbp->mem_upper + mbp->mem_lower);	
	
	 kheap = _heapmngr_initialize(0x3000000, 0x3900000, 0x2000);
	    
	 con_init();

	 init_IRQ();
	 time_init();
    
     _kbd_init_hook();
     /*setup_irq(2, &irq2); 
     setup_irq(3, &irq3 ); 
     setup_irq(4, &irq4 ); 
     setup_irq(5, &irq5 ); 

     setup_irq(8, &irq8 );
	*/
	 auto_fpu();
	 task_initialize();
	 syscalls_install();
	 fs_root_initrd = install_initrd(initrd_location);
	//create_thread(test_task);
	 
	 pci_inst_check();
	 enable_pci_master(0,3,0);		//8139 need this
     struct request *info = 0;
	 probe_ide(info);
	 hd_init_hook_();
     
    //serial_install();

    //unsigned long cpu_khz = init_tsc();
   	//u32 *pf = (u32 *)0xffff0000;
	//*pf = 10;
	ext2_read_superblock();
	register_filesystem();
	__asm__ __volatile__("sti");
	
	while(1)
	{
	if(getch_polling() == 'i')
		create_thread(test_task,0);
		show_state();
	}
}
