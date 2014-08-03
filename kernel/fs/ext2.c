#include <ext2.h>
#include <types.h>
#include <hdregs.h>
#include <fs.h>
#include <printk.h>
#include <heapmngr.h>
#include <system.h>
#include <sched.h>
#include <hardisk.h>
u32 find_inode(u32 inode);
u32 inode_count = 0;
u32 inodes_per_group = 0;
u32 s_log_block_size;
u32 super_s_log_block_size;
int ext2_lseek(struct inode *inode, struct file * file, int offset, int whence);
struct ext2_super_block * ext2_read_superblock(void)
{
	int nr_sectors = 2;
	int start_sector = 2;
	struct ext2_super_block *buf = malloc(sizeof(struct ext2_super_block));
	read_disc_sector(start_sector,nr_sectors,buf);
	
	printk("s_inodes_count %d\n", buf->s_inodes_count);
	printk("s_blocks_count %d\n", buf->s_blocks_count);
	printk("s_free_blocks_count %d\n", buf->s_free_blocks_count);
	printk("s_log_frag_size %d\n", buf->s_log_frag_size);
	printk("s_creator_os %x\n", buf->s_creator_os);
	printk("s_free_inodes_count %d\n", buf->s_free_inodes_count);
	printk("s_frags_per_group %d\n",buf->s_frags_per_group);
	printk("s_blocks_per_group %d\n", buf->s_blocks_per_group);
	printk("s_magic %x\n", buf->s_magic);
	printk("s_inodes_per_group %d\n", buf->s_inodes_per_group);
	super_s_log_block_size = buf->s_log_block_size;
    s_log_block_size = 1024 << buf->s_log_block_size;
    printk("s_log_block_size %d\n", s_log_block_size);
	inode_count = buf->s_inodes_count;
	inodes_per_group = buf->s_inodes_per_group;
	
	return buf;
}
unsigned int inode_contain(u32 start_inode,u32 inode);

u32 BGDT(u32 blockgroup, u32 inode)
{
	printk("test");
	struct ext2_group_desc *p = malloc(512);
	read_disc_sector(4,1,p);
	p += blockgroup;

	printk("bg_block_bitmap %d\n", p->bg_block_bitmap);
	printk("bg_inode_bitmap %d\n", p->bg_inode_bitmap);
	printk("bg_inode_table %d\n", p->bg_inode_table); 

	printk("bg_free_blocks_count %d\n", p->bg_free_blocks_count);
	printk("bg_free_inodes_count %d\n", p->bg_free_inodes_count);
	printk("bg_used_dirs_count %d\n", p->bg_used_dirs_count);
	
	return inode_contain(p->bg_inode_table, inode);	
}

u32 find_inode(u32 inode)
{
	u32 block_group;
	block_group = (inode -1 ) / (inodes_per_group);
	return block_group;
}


u32 what_i_inside_grp(u32 inode)
{
	u32 index;
	index = (inode - 1)%(inodes_per_group); 
	return index;  
    u32 blocksize = 1024;
    u32 containing_block;    
    u32 inode_size = 128;
    containing_block = (index * inode_size) / blocksize; 
    return containing_block;
}
u32 file_size;
u32 block_id_s;
u32 block_id_d;
u32 block_id_t;
u32 i_block[11];
unsigned int inode_contain(u32 start_inode, u32 inode_number)
{
	int nr_sectors = 0;
	int i;
	struct ext2_inode  * buf;
	u32 counter =0;
	u32 sector_i = 512 / 128 ; 
	
	while(nr_sectors <= (int)inode_count)
	{		
		buf = malloc(nr_sectors*512*22);		
		read_disc_sector((start_inode*2)+nr_sectors,2,buf);		    
		for(i = 0; i < (int)sector_i; i++)
		{	
			u32 local_inode_index = what_i_inside_grp(inode_number);
			if(buf->i_size )
			{
					while(i < 15)
					{
						printk("i_block[0] %d\n", buf->i_block[i]);
						block_id_s = buf->i_block[12];
						block_id_d = buf->i_block[13];
						block_id_t = buf->i_block[14];
						i++;
					}					
			}
				if(counter  == local_inode_index )
				{
					for( i = 0; i < 11; i++)
						i_block[i] = buf->i_block[i];
					file_size = buf->i_size;
					return  buf->i_block[0];
				}

			counter++;																									    				
			
			buf = buf + 1;				
		}	
		free(buf);
		nr_sectors++;
	}
	return 0;
}

