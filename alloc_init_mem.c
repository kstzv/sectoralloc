#include "alloc_memory_header.h"

// Allocator memory initialization file
// Memory initialization function. Takes - a pointer to memory
// And the memory size in bytes. Returns - a pointer to a structure
// Memory management by an allocator of type struct mem_alloc
struct mem_alloc *init_mem(void *mem, size_t size);

// Local function - calculates the number of bytes needed for metadata and
// The memory itself takes the number of bytes of memory, returns - the number of pages
void calc_layout(size_t size_mem, u8 *number_sectors, u8 *size_last_sector, u16 *number_pages);


// The main initialization function is the allocator - it allocates memory between metadata and memory itself 
// The distribution scheme is as follows: [struct mem_alloc][metadata all for all sectors][metadata all for all pages][memory][tail( < 4096)]
struct mem_alloc *init_mem(void *mem, size_t size)
{
	// Validation of received data
	if(!mem) { return NULL; }
	else if(size > MAX_SIZE_MEM) { return NULL; }
	else if(size%SIZE_PAGE != 0) { return NULL; }
	else if(size < MIN_SIZE_MEM) { return NULL; }
	else if(((uintptr_t)mem % SIZE_BLOCK_PAGE) != 0) { return NULL; }
	
	
	
	// Declare variables to determine the number of sectors, pages
	// And the number of pages in the last sector
	// And call the appropriate byte counting function for this
	u8 number_sectors;
	u8 size_last_sector;
	u16 number_pages;
	calc_layout(size, &number_sectors, &size_last_sector, &number_pages);
	if(number_pages < 1) { return NULL; }
	
	
	
	// Declaration of memory pointer in bytes, main structure, byte counter
	u8 *mem_cursor;
	mem_cursor = (u8 *)mem;
	struct mem_alloc *ctx;
	size_t bytes_counter = 0;
	
	// Get and aligning metadata for the main structure
	ctx = (struct mem_alloc *)mem_cursor;
	bytes_counter = ALIGN_UP(sizeof(struct mem_alloc), SIZE_BLOCK_PAGE);
	
	// Filling part of the metadata with starting values
	ctx->number_pages = number_pages;
	ctx->size_last_sector = size_last_sector;
	ctx->large_pages_front = 0;
	ctx->small_pages_front = 0;
	
	// Filling in the fields for sectors
	ctx->number_sectors = number_sectors;
	ctx->bitmap_fragment_mem_sectors = 0;
	ctx->bitmap_tail_mem_sectors = 0;
	// Running a loop to fill an array of sectors and fields of all its elements
	ctx->first_sector = (struct sector *)(mem_cursor + bytes_counter);
	for(u8 i = 0; i < number_sectors; i++)
	{
		atomic_flag_clear(&ctx->first_sector[i].access);
		ctx->first_sector[i].bitmap_seek_fragment = 0;
		ctx->first_sector[i].bitmap_seek_tail = 0;
		ctx->first_sector[i].size_max_fragment = 0;
		ctx->first_sector[i].size_max_tail = 0;
	}
	// Pointer offset and alignment after sector metadata array
	bytes_counter += ALIGN_UP(sizeof(struct sector) * number_sectors, SIZE_BLOCK_PAGE);
	
	// Starting a metadata populating loop for all pages. The loop goes through
	// All sectors except the last one, due to the unknown number of pages on it
	u8 i = 0;
	while(i < number_sectors - 1)
	{
		ctx->first_sector[i].first_page = (struct page *)(mem_cursor + bytes_counter) + (DEFAULT_SIZE_SECTOR * i);
		for(u8 j = 0; j < DEFAULT_SIZE_SECTOR; j++)
		{
			ctx->first_sector[i].first_page[j].bitmap_seek_free_mem[0] = 0;
			ctx->first_sector[i].first_page[j].bitmap_seek_free_mem[1] = 0;
			ctx->first_sector[i].first_page[j].bitmap_seek_free_mem[2] = 0;
			ctx->first_sector[i].first_page[j].bitmap_seek_free_mem[3] = 0;
			ctx->first_sector[i].first_page[j].number_this_page = j;
			ctx->first_sector[i].first_page[j].size_max_fragment = 0;
			ctx->first_sector[i].first_page[j].size_tail = 0;
			ctx->first_sector[i].first_page[j].state_page = PAGE_FREE;
			ctx->first_sector[i].first_page[j].type_page = NOT_DEFINED;
		}
		i++;
	}
	
