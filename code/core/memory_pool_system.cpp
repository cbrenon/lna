#include <cstdint>
#include "core/memory_pool_system.hpp"

void lna::memory_pool_system_free_all(
    lna::memory_pool_system& pool_system
    )
{
    for (uint32_t i = 0; i < lna::memory_pool_system_id::COUNT; ++i)
    {
        lna::memory_pool_free(
            pool_system.pools[i]
            );
    }
}