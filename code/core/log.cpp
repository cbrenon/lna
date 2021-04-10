#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "core/log.hpp"

void lna::log::message(const char* text, ...)
{
    if (lna::log::_filter & lna::log::filter::SHOW_NONE)
    {
        va_list args;
        printf("lna> ");
        va_start(args, text);
        vprintf(text, args);
        va_end(args);
        printf("\n");
    }
}

void lna::log::warning(const char* text, ...)
{
    if (lna::log::_filter & lna::log::filter::SHOW_WARNING)
    {
        va_list args;
        printf("lna> WARNING: ");
        va_start(args, text);
        vprintf(text, args);
        va_end(args);
        printf("\n");
    }
}

void lna::log::error(const char* text, ...)
{
    if (lna::log::_filter & lna::log::filter::SHOW_ERROR)
    {
        va_list args;
        printf("lna> ERROR: ");
        va_start(args, text);
        vprintf(text, args);
        va_end(args);
        printf("\n");
    }
}

void lna::log::debug(const char* text, ...)
{
    if (lna::log::_filter & lna::log::filter::SHOW_ERROR)
    {
        va_list args;
        printf("lna> DEBUG: ");
        va_start(args, text);
        vprintf(text, args);
        va_end(args);
        printf("\n");
    }
}