	// Filling all pages on the last sector and aligning metadata for pages at the end, after the loop
	ctx->first_sector[i].first_page = (struct page *)(mem_cursor + bytes_counter) + (DEFAULT_SIZE_SECTOR * i);
	for(u8 j = 0; j < size_last_sector; j++)
	{
		ctx->first_sector[i].first_page[j].bitmap_seek_free_mem[0] = 0;
		ctx->first_sector[i].first_page[j].bitmap_seek_free_mem[1] = 0;
		ctx->first_sector[i].first_page[j].bitmap_seek_free_mem[2] = 0;
		ctx->first_sector[i].first_page[j].bitmap_seek_free_mem[3] = 0;
		ctx->first_sector[i].first_page[j].number_this_page = j;
		ctx->first_sector[i].first_page[j].size_max_fragment = 0;
		ctx->first_sector[i].first_page[j].size_tail = 0;
		ctx->first_sector[i].first_page[j].state_page = PAGE_FREE;
		ctx->first_sector[i].first_page[j].type_page = NOT_DEFINED;
	}
	bytes_counter += ALIGN_UP(sizeof(struct page) * number_pages, SIZE_BLOCK_PAGE);
	
	// Assignment of pointers to the beginning of memory and current (starting)
	// Page pointers for large allocations and small allocations
	ctx->pages_base = mem_cursor + bytes_counter;
	ctx->curr_large_alloc = ctx->pages_base;
	ctx->curr_small_alloc = ctx->pages_base + SIZE_PAGE * (ctx->number_pages - 1);
	
	// Calculation, definition and designation of the remainder
	bytes_counter += (SIZE_PAGE * ctx->number_pages);
	if(bytes_counter > size)
	{
		return NULL;
	}else if(bytes_counter == size)
	{
		ctx->extra_mem = NULL;
		ctx->extra_mem_size = 0;
	}else
	{
		ctx->extra_mem = mem_cursor + bytes_counter;
		ctx->extra_mem_size = size - bytes_counter;
	}
	
	return ctx;
}
	



// Calculates how many pages, sectors there can be in the allocator, taking into account metadata,
// Also counts the number of pages in the last sector
void calc_layout(size_t size_mem, u8 *number_sectors, u8 *size_last_sector, u16 *number_pages)
{
	
	// General byte counter
	size_t general_conter = 0;
	
	// Deduction with metadata alignment for the main allocator structure
	general_conter = ALIGN_UP(sizeof(struct mem_alloc), SIZE_BLOCK_PAGE);
	
	// Outer loop counting bytes for sectors
	u16 pages_counter = 0;
	u8 sector_counter = 0;
	while(general_conter < size_mem)
	{
		// Checking for sufficient memory
		if(general_conter + sizeof(struct sector) < size_mem) 
		{ 
			general_conter += sizeof(struct sector);
			sector_counter++; 
		}
		else { break; }
		
		// Inner loop counting pages for one sector
		int i = 0;
		while(i < DEFAULT_SIZE_SECTOR)
		{
		    if(general_conter + SIZE_PAGE + sizeof(struct page) < size_mem) 
		    { 
				general_conter += sizeof(struct page);
				general_conter += SIZE_PAGE;
				pages_counter++; 
				i++; 
			}  else  { break; }
		}
		
		// Check if there is enough memory for one page and one sector
		if(general_conter + SIZE_PAGE + sizeof(struct page) >= size_mem && i == 0)
		{
			sector_counter--;
			general_conter -= sizeof(struct sector);
			break;
		} 
	}
	
	// Metadata alignment for sectors and pages
	size_t temp = 0;
	temp = ALIGN_UP(sector_counter * sizeof(struct sector), SIZE_BLOCK_PAGE) - (sector_counter * sizeof(struct sector));
	general_conter += temp;
	temp = 0;
	temp = ALIGN_UP(pages_counter * sizeof(struct page), SIZE_BLOCK_PAGE) - (pages_counter * sizeof(struct page));
	general_conter += temp;
	
	// Checking whether there was enough clean memory at all, separating metadata, at least by one page
	if(general_conter > size_mem)
	{
		if(pages_counter == 1)
		{
			*number_sectors = 0;
			*number_pages = 0;
			*size_last_sector = 0;
			return;
		}

		pages_counter--;
		if(pages_counter % DEFAULT_SIZE_SECTOR == 0) { sector_counter--; }
	}
	 
	 // Assigning the received values ​​to the pointers about the number of sectors and pages
	 *number_sectors = sector_counter;
	 *number_pages = pages_counter;
	 
	 // Determining the number of pages in the last sector
	 temp = pages_counter % DEFAULT_SIZE_SECTOR;

	if(temp == 0)
	{
		temp = DEFAULT_SIZE_SECTOR;
	}
	*size_last_sector = temp; 
}
		 
			
		    
		














