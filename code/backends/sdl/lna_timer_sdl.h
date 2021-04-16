#ifndef LNA_BACKENDS_SDL_LNA_TIMER_SDL_H
#define LNA_BACKENDS_SDL_LNA_TIMER_SDL_H

#include <stdint.h>
#include "backends/lna_timer.h"

typedef struct lna_timer_s
{
    uint32_t    curr_frame_time_in_ms;
    uint32_t    last_frame_time_in_ms;
    uint32_t    start_frame_time_in_ms;
    uint32_t    elapsed_time_in_ms;
    uint32_t    delta_time_in_ms;
} lna_timer_t;

#endif
