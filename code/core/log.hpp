#ifndef _LNA_CORE_LOG_HPP_
#define _LNA_CORE_LOG_HPP_

#include <cstdint>

namespace lna
{
    struct log
    {
        enum level
        {
            NONE    = 0,
            MESSAGE = 1 << 0,
            WARNING = 1 << 1,
            ERROR   = 1 << 2,
            ALL     = MESSAGE | WARNING | ERROR,
        };

        inline static uint32_t _level;

        static void message (const char* text, ...);
        static void warning (const char* text, ...);
        static void error   (const char* text, ...);
    };
}

#endif // _LNA_CORE_LOG_HPP_
