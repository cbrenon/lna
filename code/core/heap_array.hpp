#ifndef _LNA_CORE_HEAP_ARRAY_HPP_
#define _LNA_CORE_HEAP_ARRAY_HPP_

#include <cstdint>
#include "core/memory_pool.hpp"

namespace lna
{
    template<typename T>
    class heap_array
    {
        public:

            heap_array() = default;
            ~heap_array() = default;

            void init(uint32_t element_count, memory_pool& mem_pool)
            {
                LNA_ASSERT(element_count > 0);
                LNA_ASSERT(_element_count == 0);
                LNA_ASSERT(_elements == nullptr);

                _elements       = mem_pool.alloc<T>(element_count);
                _element_count  = element_count;
            }

            inline T& operator[](size_t index)
            {
                LNA_ASSERT(index < _element_count);
                LNA_ASSERT(_elements);
                return _elements[index];
            }

            inline const T& operator[](size_t index) const
            {
                LNA_ASSERT(index < _element_count);
                LNA_ASSERT(_elements);
                return _elements[index];
            }

            inline auto size()  const   { return _element_count;        }
            inline auto ptr()           { return _elements;             }
            inline auto ptr()  const    { return _elements;             }
            inline auto begin()         { return &_elements[0];         }
            inline auto begin() const   { return &_elements[0];         }
            inline auto end()           { return &_elements[size()];    }
            inline auto end()   const   { return &_elements[size()];    }

        private:

            uint32_t    _element_count  { 0 };
            T*          _elements       { nullptr };
    };
}

#endif // _LNA_CORE_CONTAINER_HPP_
