#ifndef _LNA_BACKENDS_TIMER_BACKEND_HPP_
#define _LNA_BACKENDS_TIMER_BACKEND_HPP_

namespace lna
{
    struct timer_backend;

    void timer_backend_start(
        timer_backend& timer
        );

    double timer_backend_dtime_in_ms(
        timer_backend& timer
        );

    double timer_backend_elapsed_time_in_s(
        timer_backend& timer
        );
}

#endif // _LNA_BACKENDS_TIMER_BACKEND_HPP_
