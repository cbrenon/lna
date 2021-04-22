#include <string.h>
#include <stdio.h>
#include "core/lna_file.h"

void lna_file_debug_load(lna_file_content_t* file_content, lna_memory_pool_t* memory_pool, const char* filename, bool is_binary)
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

        size_t count = fread(
            lna_array_ptr(file_content),
            sizeof(char),
            lna_array_size(file_content),
            fp
            );
        lna_assert(ferror(fp) == 0)

        if (!is_binary)
        {
            lna_array_at_ref(file_content, count) = '\0';
        }
    }
    fclose(fp);
}

void lna_binary_file_debug_load_uint32(lna_binary_file_content_uint32_t* file_content, lna_memory_pool_t* memory_pool, const char* filename)
{
    lna_assert(memory_pool)
    lna_assert(file_content)
    lna_assert(strlen(filename) > 0)

    FILE* fp = NULL;
    fopen_s(&fp, filename, "rb");
    lna_assert(fp)

    if (fseek(fp, 0L, SEEK_END) == 0)
    {
        long file_length = ftell(fp);
        lna_assert(file_length != -1)

        lna_array_init(
            file_content,
            memory_pool,
            uint32_t,
            (uint32_t)file_length
            );

        if (fseek(fp, 0L, SEEK_SET) != 0)
        {
            lna_assert(0)
        }

        fread(
            lna_array_ptr(file_content),
            sizeof(uint32_t),
            lna_array_size(file_content),
            fp
            );
        lna_assert(ferror(fp) == 0)
    }
    fclose(fp);
}
