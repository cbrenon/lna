#ifndef _LNA_CORE_FIXED_SIZE_VECTOR_HPP_
#define _LNA_CORE_FIXED_SIZE_VECTOR_HPP_

#include <cstdint>
#include "core/memory_pool.hpp"

namespace lna
{
    template<typename T>
    class fixed_size_vector
    {
        public:

            fixed_size_vector() = default;
            ~fixed_size_vector() = default;

            void init(uint32_t max_element_count, memory_pool& mem_pool)
            {
                LNA_ASSERT(max_element_count > 0);
                LNA_ASSERT(_max_element_count == 0);
                LNA_ASSERT(_cur_element_count == 0);
                LNA_ASSERT(_elements == nullptr);

                _elements           = mem_pool.alloc<T>(max_element_count);
                _max_element_count  = max_element_count;
            }

            T& new_element()
            {
                LNA_ASSERT(_cur_element_count < _max_element_count);
                LNA_ASSERT(_elements);
                return _elements[_cur_element_count++];
            }

            void release()
            {
                _elements           = nullptr;
                _cur_element_count  = 0;
                _max_element_count  = 0;
            }

            inline T& operator[](size_t index)
            {
                LNA_ASSERT(index < _cur_element_count);
                LNA_ASSERT(_elements);
                return _elements[index];
            }

            inline const T& operator[](size_t index) const
            {
                LNA_ASSERT(index < _cur_element_count);
                LNA_ASSERT(_elements);
                return _elements[index];
            }

            inline auto cur_size()  const   { return _cur_element_count;        }
            inline auto max_size()  const   { return _max_element_count;        }
            inline auto ptr()               { return _elements;                 }
            inline auto ptr()       const   { return _elements;                 }
            inline auto begin()             { return &_elements[0];             }
            inline auto begin()     const   { return &_elements[0];             }
            inline auto end()               { return &_elements[cur_size()];    }
            inline auto end()       const   { return &_elements[cur_size()];    }

        private:

            uint32_t    _cur_element_count  { 0 };
            uint32_t    _max_element_count  { 0 };
            T*          _elements           { nullptr };
    };
}

#endif // _LNA_CORE_FIXED_SIZE_VECTOR_HPP_
