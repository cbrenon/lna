#ifndef _LNA_CORE_ALLOCATOR_HPP_
#define _LNA_CORE_ALLOCATOR_HPP_

#include <cstddef>
#include "core/assert.hpp"

namespace lna
{
    class based_allocator
    {
        public:

            based_allocator() = default;
            ~based_allocator();

            void        init(size_t max_size_in_bytes);
            std::byte*  alloc(size_t size_in_bytes);

        private:

            size_t      _cur_content_offset         { 0 };
            size_t      _content_max_size_in_bytes  { 0 };
            std::byte*  _content                    { nullptr };
    };
}

#endif // _LNA_CORE_ALLOCATOR_HPP_
