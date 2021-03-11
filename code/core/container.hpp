#ifndef _LNA_CORE_CONTAINER_HPP_
#define _LNA_CORE_CONTAINER_HPP_

#include <cstdint>
#include "core/memory_pool.hpp"
#include "core/assert.hpp"

namespace lna
{
    template<typename T>
    struct heap_array
    {
        uint32_t    _element_count  { 0 };
        T*          _elements       { nullptr };
    }; 

    template<typename T>
    void heap_array_set_max_element_count(
        heap_array<T>& array,
        memory_pool& pool,
        uint32_t element_count
        )
    {
        LNA_ASSERT(array._element_count == 0);
        LNA_ASSERT(array._elements == nullptr);

        array._elements = (T*)memory_pool_reserve(
            pool,
            element_count * sizeof(T)
            );
        array._element_count = element_count;
    }

    template<typename T>
    void heap_array_fill_with_unique_value(
        heap_array<T>& array,
        T& value
        )
    {
        LNA_ASSERT(array._elements);

        // TODO: we will see later for optimization
        for (uint32_t i = 0; i < array._element_count; ++i)
        {
            array._elements[i] = value;
        }
    }

    template<typename T>
    void heap_array_fill_with_unique_value(
        heap_array<T>& array,
        nullptr_t value
        )
    {
        LNA_ASSERT(array._elements);

        // TODO: we will see later for optimization
        for (uint32_t i = 0; i < array._element_count; ++i)
        {
            array._elements[i] = value;
        }
    }

    template<typename T>
    void heap_array_reset(
        heap_array<T>& array
        )
    {
        array._element_count    = 0;
        array._elements         = nullptr;
    }

    template<typename T, uint32_t size>
    struct stack_array
    {
        uint32_t    _element_count  { size };
        T           _elements[size];
    };
}

#endif //_LNA_CORE_CONTAINER_HPP_
