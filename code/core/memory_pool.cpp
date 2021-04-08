#include <stdlib.h>
#include "core/memory_pool.hpp"
#include "core/assert.hpp"
#include "core/allocator.hpp"

namespace lna
{
    void memory_pool_configure(
        memory_pool& mem_pool,
        memory_pool_config& config
        )
    {
        LNA_ASSERT(mem_pool.content == nullptr);
        LNA_ASSERT(mem_pool.content_cur_size == 0);
        LNA_ASSERT(mem_pool.content_max_size == 0);
        LNA_ASSERT(config.allocator_ptr);
        LNA_ASSERT(config.size_in_bytes > 0);

        mem_pool.content = default_allocator_alloc(
            *config.allocator_ptr,
            config.size_in_bytes
            );
        mem_pool.content_max_size = config.size_in_bytes;
    }

    void* memory_pool_alloc(
        memory_pool& mem_pool,
        size_t size
        )
    {
        LNA_ASSERT(mem_pool.content);
        LNA_ASSERT(mem_pool.content_max_size > 0);
        LNA_ASSERT(mem_pool.content_cur_size + size <= mem_pool.content_max_size);

        size_t offset = mem_pool.content_cur_size;
        mem_pool.content_cur_size += size;
        return (void*)((char*)mem_pool.content + offset);
    }

    void memory_pool_empty(
        memory_pool& mem_pool
        )
    {
        mem_pool.content_cur_size = 0;
    }
}

