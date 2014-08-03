#ifndef mem_H
#define mem_H

#include <types.h>


typedef struct page {
	u32 present:1;
	u32 rw:1;
	u32 user:1;
	u32 accessed:1;
	u32 dirty:1;
	u32 unused:7;
	u32 frame:20;
} __attribute__((packed)) page_t;

typedef struct page_table {
	page_t *pages[1024]; //ändra på denna (*)  vid ny kompilering av task.c
} page_table_t;

typedef struct page_directory {
	page_table_t *tables[1024];	/* 1024 pointers to page tables... */
	u32 physical_tables[1024];	/* Physical addresses of the tables */
	u32 physical_address;	/* The physical address of physical_tables */

	int ref_count;
} page_directory_t;

extern page_directory_t *kernel_directory;
extern page_directory_t *current_directory;

extern u32 placement_pointer;
void
vmmngr_initialize(u32 memsize);
extern u32
kvmalloc(
		size_t size
		);
extern page_directory_t *clone_directory(page_directory_t *src);
extern void switch_page_directory(page_directory_t * dir);
extern void
_vmmngr_alloc_frame(
		page_t *page,
		int is_kernel,
		int is_writeable
		);
		
extern page_t *
_vmm_get_page_addr(
		u32 address,
		int make,
		page_directory_t * dir
		);
#endif
