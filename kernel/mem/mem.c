#include <heapmngr.h>
#include <types.h>
#include <mem.h>
#include <multiboot.h>
#include <printk.h>
#include <system.h>
#include <irq.h>
#define KERNEL_HEAP_INIT 0x02000000
#define KERNEL_HEAP_END  0x20000000
extern int copy_page_physical(int x, int y);
extern void *end;
u32 placement_pointer = (u32)&end;
u32 heap_end = (u32)NULL;
page_directory_t *kernel_directory = 0;
page_directory_t *current_directory = 0;
void
page_fault(struct pt_regs *r);


u32 paging_getPhysAddr(void* virtAddress)
{
    page_directory_t* pd = kernel_directory;


    u32 pagenr = (u32)virtAddress / 0x1000;
    page_table_t* pt = pd->tables[pagenr/1024];


    return ((u32)pt->pages[pagenr%1024]&0xFFFF000) + (((u32)virtAddress)&0x00000FFF);
}

		
#define PCI_MEM_START ((unsigned char*)0x0d0000000)
#define PCI_MEM_END ((unsigned char*)0x100000000) 

enum MEM_FLAGS {MEM_KERNEL = 0, MEM_PRESENT = 1, MEM_WRITE = 2, MEM_USER = 4};
void* physicaltovirt(unsigned int physAddress, unsigned int numberOfPages)
{
    static unsigned char* virtAddress = PCI_MEM_START;
    unsigned char* retVal = 0;
    

    for (unsigned int i=0; i<numberOfPages; i++)
    {
        if (virtAddress == PCI_MEM_END)
        {
        
       //   debug_print(CRITICAL, "ERROR translating phys to virtual!");
       
        }

        unsigned int pagenr = (unsigned int)virtAddress/0x1000;

        kernel_directory->tables[pagenr/1024]->pages[pagenr%1024] = (page_t *)((u32)physAddress | MEM_PRESENT | MEM_WRITE);
		
        if (i==0)
        {
            retVal = virtAddress;
        }

        virtAddress += 0x1000;
        physAddress += 0x1000;
    }


    return (unsigned int*)retVal; 
}
void
kmalloc_startat(
		u32 address
		) {
	placement_pointer = address;
}

/*
 * kmalloc() is the kernel's dumb placement allocator
 */
u32
kmalloc_real(
		size_t size,
		int align,
		u32 * phys
		) {
	if (heap_end) {
		void * address;
		if (align) {
			address = malloc(size);
		} else {
			address = malloc(size);
		}
		if (phys) {
			page_t *page = _vmm_get_page_addr((u32)address, 0, kernel_directory);
			*phys = page->frame * 0x1000 + ((u32)address & 0xFFF);
		}
		return (u32)address;
	}

	if (align && (placement_pointer & 0xFFFFF000)) {
		placement_pointer &= 0xFFFFF000;
		placement_pointer += 0x1000;
	}
	if (phys) {
		*phys = placement_pointer;
	}
	u32 address = placement_pointer;
	placement_pointer += size;
	return (u32)address;
}
/*
 * Normal
 */
u32
kmalloc(
		size_t size
		) {
	return kmalloc_real(size, 0, NULL);
}
/*
 * Aligned
 */
u32
kvmalloc(
		size_t size
		) {
	return kmalloc_real(size, 1, NULL);
}
/*
 * With a physical address
 */
u32
kmalloc_p(
		size_t size,
		u32 *phys
		) {
	return kmalloc_real(size, 0, phys);
}
/*
 * Aligned, with a physical address
 */
u32
kvmalloc_p(
		size_t size,
		u32 *phys
		) {
	return kmalloc_real(size, 1, phys);
}



u32 *frames;
u32 nframes;

#define INDEX_FROM_BIT(b) (b / 0x20)
#define OFFSET_FROM_BIT(b) (b % 0x20)

static void set_frame(u32 frame_adress)
{
u32 index = INDEX_FROM_BIT(frame_adress);
u32 offset = OFFSET_FROM_BIT(frame_adress);
frames[index] |= (0x1 << offset);

}

