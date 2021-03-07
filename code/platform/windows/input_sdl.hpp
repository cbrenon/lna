#ifndef _LNA_PLATFORM_WINDOWS_INPUT_SDL_HPP_
#define _LNA_PLATFORM_WINDOWS_INPUT_SDL_HPP_

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
    struct input_sdl
    {
        std::array<Uint8, SDL_NUM_SCANCODES> _curr_frame_keyboard_state {};
    };

    template<>
    input_event input_poll_events<input_sdl>(
        input_sdl& input
        );

    template<>
    bool input_is_key_pressed<input_sdl>(
        input_sdl& input,
        key k
        );
}

#endif // _LNA_PLATFORM_WINDOWS_INPUT_SDL_HPP_
