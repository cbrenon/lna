#ifndef _LNA_PLATFORM_WINDOWS_TIMER_STD_HPP_
#define _LNA_PLATFORM_WINDOWS_TIMER_STD_HPP_

#include <chrono>
#include "platform/timer.hpp"

namespace lna
{
    struct timer_api
    {
        std::chrono::time_point<std::chrono::system_clock> last_frame_time;
    };
}

#endif // _LNA_PLATFORM_WINDOWS_TIMER_STD_HPP_
