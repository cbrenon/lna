#ifndef _LNA_PLATFORM_SDL_SDL_INPUT_HPP_
#define _LNA_PLATFORM_SDL_SDL_INPUT_HPP_

#include <array>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlanguage-extension-token"
#pragma warning(push, 0)
#define GL3_PROTOTYPES 1
#include <SDL.h>
#pragma warning(pop)
#pragma clang diagnostic pop
#include "platform/input.hpp"

namespace lna
{
    struct sdl_input
    {
        std::array<Uint8, SDL_NUM_SCANCODES> curr_frame_keyboard_state {};
    };

    template<>
    input_event input_poll_events<sdl_input>(
        sdl_input& input
        );

    template<>
    bool input_is_key_pressed<sdl_input>(
        sdl_input& input,
        key k
        );
}

#endif // _LNA_PLATFORM_SDL_SDL_INPUT_HPP_