void ext2_output_( unsigned char * buf)
{
	u32 nr_sectors = file_size / 512;
	if(nr_sectors == 0)
		nr_sectors = 1;
	if(nr_sectors % file_size != 0)
		nr_sectors++;
	if (buf !=0 ) {
 
		int i = 0;
		for ( int c = 0; c < (int)nr_sectors; c++ ) {
			
			for (int j = 0; j <= 512; j++)
			{
				if(buf[i+j] != 0)
				printk ("%c", buf[ i + j]);
			}
			i += 512;	
		}	
	}
}

struct ext2_dir_entry * parse_dir(struct ext2_dir_entry * dir,char * file_name)
{
		while(dir->rec_len > 0)
	{	    
		char name[EXT2_NAME_LEN + 1] = {0};
		memcpy(name, dir->name, dir->name_len);
		printk("%s     %d\n",  name, /*dir->rec_len, */ dir->inode);
		
		if(strcmp(name, file_name) == 0)
		{
			printk("inode %d\n", dir->inode);
			printk("\n");
			return dir;
		}	
		dir = (struct ext2_dir_entry *)  ( (char *)dir + dir->rec_len );		
	}	
	return dir;
}

struct ext2_dir_entry * parse_root_directory(char * file_name)
{
	u32 start_sector = inode_contain(164,2);	//root dir has always inode 2
	int nr_sectors = 1;

    struct ext2_dir_entry *dir = malloc(nr_sectors*512);
    read_disc_sector(start_sector*2,nr_sectors,dir);

	
	return (parse_dir(dir,file_name));
}

char  * ext2_read_block(struct ext2_dir_entry *dir)
{
	 printk("dir->inode = %d\n\n", dir->inode);
     u32 what_block_group = find_inode(dir->inode);
    
	 u32 blocktoread = BGDT(what_block_group, dir->inode);	
	

	 u32 nr_sectors = file_size/512;
	 if (file_size % 512 != 0) { nr_sectors++; }
	  

	 printk("blocktoread %d\n",blocktoread);

	 char *buf =  malloc(1024);
	 return read_disc_sector(blocktoread*2,24,buf);	
}

struct ext2_dir_entry * ext2_read_dir_contents(char *dir_name, char * file_name)
{
	 struct ext2_dir_entry *dir = parse_root_directory(dir_name);
	  
 
	 struct ext2_dir_entry * new_buf = (struct ext2_dir_entry *)ext2_read_block(dir);	

	 return(parse_dir(new_buf, file_name));
}

void ext2_read_file_dir_contents(char * dir_name, char *file_name)
{	
	 struct ext2_dir_entry *dir =ext2_read_dir_contents(dir_name, file_name);
	
	 struct ext2_dir_entry * new_buf = (struct ext2_dir_entry *)ext2_read_block(dir);
	 
	 free(dir);

	 ext2_output_((unsigned char*)new_buf);	 
}


int ext2_open(int fd, struct file * file, char *file_path )
{
	printk("ext2_open");
	file = malloc_(sizeof(struct file));
	char *dirc, *basec, *bname, *dname; 
	dirc = strdup(file_path);
	basec = strdup(file_path);
	dname = dirname(dirc);
	bname = basename(basec);

	 struct ext2_dir_entry *dir =ext2_read_dir_contents(dname, bname);

	 file->f_inode = malloc(sizeof(struct inode));
	 file->f_op = malloc(sizeof(struct file_operations));
	 file->f_inode->i_ino = dir->inode;	
     file->f_inode->i_size = file_size;
     file->f_pos = 0;
     file->f_op->lseek = ext2_lseek;
	 file->f_op->read =  ext2_read;
	
	 file->f_op->close = sys_close;
	 current->files->fd[fd] = file;
	 
	 free(file);
	 free(file->f_inode);
	 free(file->f_op);
	 free(dirc);
	 free(basec);
	 return 0;
}

static void	read_indirect_blocks( u32 indir_block, u32 max_num, void *buf__) {
printk("indir_block %d\n", indir_block);
printk("max_num %d\n", max_num);


char *local_buf = malloc(1024);
read_disc_sector( indir_block*2, 1,local_buf);


u32 *blocks = (u32 *)local_buf;

int i;
for (i = 0; i < (int)max_num; i++) {

	read_disc_sector(*blocks*2, 2,(char*)buf__ + i * 1024);
		//printk("indirect block[%d]\n", *blocks++);
}

free(local_buf);
}

static u32 read_direct_blocks( u32 *blocklist, u32 num, void *buf) {

 printk("read_direct_blocks(num = %u)\n", num);

for (u32 i = 0; i < num; i++) {
if (blocklist[i] == 0) {
continue;
}

read_disc_sector(blocklist[i]*2, 2,(char *)buf + i * 1024);
}

return num;
}

