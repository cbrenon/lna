#include <stdlib.h>
#include "core/memory_pool.hpp"
#include "core/assert.hpp"
#include "core/log.hpp"

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
        //TODO: is it more accurate to use VirtualAlloc instead of malloc. doc=> https://docs.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc
        pool.content = (char*)malloc(size);
        LNA_ASSERT(pool.content);
    }
}

void lna::memory_pool_init(
    lna::memory_pool& pool
    )
{
    pool.content            = nullptr;
    pool.content_cur_size   = 0;
    pool.content_max_size   = 0;
}

void lna::memory_pool_allocate_megabytes(
    lna::memory_pool& pool,
    size_t size
    )
{
    memory_pool_allocate(
        pool,
        LNA_MEGABYTES(size)
        );
}

void lna::memory_pool_allocate_gigabytes(
    lna::memory_pool& pool,
    size_t size
    )
{
    memory_pool_allocate(
        pool,
        LNA_GIGABYTES(size)
        );
}

void* lna::memory_pool_reserve(
    lna::memory_pool& pool,
    size_t size
    )
{
    LNA_ASSERT(pool.content);
    LNA_ASSERT((pool.content_cur_size + size) < pool.content_max_size);

    size_t offset = pool.content_cur_size;
    pool.content_cur_size += size;
    return (void*)(pool.content + offset);
}

void lna::memory_pool_empty(
    lna::memory_pool& pool
    )
{
    pool.content_cur_size = 0;
}
    
void lna::memory_pool_free(
    lna::memory_pool& pool
    )
{
    if (pool.content)
    {
        free(pool.content);
    }
    lna::memory_pool_init(pool);
}
