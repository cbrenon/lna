#include "backends/std/std_backend.hpp"

namespace lna
{
    void timer_backend_start(
        timer_backend& timer
        )
    {
        timer.start_time = std::chrono::system_clock::now();
        timer.last_frame_time = timer.start_time;
    }

    double timer_backend_dtime_in_s(
        timer_backend& timer
        )
    {
        auto delta_time     = std::chrono::system_clock::now() - timer.last_frame_time;
        auto dtime_in_ms    = std::chrono::duration_cast<std::chrono::seconds>(delta_time);
        timer.last_frame_time += delta_time;
        return dtime_in_ms.count();
    }

    double timer_backend_elapsed_time_in_s(
        timer_backend& timer
        )
    {
        auto current_time = std::chrono::system_clock::now();
        return std::chrono::duration<double, std::chrono::seconds::period>(current_time - timer.start_time).count();
    }
}
