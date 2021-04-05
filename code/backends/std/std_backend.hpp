#ifndef _LNA_BACKENDS_STD_STD_BACKEND_HPP_
#define _LNA_BACKENDS_STD_STD_BACKEND_HPP_

#include <chrono>
#include "backends/timer_backend.hpp"

namespace lna
{
    struct timer_backend
    {
        std::chrono::time_point<std::chrono::system_clock> start_time;
        std::chrono::time_point<std::chrono::system_clock> last_frame_time;
    };
}

#endif // _LNA_BACKENDS_STD_STD_BACKEND_HPP_
