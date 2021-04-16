#include <string.h>
#include <stdio.h>
#include "core/lna_file.h"

void lna_file_debug_load(lna_file_content* file_content, lna_memory_pool_t* memory_pool, const char* filename, bool is_binary)
{
    lna_assert(memory_pool)
    lna_assert(file_content)
    lna_assert(strlen(filename) > 0)

    FILE* fp = NULL;
    fopen_s(&fp, filename, is_binary ? "rb" : "r");
    lna_assert(fp)

    if (fseek(fp, 0L, SEEK_END) == 0)
    {
        long file_length = ftell(fp);
        lna_assert(file_length != -1)

        uint32_t size = is_binary ? (uint32_t)file_length : (uint32_t)file_length + 1;
        lna_array_init(
            file_content,
            memory_pool,
            char,
            size
            );

        if (fseek(fp, 0L, SEEK_SET) != 0)
        {
            lna_assert(0)
        }

        size_t count = fread(file_content->elements, sizeof(char), file_content->element_count, fp);
        lna_assert(ferror(fp) == 0)

        if (!is_binary)
        {
            file_content->elements[count] = '\0';
        }
    }
    fclose(fp);
}
