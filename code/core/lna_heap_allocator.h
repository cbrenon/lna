#ifndef LNA_CORE_LNA_HEAP_ALLOCATOR_H
#define LNA_CORE_LNA_HEAP_ALLOCATOR_H

#include <stddef.h>

typedef struct lna_heap_allocator_s
{
    size_t  cur_content_offset;
    size_t  max_content_size;
    char*   content;
} lna_heap_allocator_t;

extern void     lna_heap_allocator_init      (lna_heap_allocator_t* allocator, size_t max_size_in_bytes);
extern char*    lna_heap_allocator_alloc     (lna_heap_allocator_t* allocator, size_t size_in_bytes);
extern void     lna_heap_allocator_release   (lna_heap_allocator_t* allocator);

#endif
