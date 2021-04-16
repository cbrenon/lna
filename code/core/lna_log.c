#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "lna_log.h"

static lna_log_level_flags g_log_level = LNA_LOG_LEVEL_ALL;

void lna_log_set_level(lna_log_level_flags flags)
{
    g_log_level = flags;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"

void lna_log_message(const char* txt, ...)
{
    if (g_log_level & LNA_LOG_LEVEL_MESSAGE)
    {
        va_list args;
        printf("lna> ");
        va_start(args, txt);
        vprintf(txt, args);
        va_end(args);
        printf("\n");
    }
}

void lna_log_warning(const char* txt, ...)
{
    if (g_log_level & LNA_LOG_LEVEL_WARNING)
    {
        va_list args;
        printf("lna> WARNING! ");
        va_start(args, txt);
        vprintf(txt, args);
        va_end(args);
        printf("\n");
    }
}

void lna_log_error(const char* txt, ...)
{
    if (g_log_level & LNA_LOG_LEVEL_ERROR)
    {
        va_list args;
        printf("lna> ERROR! ");
        va_start(args, txt);
        vprintf(txt, args);
        va_end(args);
        printf("\n");
    }
}

void lna_log_debug(const char* txt, ...)
{
    if (g_log_level & LNA_LOG_LEVEL_WARNING)
    {
        va_list args;
        printf("lna> DEBUG! ");
        va_start(args, txt);
        vprintf(txt, args);
        va_end(args);
        printf("\n");
    }
}

#pragma clang diagnostic pop
