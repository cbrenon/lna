#include "platform/windows/std_timer.hpp"

namespace lna
{
    void timer_start(
        timer_api& timer
        )
    {
        timer.last_frame_time = std::chrono::system_clock::now();
    }

    double timer_dtime_in_ms(
        timer_api& timer
        )
    {
        auto delta_time     = std::chrono::system_clock::now() - timer.last_frame_time;
        auto dtime_in_ms    = std::chrono::duration_cast<std::chrono::milliseconds>(delta_time);
        timer.last_frame_time += delta_time;
        return dtime_in_ms.count();
    }
}
