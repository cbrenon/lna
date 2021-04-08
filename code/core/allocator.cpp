#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "core/allocator.hpp"
#include "core/assert.hpp"

namespace lna
{
    void default_allocator_init(
        default_allocator&  allocator,
        size_t              max_size_in_bytes
        )
    {
        LNA_ASSERT(allocator.content == nullptr);
        LNA_ASSERT(allocator.content_max_size_in_bytes == 0);
        LNA_ASSERT(allocator.current_content_offset == 0);

        allocator.content_max_size_in_bytes = max_size_in_bytes;
        allocator.content                   = VirtualAlloc(
            0,
            max_size_in_bytes,
            MEM_RESERVE | MEM_COMMIT,
            PAGE_EXECUTE_READWRITE
            );
        LNA_ASSERT(allocator.content);
    }

    void* default_allocator_alloc(
        default_allocator&  allocator,
        size_t              size_in_bytes
        )
    {
        LNA_ASSERT(allocator.content);
        LNA_ASSERT(size_in_bytes > 0);
        LNA_ASSERT(allocator.current_content_offset + size_in_bytes <= allocator.content_max_size_in_bytes);

        size_t offset = allocator.current_content_offset;
        allocator.current_content_offset += size_in_bytes;
        return (void*)((char*)allocator.content + offset);
    }

    void default_allocator_release(
        default_allocator&  allocator
        )
    {
        if (allocator.content)
        {
            VirtualFree(
                allocator.content,
                0,
                MEM_RELEASE
                );
            allocator.content                   = nullptr;
            allocator.content_max_size_in_bytes = 0;
            allocator.current_content_offset    = 0;
        }
    }
}