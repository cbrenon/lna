#ifndef _LNA_CORE_MEMORY_POOL_HPP_
#define _LNA_CORE_MEMORY_POOL_HPP_

#include <cstdint>

#define LNA_KILOBYTES(value)    ((value) * 1024LL)
#define LNA_MEGABYTES(value)    (LNA_KILOBYTES(value) * 1024LL)
#define LNA_GIGABYTES(value)    (LNA_MEGABYTES(value) * 1024LL)

namespace lna
{
    struct default_allocator;

    struct memory_pool
    {
        size_t              content_max_size;
        size_t              content_cur_size;
        void*               content;
    };

    struct memory_pool_config
    {
        size_t              size_in_bytes;
        default_allocator*  allocator_ptr;
    };

    void memory_pool_configure(
        memory_pool& mem_pool,
        memory_pool_config& config
        );
    
    void* memory_pool_alloc(
        memory_pool& mem_pool,
        size_t size
        );

    void memory_pool_empty(
        memory_pool& mem_pool
        );
}

#define LNA_ALLOC(mem_pool, type, element_count) (type*)lna::memory_pool_alloc(mem_pool, element_count * sizeof(type))

#endif // _LNA_CORE_MEMORY_POOL_HPP_
