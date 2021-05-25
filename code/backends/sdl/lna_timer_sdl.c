#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <SDL.h>
#pragma clang diagnostic pop
#include "system/lna_timer.h"
#include "backends/sdl/lna_timer_sdl.h"
#include "core/lna_assert.h"

void lna_timer_start(lna_timer_t* timer)
{
    lna_assert(timer)
    timer->start_frame_time_in_ms   = SDL_GetTicks();
    timer->curr_frame_time_in_ms    = timer->start_frame_time_in_ms;
    timer->last_frame_time_in_ms    = timer->start_frame_time_in_ms;
    timer->delta_time_in_ms         = 0;
}

void lna_timer_update(lna_timer_t* timer)
{
    lna_assert(timer)
    timer->curr_frame_time_in_ms    = SDL_GetTicks();
    timer->elapsed_time_in_ms       = timer->curr_frame_time_in_ms - timer->start_frame_time_in_ms;
    timer->delta_time_in_ms         = timer->curr_frame_time_in_ms - timer->last_frame_time_in_ms;
    timer->last_frame_time_in_ms    = timer->curr_frame_time_in_ms;
}

lna_second_t lna_timer_elasped_time_in_s(lna_timer_t* timer)
{
    lna_assert(timer)
    return (lna_second_t) { .value = (double)timer->elapsed_time_in_ms / 1000.0 };
}

lna_millisecond_t lna_timer_elasped_time_in_ms(lna_timer_t* timer)
{
    lna_assert(timer)
    return (lna_millisecond_t) { .value = (double)timer->elapsed_time_in_ms };
}

lna_second_t lna_timer_delta_time_in_s(lna_timer_t* timer)
{
    lna_assert(timer)
    return (lna_second_t) { .value = (double)timer->delta_time_in_ms / 1000.0 };
}

lna_millisecond_t lna_timer_delta_time_in_ms(lna_timer_t* timer)
{
    lna_assert(timer)
    return (lna_millisecond_t) { .value = (double)timer->delta_time_in_ms };
}
