#include "core/lna_memory_pool.h"
#include "core/lna_assert.h"
#include "core/lna_heap_allocator.h"

void lna_memory_pool_init_with_heap(lna_memory_pool_t* memory_pool, lna_heap_allocator_t* allocator, size_t size_in_bytes)
{
    lna_assert(memory_pool)
    lna_assert(memory_pool->content == 0)
    lna_assert(memory_pool->cur_content_size == 0)
    lna_assert(memory_pool->max_content_size == 0)

    lna_log_message(
        "reserve %d bytes in allocator %p for memory pool %p",
        size_in_bytes,
        allocator,
        memory_pool
        );

    memory_pool->content = lna_heap_allocator_alloc(
        allocator,
        size_in_bytes
        );
    memory_pool->max_content_size = size_in_bytes;
}

void* lna_memory_pool_reserve(lna_memory_pool_t* memory_pool, size_t size_in_bytes)
{
    lna_assert(memory_pool)
    lna_assert(memory_pool->content)
    lna_assert(memory_pool->cur_content_size + size_in_bytes <= memory_pool->max_content_size)
    lna_assert(size_in_bytes > 0)

    size_t offset = memory_pool->cur_content_size;
    memory_pool->cur_content_size += size_in_bytes;
    return (void*)(memory_pool->content + offset);
}

void lna_memory_pool_empty(lna_memory_pool_t* memory_pool)
{
    lna_assert(memory_pool)
    memory_pool->cur_content_size = 0;
}
