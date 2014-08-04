#include <initrd.h>
#include <heapmngr.h>
#include <types.h>
#include <elf.h>
#include <sched.h>
#include <main.h>
#include <printk.h>
#include <timer.h>
initrd_header_t *initrd_header;     
initrd_file_header_t *file_headers; 
fs_node_t *initrd_root;             
fs_node_t *initrd_dev;              
fs_node_t *root_nodes;             
int nroot_nodes;                  

struct dirent dirent;

static u32 initrd_read(fs_node_t *node, u32 offset, u32 size, u8 *buffer)
{
    initrd_file_header_t header = file_headers[node->inode];	 
    if (offset > header.length)
        return 0;
    if (offset+size > header.length)
        size = header.length-offset;
    memcpy(buffer, (u8*) (header.offset+offset), size);
    return size;
}

static struct dirent *initrd_readdir(fs_node_t *node, u32 index)
{
    if (node == initrd_root && index == 0)
    {
      strcpy(dirent.name, "dev");
      dirent.name[3] = 0;
      dirent.ino = 0;
      return &dirent;
    }

    if ((int)index-1 >= nroot_nodes)
        return 0;
    strcpy(dirent.name, root_nodes[index-1].name);
    dirent.name[strlen(root_nodes[index-1].name)] = 0;
    dirent.ino = root_nodes[index-1].inode;
    return &dirent;
}

static fs_node_t *initrd_finddir(fs_node_t *node, char *name)
{
    if (node == initrd_root &&
        !strcmp(name, "dev") )
        return initrd_dev;

    int i;
    for (i = 0; i < nroot_nodes; i++)
        if (!strcmp(name, root_nodes[i].name))
            return &root_nodes[i];
    return 0;
}

fs_node_t *install_initrd(u32 location)
{
  	
    initrd_header = (initrd_header_t *)location;
    file_headers = (initrd_file_header_t *) (location+sizeof(initrd_header_t));	



    initrd_root = (fs_node_t*)malloc_(sizeof(fs_node_t));
    
	strcpy(initrd_root->name, "initrd");
	initrd_root->flags = FS_DIRECTORY;
  
    initrd_root->readdir = &initrd_readdir;		
    initrd_root->finddir = &initrd_finddir;
    

    initrd_dev = (fs_node_t*)malloc_(sizeof(fs_node_t));
    strcpy(initrd_dev->name, "dev");
    initrd_dev->flags = FS_DIRECTORY;
   
    initrd_dev->readdir = &initrd_readdir;
    initrd_dev->finddir = &initrd_finddir;
    

    root_nodes = (fs_node_t*)malloc_(sizeof(fs_node_t) * initrd_header->nfiles); 
    nroot_nodes = initrd_header->nfiles;

	int i;

   for (i = 0; i < (int)initrd_header->nfiles; i++)
   {
       file_headers[i].offset += location;
      
       strcpy(root_nodes[i].name, (char*)&file_headers[i].name);
       root_nodes[i].mask = root_nodes[i].uid = root_nodes[i].gid = 0;
       root_nodes[i].length = file_headers[i].length;
       root_nodes[i].inode = i;
       root_nodes[i].flags = FS_FILE;
       root_nodes[i].read = &initrd_read;
       root_nodes[i].write = 0;
       root_nodes[i].readdir = 0;
       root_nodes[i].finddir = 0;
       root_nodes[i].open = 0;
       root_nodes[i].close = 0;
       root_nodes[i].impl = 0;
   }
   return initrd_root;
} 
bool elf_exec__(const void* elf_program_buf, u32 elf_file_size, const char* programName,size_t argc, char** argv)
{
	

	printk("---> Starting load.");
		
	
	const char *elf_start = elf_program_buf;

	const elf_header_t* elf_header = elf_program_buf;
	
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


    memset((void*)ph->vaddr, 0, ph->filesz);

    memcpy((void*)ph->vaddr, elf_start+ph->offset, ph->filesz);


    
    uintptr_t heap = current->eip + elf_file_size;
	printk("heap %x\n",heap);
	heap_global        = heap; /* heap end */
	heap_actual_global = heap + (0x1000 - heap % 0x1000);


    printk("current->heap_global  %x\n", current->heap_global );
     printk("current->heap_actual_global %x\n", current->heap_actual_global);
	create_thread((void*)ph->vaddr/*,argv,envp */,0);
     

    
	return 1;
}

char * load_initrd_app(char *name, char **argv, char **env)
{
	
	int argc = 0;
	while (argv[argc]) { ++argc; }
		struct dirent* node = 0;
    for (size_t i = 0; (node = readdir_fs(fs_root_initrd, i)) != 0; ++i)
    {
		fs_node_t * fsnode = finddir_fs(fs_root_initrd, node->name);
        if ((fsnode->flags & 0x7) == FS_DIRECTORY)
        {
        }
        else
        {
			char * argv_[] = {
			name,
			NULL,
			NULL,
			NULL
		};
	int argc_ = 0;
	while (argv_[argc_]) {
		argc_++;
	}
				printk("\nSearching!\n");				
                char* buf = malloc_( fsnode->length);         
                printk("%d\n",fsnode->length);
                u32 sz =  read_fs(fsnode, 0, fsnode->length, buf);

                printk("Size: %d bytes\t Name: %s\n", fsnode->length, node->name);
                if(strcmp(node->name, name) == 0)
				{
					return buf;
					elf_exec__(buf, sz, name,argc,argv);
					memset(buf,0,400000);
				}

                
        }
          
    }

return 0;
}