u32 _pmm_find_first_free_frame_addr(void) {
	u32 i = 0;
	u32 j;
	//u32 index = num_pages / 32;	
	while(i < INDEX_FROM_BIT(nframes))
	{	
		if(frames[i] != 0xffffffff)
		{
			for(j = 0; j < 32; j++)
			{
				if(!(frames[i]&(0x1 << j)))
					return i*32+j;	/*address of free frame*/
			}		
		}	
	i++;
	}

//	kprintf("\033[1;37;41mWARNING: System claims to be out of usable memory, which means we probably overwrote the page frames.\033[0m\n");

	for(;;);
}



void
_vmmngr_alloc_frame(
		page_t *page,
		int is_kernel,
		int is_writeable
		) {
	if(page->frame != 0)
		return;
	else
		{	
			u32 idx = _pmm_find_first_free_frame_addr();
		//	if(idx == (u32)-1)
			//	kprintf("No free frames");
			page->frame = idx;
			page->present = 1;
		page->user = (is_kernel)?0:1;
			page->rw = (is_writeable)?1:0;
			set_frame(idx);
	//frames[idx/32] |= (0x1 << idx%32); 
		}
}

void
dma_frame(
		page_t *page,
		int is_kernel,
		int is_writeable,
		u32 address
		) {
	/* Page this address directly */
	page->present = 1;
	page->rw      = (is_writeable) ? 1 : 0;
	page->user    = (is_kernel)    ? 0 : 1;
	page->frame   = address / 0x1000;
}

int GPF(struct pt_regs * regs)
{
	printk("GPF ERROR");
	  
 
        
              //  current->comm, current->pid,nr_running, 4096+(unsigned long)current);
	for(;;);
}
extern void isr13(void);
extern void isr14(void);


		

void
vmmngr_initialize(u32 memsize) {
	//printk("[INFO] size of RAM: %d kb\n",memsize);
	nframes = 2072192000 / 4; //2072192000
	frames  = (u32 *)kmalloc(INDEX_FROM_BIT(nframes * 8));
	memset(frames, 0, INDEX_FROM_BIT(nframes));
	//debug_print(NOTICE,"Starting frame allocation\n");
	u32 phys;
	kernel_directory = (page_directory_t *)kvmalloc_p(sizeof(page_directory_t),&phys);
	memset(kernel_directory, 0, sizeof(page_directory_t));


	
	for (u32 i = 0; i < placement_pointer + 0xf00000; i += 0x1000) {
		_vmmngr_alloc_frame(_vmm_get_page_addr(i, 1, kernel_directory), 0, 1);
	}
	/* XXX VGA TEXT MODE VIDEO MEMORY EXTENSION */
	for (u32 j = 0xb8000; j < 0xc0000; j += 0x1000) {
		_vmmngr_alloc_frame(_vmm_get_page_addr(j, 1, kernel_directory), 0, 1);
	}
	
	kernel_directory->physical_address = (u32)kernel_directory->physical_tables;

	/* Kernel Heap Space */
	for (u32 i = placement_pointer; i < KERNEL_HEAP_INIT; i += 0x1000) {
		_vmmngr_alloc_frame(_vmm_get_page_addr(i, 1, kernel_directory), 0, 1);
	}
	
	/* And preallocate the page entries for all the rest of the kernel heap as well */
	for (u32 i = KERNEL_HEAP_INIT; i < KERNEL_HEAP_END; i += 0x1000) {
		_vmm_get_page_addr(i, 1, kernel_directory);
	}


	switch_page_directory(kernel_directory);
	
	_set_gate(14, 0xF, isr14,0,0,0x08);
	_set_gate(13, 0xF, isr13,0,0,0x08);
}

void
switch_page_directory(
		page_directory_t * dir
		) {
	current_directory = dir;
	__asm__ __volatile__ ("mov %0, %%cr3":: "r"(dir->physical_address));
	u32 cr0;
	__asm__ __volatile__  ("mov %%cr0, %0": "=r"(cr0));
	cr0 |= 0x80000000;
	__asm__ __volatile__  ("mov %0, %%cr0":: "r"(cr0));
}

