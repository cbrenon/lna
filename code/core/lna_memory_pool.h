#ifndef LNA_CORE_LNA_MEMORY_POOL_H
#define LNA_CORE_LNA_MEMORY_POOL_H

#include <stddef.h>

typedef struct lna_memory_pool_s
{
    size_t  cur_content_size;
    size_t  max_content_size;
    char*   content;
} lna_memory_pool_t;

typedef struct lna_heap_allocator_s lna_heap_allocator_t;

extern void     lna_memory_pool_init_with_heap  (lna_memory_pool_t* memory_pool, lna_heap_allocator_t* allocator, size_t size_in_bytes);
extern void*    lna_memory_pool_reserve         (lna_memory_pool_t* memory_pool, size_t size_in_bytes);
extern void     lna_memory_pool_empty           (lna_memory_pool_t* memory_pool);

#endif
