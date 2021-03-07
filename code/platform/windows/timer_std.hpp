#ifndef _LNA_PLATFORM_WINDOWS_TIMER_STD_HPP_
#define _LNA_PLATFORM_WINDOWS_TIMER_STD_HPP_

#include <chrono>
#include "platform/timer.hpp"

namespace lna
{
    struct timer_std
    {
        std::chrono::time_point<std::chrono::system_clock> _last_frame_time;
    };

    template<>
    void timer_start<timer_std>(
        timer_std& timer
        );

    template<>
    double timer_dtime_in_ms<timer_std>(
        timer_std& timer
        );
}

#endif // _LNA_PLATFORM_WINDOWS_TIMER_STD_HPP_
