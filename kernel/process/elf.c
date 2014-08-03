#include <elf.h>
#include <task.h>
#include <fs.h>
#include <stat.h>
#include <types.h>
#include <sched.h>
#include <printk.h>
#include <heapmngr.h>
#include <mem.h>
#define O_RDONLY 0
typedef unsigned long uintptr_t;
void test_task___(void)
{

	printk("hello ");
	printk("how ");
	printk("are ");
	printk("you? \n");
	for(;;);
}
//page_directory_t* pd = 0;
u32 heap_actual_global;
u32 heap_global;
int load_elf( char *path, char *argv[], char *envp[])
{
	
	struct stat *statbuf = malloc_(sizeof(struct stat));
					
	stat(path, statbuf);
	u32 file_size = statbuf->st_size;
	free_(statbuf);
	printk("size of file : %d\n", file_size);
	 char *data = malloc_(file_size);
	
	int fd = kopen(path, O_RDONLY);
	if (fd < 0) {
		printk("elf_load(): unable to open %s\n", path);
	}
	
	//int i = 0;
	kread(fd, data, file_size);
	return 0;
    const char *elf_start = data;
//	const char *elf_end = elf_start + file_size;
	elf_header_t* elf_header = (elf_header_t*)data;
	
	char name[32] = {0};
	memcpy(name, elf_header->ident, 4);
	
	printk("IDENT: %s\n", name+1);
	printk("TYPE: %x\n", elf_header->type);
	printk("MACHINE %x\n", elf_header->machine);
	printk("VERSION %x\n", elf_header->version);
	
	
	const char* header_pos = elf_start + elf_header->phoff;
    program_header_t* ph = (program_header_t*)header_pos;

	const char* types[] = { "NULL", "Loadable Segment", "Dynamic Linking Information",
                                "Interpreter", "Note", "??", "Program Header" };
    printk(" %s\n offset: %x\n vaddr: %x\n paddr: %x\n filesz: %x\n memsz: %d\n flags: %x\n align: %x\n",
    types[ph->type], ph->offset, ph->vaddr, ph->paddr, ph->filesz, ph->memsz, ph->flags, ph->align);
    __asm__ __volatile__ ("cli");

  for (u32 i = 0x40000000; i < 0x41000000; i += 0x1000) {
		_vmmngr_alloc_frame(_vmm_get_page_addr(i, 1, current_directory),0, 1);
	}



    memset((void*)ph->vaddr, 0, ph->filesz);

    memcpy((void*)ph->vaddr, elf_start+ph->offset, ph->filesz);
   
    __asm__ __volatile__ ("sti");
    uintptr_t heap = current->eip + file_size;
    printk("heap %x\n",heap);
	heap_global        = heap; /* heap end */
	heap_actual_global = heap + (0x1000 - heap % 0x1000);

    printk("ph->vaddr %x\n", ph->vaddr);
	create_thread((void*)ph->vaddr/*,argv,envp */,0); // create_thread_u behöver 2 vanliga tasks för att funka
	
	free(data);
return 0;
}

 int sys_sbrk(int size) {
	 
	  printk("current->heap_global %d\n",heap_global);
	  printk("current->heap_actual_global %d\n",heap_actual_global);

	uintptr_t ret = heap_global;
	uintptr_t i_ret = ret;
	
	while (ret % 0x1000) {
		ret++;
	}
    heap_global += (ret - i_ret) + size;
	while (heap_global > heap_actual_global) {
		heap_actual_global += 0x1000;
		//assert(heap_actual_global % 0x1000 == 0);
	
		_vmmngr_alloc_frame( _vmm_get_page_addr(heap_actual_global, 1, current_directory), 0, 0);
	
	}
			printk("::sys_sbrk::\n");
	return ret;
}



int execve(char *name, char **argv, char **env)
{
 printk("todo");
 return 0;
}
