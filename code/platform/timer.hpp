#ifndef _LNA_PLATFORM_TIMER_HPP_
#define _LNA_PLATFORM_TIMER_HPP_

namespace lna
{
    struct timer_api;

    void timer_start(
        timer_api& timer
        );

    double timer_dtime_in_ms(
        timer_api& timer
        );
}

#endif // _LNA_PLATFORM_TIMER_HPP_
