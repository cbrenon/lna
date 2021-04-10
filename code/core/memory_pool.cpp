#include <stdlib.h>
#include "core/memory_pool.hpp"
#include "core/assert.hpp"
#include "core/allocator.hpp"

namespace lna
{
    void memory_pool::init(size_t max_size_in_bytes, based_allocator& allocator)
    {
        LNA_ASSERT(max_size_in_bytes > 0);
        LNA_ASSERT(_content == nullptr);
        LNA_ASSERT(_content_cur_size == 0);
        LNA_ASSERT(_content_max_size == 0);

        _content            = allocator.alloc(max_size_in_bytes);
        _content_max_size   = max_size_in_bytes;
    }
}

