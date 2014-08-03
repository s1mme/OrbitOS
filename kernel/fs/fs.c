#include <task.h>
#include <fs.h>
#include <ext2.h>
#include <sched.h>
#include <errno.h>
#include <stat.h>
#include <elf.h>

 #define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
 struct file * first_file = NULL;
 int nr_files = 0;
 #define NR_FILE 64
 int max_files = NR_FILE;
 
 unsigned long ffz(unsigned long word)
 {
         __asm__("bsf %1,%0"
                 : "=r" (word)
                 : "r" (~word));
         return word;
 }
 #define BITS_PER_LONG 32
unsigned long find_first_zero_bit(const unsigned long *addr, unsigned long size)
 {
         const unsigned long *p = addr;
         unsigned long result = 0;
         unsigned long tmp;
 
         while (size & ~(BITS_PER_LONG-1)) {
                 if (~(tmp = *(p++)))
                         goto found;
                 result += BITS_PER_LONG;
                 size -= BITS_PER_LONG;
         }
         if (!size)
                 return result;
 
         tmp = (*p) | (~0UL << size);
         if (tmp == ~0UL)        /* Are any bits zero? */
                 return result + size;   /* Nope. */
 found:
         return result + ffz(tmp);
 }
  #define RLIMIT_NOFILE   7  
 int get_unused_fd(void)
 {
         int fd;
         struct files_struct * files = current->files;
 
         fd = find_first_zero_bit((const unsigned long *)&files->open_fds, NR_OPEN);
         printk("fd = %d\n", fd);
       //  if (fd < current->rlim[RLIMIT_NOFILE].rlim_cur) {
		
                 FD_SET(fd, &files->open_fds);
                 FD_CLR(fd, &files->close_on_exec);
                 return fd;
         //}
 }

    
struct file *ext2_;  

static int do_open( char * filename, int flags, int mode, int fd)
{
  //       struct inode * inode;
         
         struct file * f;
         ext2_->f_op->open = ext2_open;
         ext2_->f_op->read = ext2_read;
    //   f = get_empty_filp();
         f = ext2_;
		 printk("do_open");
         f->f_op->open(fd, f, filename);
         
         return 0;
}

int sys_open(char * filename, int flags, int mode)
 {

         int fd, error;
 
         fd = get_unused_fd();
         printk("fd = %d\n", fd);
         if (fd < 0)
                return fd;
         printk("fd_ = %d\n", fd);
         error = do_open(filename, flags, mode, fd);
         if (!error)
               return fd;

		 printk("fd__ = %d\n", fd);
         return error;
 }
 
int kopen( char * filename,int mode)
{
	return sys_open(filename, 0, mode);
}

 struct file * fget(unsigned long fd)
 {
         struct file * file = NULL;
         if (fd < NR_OPEN) {
		
                 file = current->files->fd[fd];

                 if (file)
                          file->f_count++;
          }
          return file;
  }
  /*
void fput(struct file *file, struct inode *inode)
  {
          int count = file->f_count-1;
          if (!count)
                  __fput(file, inode);
          file->f_count = count;
  } */
  
int sys_read(unsigned int fd,char * buf,int count)
 {
         int error;
         struct file * file ;
         struct inode * inode;
 
         error = -EBADF;
         file = fget(fd);
         printk("file %d\n", file);
         if (!file)
                 goto bad_file;
         inode = file->f_inode;	
      
         error = 0;
         if (count <= 0)
                 goto out;

        file->f_op->read(inode,file,count,buf);
    
		 return error;
 out:
     //    fput(file, inode);
 bad_file:
         return error;
}
int kread(unsigned int fd,char * buf,int count)
{
	return sys_read(fd,buf,count);
}
	

void register_filesystem(void)
{
	ext2_->f_op->open = ext2_open;
}

int sys_close( int fd) 
{
if(current->files->fd[fd] == NULL)
	return 1;
else
	current->files->fd[fd] = NULL;

return 0;
}


 int sys_stat(const char * filename, struct stat * statbuf)
  {

         struct inode *inode_; //= malloc_(sizeof(struct inode));
		 inode_ = ext2_stat(filename);
		 printk("filesize is %d\n", inode_->i_size);
        // tmp.st_dev = kdev_t_to_nr(inode->i_dev);
         statbuf->st_ino = inode_->i_ino;
         statbuf->st_mode = inode_->i_mode;
   //      tmp.st_nlink = inode->i_nlink;
   //      tmp.st_uid = inode->i_uid;
   //      tmp.st_gid = inode->i_gid;
   //      tmp.st_rdev = kdev_t_to_nr(inode->i_rdev);
         statbuf->st_size = inode_->i_size;
       return 0;
    //       if (inode->i_pipe)
    //             tmp.st_size = PIPE_SIZE(*inode);
    //      tmp.st_atime = inode->i_atime;
    //      tmp.st_mtime = inode->i_mtime;
    //      tmp.st_ctime = inode->i_ctime;
       
 }
 void stat(const char * filename, struct stat * statbuf)
 {
	 sys_stat(filename, statbuf);
 }

long sys_lseek(unsigned int fd, off_t offset, unsigned int origin)
 {
         struct file * file =  fget(fd);
        
 
         if (fd >= NR_OPEN || !(file) || !(file->f_inode))
                 return -EBADF;
         if (origin > 2)
                 return -EINVAL;
         if (file->f_op && file->f_op->lseek)
                 return file->f_op->lseek(file->f_inode,file,offset,origin);
   return 0;
 }
