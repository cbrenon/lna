#ifndef _LNA_CORE_MEMORY_POOL_HPP_
#define _LNA_CORE_MEMORY_POOL_HPP_

#include <cstddef>
#include <cstdint>
#include "core/assert.hpp"

#define LNA_KILOBYTES(value)    ((value) * 1024LL)
#define LNA_MEGABYTES(value)    (LNA_KILOBYTES(value) * 1024LL)
#define LNA_GIGABYTES(value)    (LNA_MEGABYTES(value) * 1024LL)

namespace lna
{
    class based_allocator;

    class memory_pool
    {
        public:

            memory_pool() = default;
            ~memory_pool() = default;

            void init(size_t max_size_in_bytes, based_allocator& allocator);

            template<typename T>
            inline T* alloc(size_t element_count)
            {
                LNA_ASSERT(_content);
                LNA_ASSERT(_content_max_size > 0);

                size_t size = sizeof(T) * element_count;
                LNA_ASSERT(_content_cur_size + size <= _content_max_size);

                size_t offset = _content_cur_size;
                _content_cur_size += size;
                return (T*)(_content + offset);
            }

            inline void empty() { _content_cur_size = 0; }

        private:

            size_t      _content_cur_size   { 0 };
            size_t      _content_max_size   { 0 };
            std::byte*  _content            { nullptr };
    };
}

#endif // _LNA_CORE_MEMORY_POOL_HPP_
