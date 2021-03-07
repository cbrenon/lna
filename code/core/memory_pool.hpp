#ifndef _LNA_CORE_MEMORY_POOL_HPP_
#define _LNA_CORE_MEMORY_POOL_HPP_

namespace lna
{
    struct memory_pool
    {
        size_t  _content_max_size   { 0 };
        size_t  _content_cur_size   { 0 };
        char*   _content            { nullptr };
    };

    void memory_pool_allocate_megabytes(
        memory_pool& pool,
        size_t size
        );

    void memory_pool_allocate_gigabytes(
        memory_pool& pool,
        size_t size
        );

    void* memory_pool_reserve(
        memory_pool& pool,
        size_t size
        );

    void memory_pool_empty(
        memory_pool& pool
        );
    
    void memory_pool_free(
        memory_pool& pool
        );
}

#endif // _LNA_CORE_MEMORY_POOL_HPP_
