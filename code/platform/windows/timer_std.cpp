#include "platform/windows/timer_std.hpp"

namespace lna
{
    template<>
    void timer_start<timer_std>(
        timer_std& timer
        )
    {
        timer._last_frame_time = std::chrono::system_clock::now();
    }

    template<>
    double timer_dtime_in_ms<timer_std>(
        timer_std& timer
        )
    {
        auto delta_time         = std::chrono::system_clock::now() - timer._last_frame_time;
        auto dtime_in_ms        = std::chrono::duration_cast<std::chrono::milliseconds>(delta_time);
        timer._last_frame_time += delta_time;
        return dtime_in_ms.count();
    }
}
