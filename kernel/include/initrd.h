#ifndef INITRD_H
#define INITRD_H
#include <types.h>

#define PATH_SEPARATOR '/'
#define PATH_SEPARATOR_STRING "/"
#define PATH_UP  ".."
#define PATH_DOT "."

#define O_RDONLY     0x0000
#define O_WRONLY     0x0001
#define O_RDWR       0x0002
#define O_APPEND     0x0008
#define O_CREAT      0x0200
#define O_TRUNC      0x0400
#define O_EXCL       0x0800

#define FS_FILE        0x01
#define FS_DIRECTORY   0x02
#define FS_CHARDEVICE  0x04
#define FS_BLOCKDEVICE 0x08
#define FS_PIPE        0x10
#define FS_SYMLINK     0x20
#define FS_MOUNTPOINT  0x40

#define _IFMT       0170000 /* type of file */
#define     _IFDIR  0040000 /* directory */
#define     _IFCHR  0020000 /* character special */
#define     _IFBLK  0060000 /* block special */
#define     _IFREG  0100000 /* regular */
#define     _IFLNK  0120000 /* symbolic link */
#define     _IFSOCK 0140000 /* socket */
#define     _IFIFO  0010000 /* fifo */

struct fs_node;

typedef u32 (*read_type_t) (struct fs_node *, u32, u32, u8 *);
typedef u32 (*write_type_t) (struct fs_node *, u32, u32, u8 *);
typedef void (*open_type_t) (struct fs_node *, unsigned int flags);
typedef void (*close_type_t) (struct fs_node *);
typedef struct dirent *(*readdir_type_t) (struct fs_node *, u32);
typedef struct fs_node *(*finddir_type_t) (struct fs_node *, char *name);
typedef void (*create_type_t) (struct fs_node *, char *name, u16 permission);
typedef void (*unlink_type_t) (struct fs_node *, char *name);
typedef void (*mkdir_type_t) (struct fs_node *, char *name, u16 permission);
typedef int (*ioctl_type_t) (struct fs_node *, int request, void * argp);
typedef int (*get_size_type_t) (struct fs_node *);
typedef int (*chmod_type_t) (struct fs_node *, int mode);

typedef struct fs_node {
	char name[256];         /* The filename. */
	void * device;          /* Device object (optional) */
	u32 mask;          /* The permissions mask. */
	u32 uid;           /* The owning user. */
	u32 gid;           /* The owning group. */
	u32 flags;         /* Flags (node type, etc). */
	u32 inode;         /* Inode number. */
	u32 length;        /* Size of the file, in byte. */
	u32 impl;          /* Used to keep track which fs it belongs to. */
	u32 open_flags;    /* Flags passed to open (read/write/append, etc.) */

	/* times */
	u32 atime;         /* Accessed */
	u32 mtime;         /* Modified */
	u32 ctime;         /* Created  */

	/* File operations */
	read_type_t read;
	write_type_t write;
	open_type_t open;
	close_type_t close;
	readdir_type_t readdir;
	finddir_type_t finddir;
	create_type_t create;
	mkdir_type_t mkdir;
	ioctl_type_t ioctl;
	get_size_type_t get_size;
	chmod_type_t chmod;
	unlink_type_t unlink;

	struct fs_node *ptr;   /* Alias pointer, for symlinks. */
	u32 offset;       /* Offset for read operations XXX move this to new "file descriptor" entry */
	int shared_with;   /* File descriptor sharing XXX */
} fs_node_t;

struct dirent {
	u32 ino;           /* Inode number. */
	char name[256];         /* The filename. */
};



extern fs_node_t *fs_root;
extern fs_node_t * null_device_create(void);
extern fs_node_t * zero_device_create(void);
extern fs_node_t * serial_device_create(int device);
extern fs_node_t * procfs_create(void);
extern fs_node_t * tmpfs_create(void);
extern void serial_mount_devices(void);
extern int openpty(int * master, int * slave, char * name, void * _ign0, void * size);

extern fs_node_t * hello_device_create(void);
extern fs_node_t * random_device_create(void);

int read_fs(fs_node_t *node, int offset, int size,  char *buffer);
u32 write_fs(fs_node_t *node, u32 offset, u32 size, u8 *buffer);
void open_fs(fs_node_t *node, unsigned int flags);
void close_fs(fs_node_t *node);
extern struct dirent *readdir_fs(fs_node_t *node, int index);
fs_node_t *finddir_fs(fs_node_t *node, char *name);

typedef struct
{
    u32 nfiles; // The number of files in the ramdisk.
} initrd_header_t;

typedef struct
{
    u8 magic;     
    s8 name[64];  // Filename.
    u32 offset;   
    u32 length;   
} initrd_file_header_t;


fs_node_t *install_initrd(u32 location);
extern fs_node_t *fs_root_;
extern char * load_initrd_app(char *name, char **argv, char **env);
#endif
