#ifndef _LNA_CORE_MEMORY_POOL_SYSTEM_HPP_
#define _LNA_CORE_MEMORY_POOL_SYSTEM_HPP_

#include "core/memory_pool.hpp"

namespace lna
{
    enum memory_pool_system_id
    {
        FRAME_LIFETIME,
        PERSISTENT_LIFETIME,
        COUNT,
    };

    struct memory_pool_system
    {
        memory_pool pools[memory_pool_system_id::COUNT];
    };

    void memory_pool_system_free_all(
        memory_pool_system& pool_system
        );
}

#endif // _LNA_CORE_MEMORY_POOL_SYSTEM_HPP_
