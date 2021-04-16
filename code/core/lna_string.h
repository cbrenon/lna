#ifndef LNA_CORE_LNA_STRING_H
#define LNA_CORE_LNA_STRING_H

#include <stdint.h>

extern void lna_string_copy         (char* dst, const char* src, size_t dst_max_size);
extern void lna_string_to_int       (int32_t* dst, const char* src);
extern void lna_string_to_uint      (uint32_t* dst, const char* src);
extern void lna_string_to_float     (float* dst, const char* src);
extern void lna_string_to_double    (double* dst, const char* src);

#endif // LNA_CORE_LNA_STRING_H
