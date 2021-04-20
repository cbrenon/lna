#ifndef LNA_CORE_LNA_FILE_H
#define LNA_CORE_LNA_FILE_H

#include <stdbool.h>
#include "core/lna_container.h"

lna_array_def(char) lna_file_content_t;

extern void lna_file_debug_load(lna_file_content_t* file_content, lna_memory_pool_t* memory_pool, const char* filename, bool is_binary);

#endif
