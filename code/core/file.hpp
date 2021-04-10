#ifndef _LNA_CORE_FILE_HPP_
#define _LNA_CORE_FILE_HPP_

#include "core/heap_array.hpp"

namespace lna
{
    class file
    {
        public:

            file() = default;
            ~file() = default;

            //! used during development. Dot not use for retail (prefer future file system)
            void debug_load(const char* filename, bool binary, memory_pool& mem_pool);

            inline auto size() const    { return _content.size();   }
            inline auto content() const { return _content.ptr();    }
        
        private:

            heap_array<char>    _content {};
    };
}

#endif // _LNA_CORE_FILE_HPP_
