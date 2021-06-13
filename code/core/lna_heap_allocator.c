#include "core/lna_heap_allocator.h"
#include "core/lna_assert.h"
#include "core/lna_log.h"

char* lna_heap_allocator_alloc(lna_heap_allocator_t* allocator, size_t size_in_bytes)
{
    lna_assert(allocator)
    lna_assert(allocator->content)
    lna_assert(allocator->cur_content_offset + size_in_bytes <= allocator->max_content_size)

    size_t offset = allocator->cur_content_offset;
    allocator->cur_content_offset += size_in_bytes;
    return allocator->content + offset;
}
