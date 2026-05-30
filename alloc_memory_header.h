#ifndef LIB_H
#define LIB_H

// Allocator header
// Implemented basic constants, alignment formula, error codes,
// Page types and states, structure for managing a single page,
// Structure for managing a sector,
// Structure for managing all memory,
// Maximum number of sectors - 64,
// Number of pages in a sector - 32 (the last sector may be an exception)

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <stdatomic.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

// Determining the size correspondence of data types on the architecture
_Static_assert(sizeof(u8)  == 1, "u8 error");
_Static_assert(sizeof(u16) == 2, "u16 error");
_Static_assert(sizeof(u32) == 4, "u32 error");
_Static_assert(sizeof(u64) == 8, "u64 error");

#define SIZE_PAGE           4096                   // Size page
#define SIZE_BLOCK_PAGE     16                     // Size of one page block (in bytes)
#define NUMBER_BLOCKS_PAGE  256                    // Number of blocks per page
#define LIMIT_SIZE_BYTES    256                    // The boundary for the division of small fragments and large ones
#define MAX_SIZE_MEM        4096 * 2048            // Maximum number of bytes
#define MIN_SIZE_MEM        4096 * 64              // Minimum number of bytes - two sectors
#define MAX_PAGES           MAX_SIZE_MEM/SIZE_PAGE // Maximum number of pages
#define DEFAULT_SIZE_SECTOR 32                     // Default sector size
#define MAX_SECTORS         64                     // Maximum number of sectors

#define BITMAP_BLOCK_IN_PAGE   4  // Bitmap for finding free blocks in a page

// Alignment according to the formula
#define ALIGN_UP(x, align) (((x) + (align) - 1) & ~((align) - 1))

// Errors codes
#define ZD_ENOMEM  (-12)
#define ZD_EINVAL  (-22)
#define ZD_EBUSY   (-16)
#define ZD_EAGAIN  (-11)

// Page type - for large or small allocations
enum page_type {
	PAGE_SMALL,
    PAGE_LARGE,
    NOT_DEFINED
};

// Page status - busy, free, or partial
enum page_state {
	PAGE_FREE,
    PAGE_PARTIAL,
    PAGE_FULL
};

// Special parameters for the allocation function that
// Specify where to look for memory first
// In fragments, tails, or from the current pointer
enum param_seek {
	GET_QUICKLY,    // First get memory from the linear current pointer
    GET_FROM_TAILS, // First get the memory to search in the tails of the pages
    DEFAULT         // By default, the allocator will search in fragments first (you can also enter NULL)
};
	

// Struct for describe page
struct page {
	u64 bitmap_seek_free_mem[BITMAP_BLOCK_IN_PAGE]; // Bitmap of searching for free page blocks, if 1 - busy, and 0 - free
	u8 number_this_page;                  			// Number curr page
	enum page_type type_page;    					// Type page - for small or large allocatons
	enum page_state state_page;  					// Busy, free or partial
	u8 size_max_fragment;                           // Size largest fragment(in blocks)
	u8 size_tail;                                   // Size tail(in blocks)
};

// Struct for describe setcor
struct sector {
	atomic_flag access;                     		  // Lock for atomic sector capture
	struct page *first_page;                          // Pointer to the first page of the current sector
	u32 bitmap_seek_fragment;                         // Bitmap for searching pages where there are fragments (u32 - 4 bytes == 32 bits)
	u32 bitmap_seek_tail;                             // Bitmap for finding pages where there is free memory on the tail (u32 - 4 bytes == 32 bits)
	u8 size_max_fragment;                             // The size of the largest fragment of one of the pages on this sector (in blocks)
	u8 size_max_tail;                                 // The size of the largest tail of one of the pages in this sector (in blocks)
};
	
	
// Struct for describe base logic of allocator
struct mem_alloc {
	u16 number_pages;                        		// Number pages
	u8 size_last_sector;                            // Number pages in last sector
	u16 large_pages_front;                          // Counter of the number of consecutive pages involved for large allocations
	u16 small_pages_front;                          // Counter of the number of consecutive pages involved for small allocations
	
	// For seek and work in sectors
	u8 number_sectors;                // Number sectors
	u64 bitmap_fragment_mem_sectors;  // Bitmap for finding sectors with fragmented memory
	u64 bitmap_tail_mem_sectors;      // Bitmap for finding sectors with tail memory
	struct sector *first_sector;      // Pointer to the first sector
	
	u8 *pages_base;                   // Pointer to the beginn memory for allocations
	u8 *curr_large_alloc;             // Curr ptr to the free memory for large allocations
	u8 *curr_small_alloc;             // Curr ptr to the free memory for small allocations
	
	// Variables indicating remaining memory, it is ignored for now, however, it may be revealed in future versions
	u8 *extra_mem;         // Ptr to the remainder
	u16 extra_mem_size;    // Size of remainder
};

// Initialization function
extern struct mem_alloc *init_mem(void *mem, size_t size);

#endif /* LIB_H */
