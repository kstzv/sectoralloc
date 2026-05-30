#include <stdio.h>
#include <stdlib.h>

#include "alloc_memory_header.h"

int main(void)
{
	// Allocate 1 MB memory region for allocator initialization
    size_t size = 1024 * 1024; // 1 МБ

    void *mem = aligned_alloc(SIZE_BLOCK_PAGE, size);

    if(!mem)
    {
        printf("malloc error\n");
        return 1;
    }

	 // Initialize allocator metadata
    struct mem_alloc *ctx = init_mem(mem, size);

    if(!ctx)
    {
        printf("init_mem error\n");
        free(mem);
        return 1;
    }

	 // Print calculated allocator layout
    printf("pages: %u\n", ctx->number_pages);
    printf("sectors: %u\n", ctx->number_sectors);
    printf("last sector: %u\n", ctx->size_last_sector);

    printf("pages_base: %p\n", (void *)ctx->pages_base);

    printf("large_alloc: %p\n", (void *)ctx->curr_large_alloc);

    printf("small_alloc: %p\n", (void *)ctx->curr_small_alloc);

    printf("extra size: %u\n", ctx->extra_mem_size);

	// Release test memory
    free(mem);

    return 0;
}
