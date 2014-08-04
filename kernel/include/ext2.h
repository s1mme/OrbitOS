#ifndef _orb_EXT2_FS_H
#define _orb_EXT2_FS_H
#include <types.h>
#include <fs.h>
struct ext2_super_block {
        u32   s_inodes_count;         /* Inodes count */
        u32   s_blocks_count;         /* Blocks count */
        u32   s_r_blocks_count;       /* Reserved blocks count */
        u32   s_free_blocks_count;    /* Free blocks count */
        u32   s_free_inodes_count;    /* Free inodes count */
        u32   s_first_data_block;     /* First Data Block */
        u32   s_log_block_size;       /* Block size */
        signed int   s_log_frag_size;        /* Fragment size */
        u32   s_blocks_per_group;     /* # Blocks per group */
        u32   s_frags_per_group;      /* # Fragments per group */
        u32   s_inodes_per_group;     /* # Inodes per group */
        u32   s_mtime;                /* Mount time */
        u32   s_wtime;                /* Write time */
        u16   s_mnt_count;            /* Mount count */
        signed short   s_max_mnt_count;        /* Maximal mount count */
        u16   s_magic;                /* Magic signature */
        u16   s_state;                /* File system state */
        u16   s_errors;               /* Behaviour when detecting errors */
        u16   s_minor_rev_level;      /* minor revision level */
        u32   s_lastcheck;            /* time of last check */
        u32   s_checkinterval;        /* max. time between checks */
        u32   s_creator_os;           /* OS */
        u32   s_rev_level;            /* Revision level */
        u16   s_def_resuid;           /* Default uid for reserved blocks */
        u16   s_def_resgid;           /* Default gid for reserved blocks */
        
        
         u32   s_first_ino;            /* First non-reserved inode */
         u16   s_inode_size;           /* size of inode structure */
         u16   s_block_group_nr;       /* block group # of this superblock */
         u32   s_feature_compat;       /* compatible feature set */
         u32   s_feature_incompat;     /* incompatible feature set */
         u32   s_feature_ro_compat;    /* readonly-compatible feature set */
         u32   s_reserved[230];        /* Padding to the end of the block */
};

 struct ext2_group_desc
{
         u32   bg_block_bitmap;                /* Blocks bitmap block */
         u32   bg_inode_bitmap;                /* Inodes bitmap block */
         u32   bg_inode_table;         /* Inodes table block */
         u16   bg_free_blocks_count;   /* Free blocks count */
         u16   bg_free_inodes_count;   /* Free inodes count */
         u16   bg_used_dirs_count;     /* Directories count */
         u16   bg_pad;
         u32   bg_reserved[3];
};

struct ext2_inode {
         u16   i_mode;         /* File mode */
         u16   i_uid;          /* Owner Uid */
         u32   i_size;         /* Size in bytes */
         u32   i_atime;        /* Access time */
         u32   i_ctime;        /* Creation time */
         u32   i_mtime;        /* Modification time */
         u32   i_dtime;        /* Deletion Time */
         u16   i_gid;          /* Group Id */
         u16   i_links_count;  /* Links count */
         u32   i_blocks;       /* Blocks count */
         u32   i_flags;        /* File flags */
         u32   i_reserved1;
         u32   i_block[22];/* Pointers to blocks */
};

 /*
  * Structure of a directory entry
  */
 #define EXT2_NAME_LEN 255
 
 struct ext2_dir_entry {
         u32   inode;                  /* Inode number */
         u16   rec_len;                /* Directory entry length */
         char    name_len;               /* Name length */
         char   type;
         char    name[EXT2_NAME_LEN];    /* File name */
 };

extern int ext2_read(struct inode * inode, struct file * file, int count,char *buf);
extern int ext2_open(int fd, struct file * file, char *file_name );
extern struct inode * ext2_stat( char *file_path);
extern struct ext2_super_block * ext2_read_superblock(void);
extern u32 file_size;
#endif
