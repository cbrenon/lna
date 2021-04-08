#ifndef _LNA_CORE_LOG_HPP_
#define _LNA_CORE_LOG_HPP_

#include <cstdint>

namespace lna
{
    struct log
    {
        enum filter
        {
            SHOW_NONE    = 0,
            SHOW_MESSAGE = 1 << 0,
            SHOW_WARNING = 1 << 1,
            SHOW_ERROR   = 1 << 2,
            SHOW_ALL     = SHOW_MESSAGE | SHOW_WARNING | SHOW_ERROR,
        };

        inline static uint32_t _filter;

        static void message (const char* text, ...);
        static void warning (const char* text, ...);
        static void error   (const char* text, ...);
    };
}

#endif // _LNA_CORE_LOG_HPP_
