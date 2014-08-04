#include <elf.h>
#include <task.h>
#include <fs.h>
#include <stat.h>
#include <types.h>
#include <sched.h>
#include <printk.h>
#include <heapmngr.h>
#include <mem.h>
#include <initrd.h>

typedef unsigned long uintptr_t;
void test_task___(void)
{

	printk("hello ");
	printk("how ");
	printk("are ");
	printk("you? \n");
	for(;;);
}
u16 checksum(char *addr, u32 count)
{
  u32 sum = 0;

  // Main summing loop
  while(count > 1)
  {
    sum += *((u16 *)addr);
	addr++;
    count = count - 2;
  }

  // Add left-over byte, if any
  if (count > 0)
    sum = sum + *((char *) addr);

  // Fold 32-bit sum to 16 bits
  while (sum>>16)
    sum = (sum & 0xFFFF) + (sum >> 16);

  return(~sum);
}
//page_directory_t* pd = 0;
u32 heap_actual_global;
u32 heap_global;
int load_elf(  char *path, char *argv[], char *envp[])
{
	
	struct stat *statbuf = malloc_(sizeof(struct stat));
					
	stat(path, statbuf);
	u32 file_size = statbuf->st_size;
	free_(statbuf);
	
	printk("size of file : %d\n", file_size);

	 
	
	int fd = kopen((char*)path, O_RDONLY);
	if (fd < 0) {
		printk("elf_load(): unable to open %s\n", path);
	}
	char *data = (char*)kmalloc(file_size);
	
	sys_read(fd, data, file_size);
    
	char *data2 =  load_initrd_app((char*)path,0,0);
	u16 checksum__ = checksum(data,280825);
	printk("checksum__ %04xd\n", checksum__);
	
	 for (int i=512; i < (int)file_size; i += 512) {
  if(checksum(data, i) != checksum(data2, i)) { printk("error at i = %d!\n",i); printk("."); }
}
	
    char *elf_start = data;

	elf_header_t* elf_header = (elf_header_t*)data;
	
	char name[32] = {0};
	memcpy(name, elf_header->ident, 4);
	
	printk("IDENT: %s\n", name+1);
	printk("TYPE: %x\n", elf_header->type);
	printk("MACHINE %x\n", elf_header->machine);
	printk("VERSION %x\n", elf_header->version);
	
	
	char* header_pos = elf_start + elf_header->phoff;
    program_header_t* ph = (program_header_t*)header_pos;

	const char* types[] = { "NULL", "Loadable Segment", "Dynamic Linking Information",
                                "Interpreter", "Note", "??", "Program Header" };
    printk(" %s\n offset: %x\n vaddr: %x\n paddr: %x\n filesz: %x\n memsz: %d\n flags: %x\n align: %x\n",
    types[ph->type], ph->offset, ph->vaddr, ph->paddr, ph->filesz, ph->memsz, ph->flags, ph->align);
    __asm__ __volatile__ ("cli");




    memset((void*)ph->vaddr, 0, ph->filesz);

    memcpy((void*)ph->vaddr, elf_start+ph->offset, ph->filesz);
   
    __asm__ __volatile__ ("sti");
    uintptr_t heap = current->eip + file_size;
    printk("heap %x\n",heap);
	heap_global        = heap; /* heap end */
	heap_actual_global = heap + (0x1000 - heap % 0x1000);

    printk("ph->vaddr %x\n", ph->vaddr);
	create_thread((void*)ph->vaddr/*,argv,envp */,0); 


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
	
		_vmmngr_alloc_frame( _vmm_get_page_addr(heap_actual_global, 1, kernel_directory), 0, 1);
	
	}
			printk("::sys_sbrk::\n");
	return ret;
}



int execve(char *name, char **argv, char **env)
{
 printk("todo");
 return 0;
}
