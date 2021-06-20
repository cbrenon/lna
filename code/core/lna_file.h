#ifndef LNA_CORE_LNA_FILE_H
#define LNA_CORE_LNA_FILE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct lna_memory_pool_s lna_memory_pool_t;

typedef struct lna_file_content_s
{
    char*       content;
    size_t      size;
} lna_file_content_t;

typedef struct lna_binary_file_content_uint32_s
{
    uint32_t*   content;
    size_t      size;
} lna_binary_file_content_uint32_t;

extern void lna_file_debug_load(lna_file_content_t* file_content, lna_memory_pool_t* memory_pool, const char* filename, bool is_binary);
extern void lna_binary_file_debug_load_uint32(lna_binary_file_content_uint32_t* file_content, lna_memory_pool_t* memory_pool, const char* filename);

#endif
