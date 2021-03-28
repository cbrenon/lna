#ifndef _LNA_BACKENDS_SDL_SDL_INPUT_HPP_
#define _LNA_BACKENDS_SDL_SDL_INPUT_HPP_

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlanguage-extension-token"
#pragma warning(push, 0)
#define GL3_PROTOTYPES 1
#include <SDL.h>
#pragma warning(pop)
#pragma clang diagnostic pop
#include "backends/input.hpp"

namespace lna
{
    struct input_api
    {
        Uint8 curr_frame_keyboard_state[SDL_NUM_SCANCODES];
    };
}

#endif // _LNA_BACKENDS_SDL_SDL_INPUT_HPP_
