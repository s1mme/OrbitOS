#include <system.h>
#include <initrd.h>

#include <types.h>

fs_node_t * fs_root = NULL;


int read_fs(fs_node_t *node, int offset, int size,  char *buffer)
{
    if (node->read != 0)
        return node->read(node, offset, size, (u8*)buffer);
    else
        return 0;
}


struct dirent *readdir_fs(fs_node_t *node, int index)
{
    if ( (node->flags&0x7) == FS_DIRECTORY &&
         node->readdir != 0 )
        return node->readdir(node, index);
    else
        return 0;
}

fs_node_t *finddir_fs(fs_node_t *node, char *name)
{
    if ( (node->flags&0x7) == FS_DIRECTORY &&
         node->finddir != 0 )
        return node->finddir(node, name);
    else
        return 0;
}


u32 write_fs(fs_node_t *node, u32 offset, u32 size, u8 *buffer) {
	if (node->write) {
		u32 ret = node->write(node, offset, size, buffer);
		return ret;
	} else {
		return 0;
	}
}


void open_fs(fs_node_t *node, unsigned int flags) {
	if (node->open) {
		node->open(node, flags);
	}
}


void close_fs(fs_node_t *node) {

	if (node->close) {
		node->close(node);
	}
}


int chmod_fs(fs_node_t *node, int mode) {
	if (node->chmod) {
		return node->chmod(node, mode);
	}
	return 0;
}


int ioctl_fs(fs_node_t *node, int request, void * argp) {
	if (node->ioctl) {
		return node->ioctl(node, request, argp);
	} else {
		return -1; 
	}
}


struct vfs_entry {
	char * name;
	fs_node_t * file; 
};

void vfs_install(void) {

}

