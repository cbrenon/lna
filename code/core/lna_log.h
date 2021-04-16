#ifndef LNA_CORE_LNA_LOG_H
#define LNA_CORE_LNA_LOG_H

#include <stdint.h>

typedef enum lna_log_level_e
{
    LNA_LOG_LEVEL_NONE      = 0,
    LNA_LOG_LEVEL_MESSAGE   = 1 << 0,
    LNA_LOG_LEVEL_WARNING   = 1 << 1,
    LNA_LOG_LEVEL_ERROR     = 1 << 2,
    LNA_LOG_LEVEL_DEBUG     = 1 << 3,
    LNA_LOG_LEVEL_ALL       = LNA_LOG_LEVEL_MESSAGE | LNA_LOG_LEVEL_WARNING | LNA_LOG_LEVEL_ERROR | LNA_LOG_LEVEL_DEBUG,
} lna_log_level_t;

typedef uint32_t lna_log_level_flags;

extern void lna_log_set_level   (lna_log_level_flags flags);
extern void lna_log_message     (const char* txt, ...);
extern void lna_log_warning     (const char* txt, ...);
extern void lna_log_error       (const char* txt, ...);
extern void lna_log_debug       (const char* txt, ...);

#endif
