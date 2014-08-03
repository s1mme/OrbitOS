#include <types.h>
#include <heapmngr.h>
#include <system.h>

#include <mem.h>
void *_heapmngr_get_element(u32 i, table_t *table);
 heap_t * kheap;
table_t _heapmngr_table_initialize(u32 sz, void *addr)
{
	table_t table;
	table.element = addr;
	memset(table.element, 0 , sizeof(table_t));	
	table.maxsize = sz;
	return table;	
}

void _heapmngr_insert_element(void *element, table_t *table )
{
	u32 iterator = 0;
	while(iterator < table->size && (void **)element < table->element)
	iterator++;
	if(iterator == table->size)
	table->element[table->size++] = element;
	else
	{
		void *tmp = table->element[iterator];
		table->element[iterator] = element;
		while(iterator < table->size)
		{
			iterator++;
			void *tmp2 = table->element[iterator];
		table->element[iterator] = tmp;
		tmp = tmp2;
		}
		table->size++;
	}
}

s32 _heapmngr_find_element(u32 sz,u8 page_align, heap_t *heap)
{
	u32 i = 0;
	while(i < heap->table.size)
{
	desc_head *header = _heapmngr_get_element(i, &heap->table);
	if(page_align > 0)
	{
		
		u32 location = (u32)header;
		s32 offset = 0;
		if((location+(sizeof(desc_head) & 0xfffff000) != 0)) //make sure the adress is page aligned, important for ELF parsing!
		offset = 0x1000 - (location+sizeof(desc_head))%0x1000;
		s32 hole_size = (s32)header->size - offset;
		if(hole_size >= (s32)sz)
		break;
	}
	else if(header->size >= sz)
		break;
		i++;
	}
	if(i == heap->table.size)
		return -1;
	else
		return i;
}

void* _heapmngr_get_element(u32 i, table_t *table)
{
	return table->element[i];
}

void _heapmngr_delete_element(u32 i, table_t *table)
{
   while (i < table->size)
   {
       table->element[i] = table->element[i+1];
       i++;
   }
   table->size--;
} 


heap_t *_heapmngr_initialize(u32 heap_pool_start_pos, u32 heap_pool_end_pos, u32 sz)
{
	heap_t *heap  = (heap_t*)kvmalloc(sizeof(heap_t));
	memset(heap, 0, sizeof(heap_t));

	heap->table = _heapmngr_table_initialize(sz,(void*)heap_pool_start_pos);
	
	
	// Shift the start address forward to resemble where we can start putting data.
  // heap_pool_start_pos += sizeof(type_t)*HEAP_INDEX_SIZE;

   // Make sure the start address is page-aligned.
   if (heap_pool_start_pos & (0xFFFFF000 != 0))
   {
       heap_pool_start_pos &= 0xFFFFF000;
       heap_pool_start_pos += 0x1000;
   }

	heap_pool_start_pos += sizeof(type_t)*sz;

	desc_head *hole = (desc_head*)heap_pool_start_pos;
	hole->is_hole = 1;

	hole->size = heap_pool_end_pos - heap_pool_start_pos;
	hole->magic = HEAP_MAGIC;


	_heapmngr_insert_element((void*)hole, &heap->table);
	return heap;
}

void *request_kmalloc(u32 sz, u32 align, heap_t *heap)
{
	desc_head *old_element;
	u32 new_size = sz + sizeof(desc_head) + sizeof(desc_foot);
	s32 location = _heapmngr_find_element(new_size,1, heap);
	//if(location == -1)
	  //kprintf("We got to the end and didnt find anything!\n");
	old_element = _heapmngr_get_element(location, &heap->table);
	
#ifdef DEBUGGING
	//kprint("location : %d\n", location);
	//kprint("Magic : %x\n", old_element->magic);
#endif
	u32 element_backup = (u32)old_element;
	u32 element_backup_size = old_element->size;
	
		if (align && element_backup&0xFFFFF000)
   {
       u32 new_location   = element_backup + 0x1000 - (element_backup&0xFFF) - sizeof(desc_head);
       desc_head *hole_header = (desc_head *)element_backup;
       hole_header->size     = 0x1000 - (element_backup&0xFFF) - sizeof(desc_head);
       hole_header->magic    = HEAP_MAGIC;
       hole_header->is_hole  = 1;
       desc_foot *hole_footer = (desc_foot *) ( (u32)new_location - sizeof(desc_foot) );
       hole_footer->magic    = HEAP_MAGIC;
       hole_footer->header   = hole_header;
       element_backup         = new_location;
       element_backup_size        = element_backup_size - hole_header->size;
   }
   else
   {
     /*todo? */

   } 
	
	
	_heapmngr_delete_element(location, &heap->table); /*delete current element*/
	
	
	desc_head *block_header = (desc_head *)element_backup;
	block_header->is_hole = 0;
	block_header->size = new_size;
	
	desc_foot *block_footer = (desc_foot *) (element_backup + sizeof(desc_head) + sz);
	block_footer->header = block_header;
	
	//setup a new hole
    desc_head *hole_header = (desc_head *) (element_backup + sizeof(desc_head) + sz + sizeof(desc_foot));
     
    hole_header->size = element_backup_size - new_size;
	
	_heapmngr_insert_element((void*)hole_header,&heap->table); /*add a new element*/
	

	return (void *)((u32)block_header + sizeof(desc_head));
}

void release_kmalloc(void *ptr, heap_t *heap)
{
	if(ptr == 0)
	return;
	desc_head *element_old_head = (desc_head*) ( (u32)ptr - sizeof(desc_head) );

    desc_foot *element_old_foot = (desc_foot*) ( (u32)element_old_head + element_old_head->size - sizeof(desc_foot) );

 
    desc_head *test = (desc_head*) ( (u32)element_old_foot + sizeof(desc_foot) );

    element_old_head->size += test->size;     
    u32 i = 0;
    while ( (i < heap->table.size) &&
          (_heapmngr_get_element(i, &heap->table) != (void*)test) )
              i++;
              if (i < heap->table.size)
				_heapmngr_delete_element(i, &heap->table);
  
	_heapmngr_insert_element((void*)element_old_head, &heap->table);
}

void *malloc_(u32 sz)
{
	return request_kmalloc(sz, 0, kheap);
}

void *malloc(u32 sz)
{
	return malloc_(sz);
}


void *malloc_a(u32 sz)
{
	return request_kmalloc(sz, 1, kheap);
}

void free_(void *ptr)
{
	release_kmalloc(ptr, kheap);
}

void free(void *ptr)
{

	 free_(ptr);
}
