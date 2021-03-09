#ifndef _LNA_CORE_FILE_HPP_
#define _LNA_CORE_FILE_HPP_

#include "core/container.hpp"

namespace lna
{
    //! used during development. Dot not use for retail (prefer future file system)
    heap_array<char> file_debug_load(
        const char* filename,
        bool binary,
        memory_pool& pool
        );
}

#endif // _LNA_CORE_FILE_HPP_
