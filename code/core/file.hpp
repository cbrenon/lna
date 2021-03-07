#ifndef _LNA_CORE_FILE_HPP_
#define _LNA_CORE_FILE_HPP_

#include <vector>

namespace lna
{
    //! used during development. Dot not use for retail (prefer future file system)
    std::vector<char> file_debug_load(
        const char* filename,
        bool binary
        );
}

#endif // _LNA_CORE_FILE_HPP_
