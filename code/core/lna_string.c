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
