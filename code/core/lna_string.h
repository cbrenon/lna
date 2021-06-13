#ifndef LNA_CORE_LNA_STRING_H
#define LNA_CORE_LNA_STRING_H

#include <stdint.h>
#include <stdbool.h>

extern void     lna_string_copy                             (char* dst, const char* src, size_t dst_max_size);
extern void     lna_string_to_int                           (int32_t* dst, const char* src);
extern void     lna_string_to_uint                          (uint32_t* dst, const char* src);
extern void     lna_string_to_float                         (float* dst, const char* src);
extern void     lna_string_to_double                        (double* dst, const char* src);
extern bool     lna_string_begins_with                      (const char* string, const char* pattern);
extern char*    lna_string_go_to_next_line                  (char* string);
extern char*    lna_string_go_to_next_character             (char* string, char character);
extern char*    lna_string_go_to_next_non_space_character   (char* string);
extern char*    lna_string_go_to_next_space_character       (char* string);
extern float    lna_string_parse_float                      (char* string);
extern int      lna_string_parse_int                        (char* string);

#endif // LNA_CORE_LNA_STRING_H
