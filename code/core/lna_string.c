#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include "core/lna_string.h"
#include "core/lna_assert.h"

void lna_string_copy(char* dst, const char* src, size_t dst_max_size)
{
    lna_assert(dst)
    lna_assert(src)

    size_t src_length = strlen(src);

    lna_assert(src_length > 0)
    lna_assert(src_length < dst_max_size - 1)

    memcpy(
        dst,
        src,
        sizeof(char) * strlen(src)
        );
    dst[src_length] = '\0';
}

void lna_string_to_int(int32_t* dst, const char* src)
{
    lna_assert(dst)
    lna_assert(src)

    char* ptr = 0;
    errno = 0;
    long result = strtol(src, &ptr, 10);
    lna_assert(errno != ERANGE)
    lna_assert(*ptr == '\0')
    lna_assert(ptr != src)
    lna_assert(result >= INT_MIN && result <= INT_MAX)
    *dst = (int)result;
}

void lna_string_to_uint(uint32_t* dst, const char* src)
{
    lna_assert(dst)
    lna_assert(src)

    char* ptr = 0;
    errno = 0;
    long result = strtol(src, &ptr, 10);
    lna_assert(errno != ERANGE)
    lna_assert(*ptr == '\0')
    lna_assert(ptr != src)
    lna_assert(result >= 0 && result <= (long)UINT_MAX)
    *dst = (uint32_t)result;
}

void lna_string_to_float(float* dst, const char* src)
{
    lna_assert(dst)
    lna_assert(src)

    char* ptr = 0;
    errno = 0;
    float result = strtof(src, &ptr);
    lna_assert(errno != ERANGE)
    lna_assert(*ptr == '\0')
    lna_assert(ptr != src)
    *dst = result;
}

void lna_string_to_double(double* dst, const char* src)
{
    lna_assert(dst)
    lna_assert(src)

    char* ptr = 0;
    errno = 0;
    double result = strtod(src, &ptr);
    lna_assert(errno != ERANGE)
    lna_assert(*ptr == '\0')
    lna_assert(ptr != src)
    *dst = result;
}

bool lna_string_begins_with(const char* string, const char* pattern)
{
    lna_assert(string)
    lna_assert(pattern)
    lna_assert(strlen(pattern) > 0)

    // string parameter can be 0 length but not the pattern to find parameter,
    // this is why we only assert if the pattern buffer is 0 length and not if string
    // buffer is 0 length

    size_t  string_length   = strlen(string);
    size_t  pattern_length  = strlen(pattern);
    bool    result          = string_length >= pattern_length;
    if (result)
    {
        for (size_t i = 0; i < pattern_length; ++i)
        {
            if (string[i] != pattern[i])
            {
                result = false;
                break;
            }
        }
    }
    return result;
}

char* lna_string_go_to_next_line(char* string)
{
    lna_assert(string)

    char* result        = 0;
    char* buffer_ptr    = string;
    while (*buffer_ptr != '\0')
    {
        if (*buffer_ptr == '\n')
        {
            result = ++buffer_ptr;
            break;
        }
        ++buffer_ptr;
    }
    return result;
}

char* lna_string_go_to_next_character(char* string, char character)
{
    lna_assert(string)

    char* result = string;
    while (
        *result != character
        && *result != '\0'
        )
    {
        ++result;
    }
    return result;
}

char* lna_string_go_to_next_non_space_character(char* string)
{
    lna_assert(string)

    char* result = string;
    while (
        *result == ' '
        || *result == '\t'
        )
    {
        ++result;
    }
    return result;
}

char* lna_string_go_to_next_space_character(char* string)
{
    lna_assert(string)

    char* result = string;
    while (
        *result != ' '
        && *result != '\t'
        && *result != '\0'
        )
    {
        ++result;
    }
    return result;

}

float lna_string_parse_float(char* string)
{
    lna_assert(string)

    char    buffer[32];
    char    *ptr            = string;
    int     buffer_index    = 0;
    bool    dot_found       = false;
    while (
        (*ptr >= '0' && *ptr <= '9')
        || (!dot_found && *ptr == '.')
        || (buffer_index == 0 && *ptr == '-')
        )
    {
        dot_found = (!dot_found && *ptr == '.') ? true : dot_found;
        lna_assert((uint32_t)buffer_index < (sizeof(buffer) / sizeof(buffer[0])))
        buffer[buffer_index++] = *ptr;
        ++ptr;
    }
    buffer[buffer_index] = '\0';

    float result = 0.0f;
    lna_string_to_float(&result, buffer);
    return result;

}

int lna_string_parse_int(char* string)
{
    lna_assert(string)

    char    buffer[32];
    char    *ptr            = string;
    int     buffer_index    = 0;
    while (
        (*ptr >= '0' && *ptr <= '9')
        || (buffer_index == 0 && *ptr == '-')
        )
    {
        lna_assert((uint32_t)buffer_index < (sizeof(buffer) / sizeof(buffer[0])))
        buffer[buffer_index++] = *ptr;
        ++ptr;
    }
    buffer[buffer_index] = '\0';

    int result = 0;
    lna_string_to_int(&result, buffer);
    return result;

}