static u32 read_singly_indirect_blocks( u32 indir_block, u32 max_num, void *buf) {
printk("indir_block[%d] \n",indir_block);

u32 *blocklist = malloc(1024);
read_disc_sector( indir_block*2,  1024 / 512,blocklist);

read_direct_blocks( blocklist, max_num, buf);

free(blocklist);

return max_num;
}


static u32 read_doubly_indirect_blocks( u32 doubly_block, u32 num, void *buf) {
printk("doubly_block[%d] \n",doubly_block);

u32 *singly_blocks = malloc(1024);
read_disc_sector(  doubly_block*2,  1024 / 512,singly_blocks);


printk("singly blocks for doubly block %u:\n", doubly_block);
for (u32 i = 0; i < 1024/4; i++) {
        printk("%u ", singly_blocks[i]);
        if (i % 10 == 0)
            printk("\n");
    }
printk("\n\n");


u32 read_data_blocks = 0;
for (u32 i = 0; num > read_data_blocks && i < 1024/4; i++) {
u32 singly = singly_blocks[i];


if (singly == 0)
continue;
u32 num_indir_blocks = min(num - read_data_blocks, 1024/4); 
read_data_blocks += read_singly_indirect_blocks( singly, num_indir_blocks, (char *)buf + read_data_blocks * 1024);
}
free(singly_blocks);

return read_data_blocks;
}

int ext2_read(struct inode * inode, struct file * file, int count, char *file_data)
{		
	struct ext2_dir_entry *dir= malloc(sizeof(struct ext2_dir_entry)*22);
	dir->inode = inode->i_ino;
	printk("inode nr %d\n", dir->inode);
	char * new_buf = ext2_read_block(dir);
	free(dir);
		printk("block_id_s : %d\n",block_id_s);
	free(new_buf);
	//memset(buf, 0, file_size);
	printk("file_size : %d\n",file_size);

		u32 num_blocks = file_size/(s_log_block_size);
		
		//char *file_data = malloc(num_blocks * 1024);
		//memset(file_data, 0, num_blocks * 1024);

		u32 read_blocks = 0;
		
		read_direct_blocks( &i_block[read_blocks], min(12, num_blocks), file_data);
		read_blocks += min(12, num_blocks);
		
		if (num_blocks > read_blocks) {
			u32 blocks_to_read = min(num_blocks - read_blocks, 1024/4);
			printk("\n\n%u blocks to read using the singly indirect block pointer\n", blocks_to_read);
			read_singly_indirect_blocks( block_id_s, blocks_to_read, file_data + read_blocks * 1024);
			read_blocks += blocks_to_read;
		}
		
			if (num_blocks > read_blocks) {

				u32 blocks_to_read = min(num_blocks - read_blocks, 1024/4 * 1024/4);
				// printk("\n\n%u blocks to read using the doubly indirect block pointer\n", blocks_to_read);
				read_doubly_indirect_blocks( block_id_d, blocks_to_read, file_data + read_blocks *1024);
				read_blocks += blocks_to_read;
			}
			
		return 0;
}


struct inode * ext2_stat(const char *file_path)
{
	char *dirc, *basec, *bname, *dname; 
	dirc = strdup(file_path);
	basec = strdup(file_path);
	dname = dirname(dirc);
	bname = basename(basec);

	printk("dirname=%s, basename=%s\n", dname, bname);
	
	 struct ext2_dir_entry *dir =ext2_read_dir_contents(dname, bname);
	 printk("inode number :%d\n", dir->inode);
	 
     u32 what_block_group = find_inode(dir->inode);
     
	 BGDT(what_block_group, dir->inode);
	 free(dir);	
	 printk("file size === %d\n\n", file_size);
	 struct inode *inode = malloc_(sizeof(struct inode));
	 inode->i_size = file_size;
	 free(inode);
	 return inode;
}

int ext2_lseek(struct inode *inode, struct file * file, int offset, int whence)
{
	printk("ext2_lseek called!");
	 long tmp = -1;
	switch (whence) {
                 case 0:
                         tmp = offset;
                         break;
                 case 1:
                         tmp = file->f_pos + offset;
                         break;
                 case 2:
                         if (!file->f_inode)
                                 return -EINVAL;
                         tmp = file->f_inode->i_size + offset;
                         break;
         }
				if (tmp < 0)
                 return -EINVAL;
         if (tmp != file->f_pos) {
                 file->f_pos = tmp;
                 file->f_reada = 0;
             //    file->f_version = ++event;
         }
         return file->f_pos;
}
