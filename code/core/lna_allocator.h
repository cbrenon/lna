#ifndef LNA_CORE_LNA_ALLOCATOR_H
#define LNA_CORE_LNA_ALLOCATOR_H

#include <stddef.h>

typedef struct lna_allocator_s
{
    size_t  cur_content_offset;
    size_t  max_content_size;
    char*   content;
} lna_allocator_t;

extern void     lna_allocator_init      (lna_allocator_t* allocator, size_t max_size_in_bytes);
extern char*    lna_allocator_alloc     (lna_allocator_t* allocator, size_t size_in_bytes);
extern void     lna_allocator_release   (lna_allocator_t* allocator);

#define LNA_KILOBYTES(value)    ((value) * 1024LL)
#define LNA_MEGABYTES(value)    (LNA_KILOBYTES(value) * 1024LL)
#define LNA_GIGABYTES(value)    (LNA_MEGABYTES(value) * 1024LL)

#endif
