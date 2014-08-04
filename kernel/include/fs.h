#ifndef _FS_H
#define _FS_H
#include <stat.h>
#include <task.h>
struct inode {
         unsigned long   i_ino;
         int         i_mode;
         int 		 i_size;
         int 	     block_id_s;
         int 		 block_id_d;
         int		 block_id_t;
};
 struct file {
         unsigned short f_mode;
         long long  f_pos;
         unsigned short f_flags;
         unsigned short f_count;
         unsigned long f_reada, f_ramax, f_raend, f_ralen, f_rawin;
         struct file *f_next, *f_prev;
         struct inode * f_inode;
         struct file_operations * f_op;
         unsigned long f_version;
         void *private_data;     
 };


struct file_operations {
         int (*lseek) (struct inode *, struct file *, int, int);
         int (*read) (struct inode *, struct file *, int, char *);
         int (*write) (struct inode *, struct file *, const char *, int);
      // int (*readdir) (struct inode *, struct file *, void *, filldir_t);
      // int (*select) (struct inode *, struct file *, int, select_table *);
         int (*ioctl) (struct inode *, struct file *, unsigned int, unsigned long);
      // int (*mmap) (struct inode *, struct file *, struct vm_area_struct *);
         int (*open) (int fd, struct file *, char *);
         int (*close) (int fd);
         
};


  #define __NFDBITS       (8 * sizeof(unsigned long))
  #define __FD_SETSIZE    1024
  #define __FDSET_LONGS   (__FD_SETSIZE/__NFDBITS)
  
  #define FD_SET(fd,fdsetp) \
                  __asm__ __volatile__("btsl %1,%0": \
                          "=m" (*(__kernel_fd_set *) (fdsetp)):"r" ((int) (fd)))
  

  #define FD_CLR(fd,fdsetp) \
                  __asm__ __volatile__("btrl %1,%0": \
                          "=m" (*(__kernel_fd_set *) (fdsetp)):"r" ((int) (fd)))
           
#define SEEK_SET        0       /* seek relative to beginning of file */
#define SEEK_CUR        1       /* seek relative to current file position */
#define SEEK_END        2       /* seek relative to end of file */       
 
typedef long off_t;
typedef unsigned long uintptr_t;
long sys_lseek(unsigned int fd, off_t offset, unsigned int origin);
 int sys_stat( char * filename, struct stat * statbuf);
 int sys_close( int fd);
 int sys_read(unsigned int fd,char * buf,int count);
 int sys_open( char * filename, int flags, int mode);
extern u32 heap_actual_global;
extern u32 heap_global;
extern  void stat( char * filename, struct stat * statbuf);
extern void register_filesystem(void);
extern int kopen( char * filename,int mode);
extern int kread(unsigned int fd,char * buf,int count);
#endif
