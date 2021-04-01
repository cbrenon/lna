#ifndef _LNA_CORE_MEMORY_POOL_HPP_
#define _LNA_CORE_MEMORY_POOL_HPP_

#include <cstdint>

namespace lna
{
    struct memory_pool
    {
        size_t  content_max_size;
        size_t  content_cur_size;
        char*   content;
    };

    struct memory_pool_manager
    {
        memory_pool*    pools;
        uint32_t        cur_pool_count;
        uint32_t        max_pool_count;
    };
    
    void memory_pool_manager_configure(
        memory_pool_manager& mem_pool_manager,
        uint32_t max_pool_count
        );
    
    memory_pool* memory_pool_manager_new_pool(
        memory_pool_manager& mem_pool_manager
        );

    void memory_pool_allocate_megabytes(
        memory_pool& mem_pool,
        size_t size_in_megabytes
        );

    void memory_pool_allocate_gigabytes(
        memory_pool& mem_pool,
        size_t size_in_gigabytes
        );
    
    void* memory_pool_reserve_memory(
        memory_pool& mem_pool,
        size_t size
        );

    void memory_pool_empty(
        memory_pool& mem_pool
        );

    void memory_pool_manager_release(
        memory_pool_manager& mem_pool_manager
        );
}

#endif // _LNA_CORE_MEMORY_POOL_HPP_
