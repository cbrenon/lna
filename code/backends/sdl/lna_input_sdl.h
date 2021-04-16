#ifndef LNA_BACKENDS_SDL_LNA_INPUT_SDL_H
#define LNA_BACKENDS_SDL_LNA_INPUT_SDL_H

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#define GL3_PROTOTYPES 1
#include <SDL.h>
#pragma clang diagnostic pop

#include "backends/lna_input.h"

typedef struct lna_input_s
{
    const Uint8*    keyboard_state;
    Uint8           prev_keyboard_state[SDL_NUM_SCANCODES];
} lna_input_t;

#endif
