#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "core/lna_allocator.h"
#include "core/lna_assert.h"
#include "core/lna_log.h"

void lna_allocator_init(lna_allocator_t* allocator, size_t max_size_in_bytes)
{
    lna_assert(allocator)
    lna_assert(allocator->content == 0)
    lna_assert(allocator->cur_content_offset == 0)
    lna_assert(allocator->max_content_size == 0)
    lna_assert(max_size_in_bytes > 0)

    lna_log_message(
        "allocate %d bytes in heap memory for allocator %p",
        max_size_in_bytes,
        allocator
        );

    allocator->max_content_size = max_size_in_bytes;
    allocator->content = VirtualAlloc(
        0,
        max_size_in_bytes,
        MEM_RESERVE | MEM_COMMIT,
        PAGE_EXECUTE_READWRITE
        );
    
    lna_assert(allocator->content)
}

char* lna_allocator_alloc(lna_allocator_t* allocator, size_t size_in_bytes)
{
    lna_assert(allocator)
    lna_assert(allocator->content)
    lna_assert(allocator->cur_content_offset + size_in_bytes <= allocator->max_content_size)

    size_t offset = allocator->cur_content_offset;
    allocator->cur_content_offset += size_in_bytes;
    return allocator->content + offset;
}

void lna_allocator_release(lna_allocator_t* allocator)
{
    lna_assert(allocator)
    lna_assert(allocator->content)

    lna_log_message(
        "free all %d allocated heap memory bytes for allocator %p",
        allocator->max_content_size,
        allocator
        );

    VirtualFree(
        allocator->content,
        0,
        MEM_RELEASE
        );

    allocator->content              = NULL;
    allocator->cur_content_offset   = 0;
    allocator->max_content_size     = 0;
}