page_t *
_vmm_get_page_addr(
		u32 address,
		int make,
		page_directory_t * dir
		) {
	address /= 0x1000;
	u32 table_index = address / 1024;
	if (dir->tables[table_index]) {
		return (page_t *)&dir->tables[table_index]->pages[address % 1024];
	} else if(make) {
		u32 temp;
		dir->tables[table_index] = (page_table_t *)kvmalloc_p(sizeof(page_table_t), (u32 *)(&temp));
		memset(dir->tables[table_index], 0, sizeof(page_table_t));
		dir->physical_tables[table_index] = temp | 0x7; /* Present, R/w, User */
		return (page_t *)&dir->tables[table_index]->pages[address % 1024];
	} else {
		return 0;
	}
}

static const char *exception_messages_[32] = {
	"Division by zero",				/* 0 */
	"Debug",
	"Non-maskable interrupt",
	"Breakpoint",
	"Detected overflow",
	"Out-of-bounds",				/* 5 */
	"Invalid opcode",
	"No coprocessor",
	"Double fault",
	"Coprocessor segment overrun",
	"Bad TSS",						/* 10 */
	"Segment not present",
	"Stack fault",
	"General protection fault",
	"Page fault",
	"Unknown interrupt",			/* 15 */
	"Coprocessor fault",
	"Alignment check",
	"Machine check",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved"
};
void
page_fault(
		struct pt_regs *r)  {
	
   unsigned int faulting_address;
  	__asm__ __volatile__ ("mov %%cr2, %0" : "=r" (faulting_address));
   
  // int present  = !(r->err_code & 0x1); 
   int rw = r->err_code & 0x2;           
   int us = r->err_code & 0x4;           
   int reserved = r->err_code & 0x8;     


   if(rw)
		printk("Read-only ");
   if(us)
		printk("User-mode ");
   if(reserved)
		printk("Reserved ");
 	printk("\nUnhandled exception: [%d] %s", r->int_no, exception_messages_[r->int_no]);
 	printk("\n faulting_address : %x", faulting_address);
 	for(;;);

}
static page_table_t *clone_table(page_table_t *src, u32 *physAddr)
{
    // Make a new page table, which is page aligned.
    page_table_t *table = (page_table_t*)kvmalloc_p(sizeof(page_table_t), physAddr);
    // Ensure that the new table is blank.
    memset(table, 0, sizeof(page_directory_t));

    int i;
    for (i = 0; i < 1024; i++)
    {

        if (src->pages[i]->frame)
        {
            _vmmngr_alloc_frame((page_t *)&table->pages[i], 0, 1);

            if (src->pages[i]->present) table->pages[i]->present = 1;
            if (src->pages[i]->rw) table->pages[i]->rw = 1;
            if (src->pages[i]->user) table->pages[i]->user = 1;
            if (src->pages[i]->accessed) table->pages[i]->accessed = 1;
            if (src->pages[i]->dirty) table->pages[i]->dirty = 1;

            copy_page_physical(src->pages[i]->frame*0x1000, table->pages[i]->frame*0x1000);
        }
    }
    return table;
}

page_directory_t *clone_directory(page_directory_t *src)
{
    u32 phys;
    // Make a new page directory and obtain its physical address.
    page_directory_t *dir = (page_directory_t*)kvmalloc_p(sizeof(page_directory_t), &phys);
    // Ensure that it is blank.
    memset(dir, 0, sizeof(page_directory_t));


    u32 offset = (u32)dir->physical_tables - (u32)dir;

    dir->physical_address = phys + offset;

    int i;
    for (i = 0; i < 1024; i++)
    {
        if (!src->tables[i])
            continue;

        if (kernel_directory->tables[i] == src->tables[i])
        {

            dir->tables[i] = src->tables[i];
            dir->physical_tables[i] = src->physical_tables[i];
        }
        else
        {


            dir->tables[i] = clone_table(src->tables[i], &phys);
            dir->physical_tables[i] = phys | 0x07;
        }
    }
    return dir;
}


