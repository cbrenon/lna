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
        uint32_t    element_count;
        T*          elements;
    };

    template<typename T>
    void heap_array_init(
        heap_array<T>& array
        )
    {
        array.element_count = 0;
        array.elements      = nullptr;
    }

    template<typename T>
    void heap_array_set_max_element_count(
        heap_array<T>& array,
        memory_pool& pool,
        uint32_t max_element_count
        )
    {
        LNA_ASSERT(array.element_count == 0);
        LNA_ASSERT(array.elements == nullptr);

        array.elements = (T*)memory_pool_reserve(
            pool,
            max_element_count * sizeof(T)
            );
        array.element_count = max_element_count;
    }

    template<typename T>
    void heap_array_fill_with_unique_value(
        heap_array<T>& array,
        T& value
        )
    {
        LNA_ASSERT(array.elements);

        // TODO: we will see later for optimization
        for (uint32_t i = 0; i < array.element_count; ++i)
        {
            array.elements[i] = value;
        }
    }

    template<typename T>
    void heap_array_fill_with_unique_value(
        heap_array<T>& array,
        nullptr_t value
        )
    {
        LNA_ASSERT(array.elements);

        // TODO: we will see later for optimization
        for (uint32_t i = 0; i < array.element_count; ++i)
        {
            array.elements[i] = value;
        }
    }

    template<typename T>
    void heap_array_reset(
        heap_array<T>& array
        )
    {
        array.element_count    = 0;
        array.elements         = nullptr;
    }

    template<typename T>
    bool heap_array_has_been_reset(
        heap_array<T>& array
        )
    {
        return array.element_count == 0 && array.elements == nullptr;
    }

    // template<typename T, uint32_t size>
    // struct stack_array
    // {
    //     const uint32_t  element_count = size;
    //     T               elements[size];
    // };
}

#endif //_LNA_CORE_CONTAINER_HPP_
