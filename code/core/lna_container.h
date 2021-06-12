#ifndef LNA_CORE_LNA_CONTAINER_H
#define LNA_CORE_LNA_CONTAINER_H

#include <stdint.h>
#include "core/lna_assert.h"
#include "core/lna_memory_pool.h"

//! ----------------------------------------------------------------------------
//!                                 ARRAY
//! ----------------------------------------------------------------------------

#define lna_array_def(type)         \
    typedef struct                  \
    {                               \
        uint32_t  element_count;    \
        type*   elements;           \
    } //! we let user specify the name of the struct

#define lna_array_init(array, memory_pool, type, count)                 \
    lna_assert(array);                                                  \
    lna_assert((array)->elements == 0);                                 \
    lna_assert((array)->element_count == 0);                            \
    (array)->elements = lna_memory_reserve(memory_pool, type, count);   \
    (array)->element_count = count //! we do not add ; to let user add the ; when he uses the macro

#define lna_array_release(array)    \
    (array)->elements = NULL;       \
    (array)->element_count = 0 //! we do not add ; to let user add the ; when he uses the macro

#define lna_array_at_ref(array, index)      ((array)->elements[index])
#define lna_array_at_ptr(array, index)      (&((array)->elements[index]))
#define lna_array_is_empty(array)           ((array)->elements == 0 && (array)->element_count == 0)
#define lna_array_size(array)               ((array)->element_count)
#define lna_array_ptr(array)                ((array)->elements)

//! ----------------------------------------------------------------------------
//!                                 VECTOR
//! ----------------------------------------------------------------------------

#define lna_vector_def(type)            \
    typedef struct                      \
    {                                   \
        uint32_t  cur_element_count;    \
        uint32_t  max_element_count;    \
        type*   elements;               \
    } //! we let user specify the name of the struct

#define lna_vector_init(array, memory_pool, type, count)                \
    lna_assert(array);                                                  \
    lna_assert((array)->elements == 0);                                 \
    lna_assert((array)->cur_element_count == 0);                        \
    lna_assert((array)->max_element_count == 0);                        \
    (array)->elements = lna_memory_reserve(memory_pool, type, count);   \
    (array)->max_element_count = count //! we do not add ; to let user add the ; when he uses the macro

#define lna_vector_new_element(array, result)                               \
    lna_assert(array);                                                      \
    lna_assert((array)->elements);                                          \
    lna_assert((array)->cur_element_count < (array)->max_element_count);    \
    result = &((array)->elements[(array)->cur_element_count++]) //! we do not add ; to let user add the ; when he uses the macro

#define lna_vector_at_ref(array, index) ((array)->elements[index])
#define lna_vector_at_ptr(array, index) (&((array)->elements[index]))
#define lna_vector_size(array)          ((array)->cur_element_count)
#define lna_vector_max_capacity(array)  ((array)->max_element_count)
#define lna_vector_release(array)   \
    (array)->elements = 0;          \
    (array)->cur_element_count = 0; \
    (array)->max_element_count = 0 //! we do not add ; to let user add the ; when he uses the macro

#endif
