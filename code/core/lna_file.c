#include <string.h>
#include <stdio.h>
#include "core/lna_file.h"
#include "core/lna_assert.h"
#include "core/lna_memory_pool.h"

void lna_file_debug_load(lna_file_content_t* file_content, lna_memory_pool_t* memory_pool, const char* filename, bool is_binary)
{
    lna_assert(memory_pool)
    lna_assert(file_content)
    lna_assert(file_content->content == NULL)
    lna_assert(file_content->size == 0)
    lna_assert(strlen(filename) > 0)

    FILE* fp = NULL;
    fopen_s(&fp, filename, is_binary ? "rb" : "r");
    lna_assert(fp)

    if (fseek(fp, 0L, SEEK_END) == 0)
    {
        long file_length = ftell(fp);
        lna_assert(file_length != -1)

        file_content->size      = is_binary ? (size_t)file_length : (size_t)file_length + 1;
        file_content->content   = lna_memory_pool_reserve(memory_pool, file_content->size * sizeof(char));

        if (fseek(fp, 0L, SEEK_SET) != 0)
        {
            lna_assert(0)
        }

        size_t count = fread(
            file_content->content,
            sizeof(char),
            file_content->size,
            fp
            );
        lna_assert(ferror(fp) == 0)

        if (!is_binary)
        {
            file_content->content[count] = '\0';
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

        file_content->size      = (size_t)file_length;
        file_content->content   = lna_memory_pool_reserve(memory_pool, file_content->size * sizeof(uint32_t));

        if (fseek(fp, 0L, SEEK_SET) != 0)
        {
            lna_assert(0)
        }

        fread(
            file_content->content,
            sizeof(uint32_t),
            file_content->size,
            fp
            );
        lna_assert(ferror(fp) == 0)
    }
    fclose(fp);
}
