#include <stdlib.h>
#include "core/memory_pool.hpp"
#include "core/assert.hpp"

#define LNA_KILOBYTES(value)    ((value) * 1024LL)
#define LNA_MEGABYTES(value)    (LNA_KILOBYTES(value) * 1024LL)
#define LNA_GIGABYTES(value)    (LNA_MEGABYTES(value) * 1024LL)

namespace
{
    void memory_pool_allocate(
        lna::memory_pool& pool,
        size_t size
        )
    {
        LNA_ASSERT(pool.content == nullptr);
        LNA_ASSERT(pool.content_cur_size == 0);
        LNA_ASSERT(pool.content_max_size == 0);
        LNA_ASSERT(size > 0);

        pool.content_max_size = size;
        //TODO: is it more accurate to use VirtualAlloc instead of malloc? doc=> https://docs.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc
        pool.content = (char*)malloc(size);
        LNA_ASSERT(pool.content);
    }
}

namespace lna
{
    void memory_pool_manager_init(
        memory_pool_manager& mem_pool_manager
        )
    {
        mem_pool_manager.pools          = nullptr;
        mem_pool_manager.cur_pool_count = 0;
        mem_pool_manager.max_pool_count = 0;
    }

    void memory_pool_manager_configure(
        memory_pool_manager& mem_pool_manager,
        uint32_t max_pool_count
        )
    {
        LNA_ASSERT(mem_pool_manager.pools == nullptr);
        LNA_ASSERT(mem_pool_manager.cur_pool_count == 0);
        LNA_ASSERT(mem_pool_manager.max_pool_count == 0);
        LNA_ASSERT(max_pool_count != 0);
        LNA_ASSERT(max_pool_count != (uint32_t)-1);

        mem_pool_manager.max_pool_count = max_pool_count;
        mem_pool_manager.pools          = (memory_pool*)malloc(max_pool_count * sizeof(memory_pool));
        LNA_ASSERT(mem_pool_manager.pools);

        for (uint32_t i = 0; i < mem_pool_manager.max_pool_count; ++i)
        {
            mem_pool_manager.pools[i].content           = nullptr;
            mem_pool_manager.pools[i].content_cur_size  = 0;
            mem_pool_manager.pools[i].content_max_size  = 0;
        }
    }

    memory_pool* memory_pool_manager_new_pool(
        memory_pool_manager& mem_pool_manager
        )
    {
        LNA_ASSERT(mem_pool_manager.pools);
        LNA_ASSERT(mem_pool_manager.max_pool_count > 0);
        LNA_ASSERT(mem_pool_manager.cur_pool_count < mem_pool_manager.max_pool_count);
        LNA_ASSERT(mem_pool_manager.pools[mem_pool_manager.cur_pool_count].content == nullptr);
        LNA_ASSERT(mem_pool_manager.pools[mem_pool_manager.cur_pool_count].content_cur_size == 0);
        LNA_ASSERT(mem_pool_manager.pools[mem_pool_manager.cur_pool_count].content_max_size == 0);

        return &mem_pool_manager.pools[mem_pool_manager.cur_pool_count++];
    }

    void memory_pool_allocate_megabytes(
        memory_pool& mem_pool,
        size_t size_in_megabytes
        )
    {
        LNA_ASSERT(mem_pool.content == nullptr);
        LNA_ASSERT(mem_pool.content_cur_size == 0);
        LNA_ASSERT(mem_pool.content_max_size == 0);

        memory_pool_allocate(
            mem_pool,
            LNA_MEGABYTES(size_in_megabytes)
            );
    }

    void memory_pool_allocate_gigabytes(
        memory_pool& mem_pool,
        size_t size_in_gigabytes
        )
    {
        LNA_ASSERT(mem_pool.content == nullptr);
        LNA_ASSERT(mem_pool.content_cur_size == 0);
        LNA_ASSERT(mem_pool.content_max_size == 0);

        memory_pool_allocate(
            mem_pool,
            LNA_GIGABYTES(size_in_gigabytes)
            );
    }

    void* memory_pool_reserve_memory(
        memory_pool& mem_pool,
        size_t size
        )
    {
        LNA_ASSERT(mem_pool.content);
        LNA_ASSERT((mem_pool.content_cur_size + size) < mem_pool.content_max_size);

        size_t offset = mem_pool.content_cur_size;
        mem_pool.content_cur_size += size;
        return (void*)(mem_pool.content + offset);
    }

    void memory_pool_empty(
        memory_pool& mem_pool
        )
    {
        mem_pool.content_cur_size = 0;
    }

    void memory_pool_manager_release(
        memory_pool_manager& mem_pool_manager
        )
    {
        if (mem_pool_manager.pools)
        {
            for (uint32_t i = 0; i < mem_pool_manager.cur_pool_count; ++i)
            {
                memory_pool& mem_pool = mem_pool_manager.pools[i];
                if (mem_pool.content)
                {
                    free(mem_pool.content);
                    mem_pool.content            = nullptr;
                    mem_pool.content_cur_size   = 0;
                    mem_pool.content_max_size   = 0;
                }
            }
            free(mem_pool_manager.pools);
            mem_pool_manager.pools          = nullptr;
            mem_pool_manager.cur_pool_count = 0;
            mem_pool_manager.max_pool_count = 0;
        }
    }
}
