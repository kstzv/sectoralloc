#ifndef LIB_H
#define LIB_H

// Header of the anasin allocator
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

#define SIZE_PAGE          4096                   // Size page
#define SIZE_BLOCK_PAGE    16                     // Size of one page block (in bytes)
#define NUMBER_BLOCKS_PAGE 256                    // Number of blocks per page
#define LIMIT_SIZE_BYTES   256                    // The limit for the division of small fragments and large ones
#define MAX_SIZE_MEM       4096 * 2048            // Maximum number of bytes
#define MAX_PAGES          MAX_SIZE_MEM/SIZE_PAGE // Maximum number of pages

#define BITMAP_BLOCK_IN_PAGE   4  // Bitmap for finding free blocks in the page

// For alignment
#define ALIGN_UP(x, align) (((x) + (align) - 1) & ~((align) - 1))

// Code of errors
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

// Page status - busy, not busy, or partially busy
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
    GET_FROM_TAILS, // First get memory to search in the tails of pages
    DEFAULT         // By default, the allocator will search in fragments first (you can also enter NULL)
};
	

// Structure for describing one page
struct page {
	u64 bitmap_seek_free_mem[BITMAP_BLOCK_IN_PAGE]; // Bitmap for finding free page blocks
	u16 number_this_page;                  			// Current page number
	enum page_type type_page;    					// Page type - for large or small allocations
	enum page_state state_page;  					// Busy or not
	u8 size_max_fragment;                           // Size of the largest fragment (in blocks)
	u8 size_tail;                                   // Tail size (in blocks)
};

// Structure for describing the sector
struct sector {
	atomic_flag access;                     		  // Lock for atomic sector capture
	struct page *first_page;                          // Pointer to the first page of the current sector
	u32 bitmap_seek_fragment;                         // Bitmap to search for pages where there are fragments (u32 - 4 bytes == 32 bits)
	u32 bitmap_seek_tail;                             // Bitmap for finding pages where there is free memory on the tail (u32 - 4 bytes == 32 bits)
	u8 size_max_fragment;                             // Size of the largest fragment of one of the pages on this sector (in blocks)
	u8 size_max_tail;                                 // The size of the largest tail of one of the pages in this sector (in blocks)
};
	
	
// Structure for describing all memory
struct mem_alloc {
	// Fields for browsing and selecting directly from pages
	u16 number_pages;         // Number of pages
	u8 *pages_base;           // Pointer to the beginning of memory to use
	u8 *curr_large_alloc;     // Pointer to current free memory for large allocations
	u8 *curr_small_alloc;     // Pointer to current free memory for small allocations
	u16 large_pages_front;    // Counter of the number of consecutive pages involved for large allocations
	u16 small_pages_front;    // Counter of the number of consecutively allocated pages for small allocations
	
	// For searching and working with sectors
	u8 number_sectors;                // Number of sectores
	u64 bitmap_fragment_mem_sectors;  // Bitmap for finding sectors with fragmented memory
	u64 bitmap_tail_mem_sectors;      // Bitmap for finding sectors with tail memory
	struct sector *first_sector;      // Pointer to the first sector
	
	u8 *extra_mem;         // Pointer to reserved balance
	u16 extra_mem_size;    // Size of reserve
};

// Initialization function
extern struct mem_alloc *init_mem(void *mem, size_t size);

#endif /* LIB_H */
