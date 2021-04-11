#ifndef _LNA_BACKENDS_SDL_SDL_BACKEND_HPP_
#define _LNA_BACKENDS_SDL_SDL_BACKEND_HPP_

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlanguage-extension-token"
#pragma warning(push, 0)
#define GL3_PROTOTYPES 1
#include <SDL.h>
#include <SDL_vulkan.h>
#pragma warning(pop)
#pragma clang diagnostic pop
#include "backends/window_backend.hpp"
#include "backends/input_backend.hpp"
#include "backends/gamepad_backend.hpp"

namespace lna
{
    struct window_backend
    {
        SDL_Window*             handle;
        uint32_t                width;
        uint32_t                height;
        bool                    fullscreen;
        int                     display_index;
        window_extension_infos  extension_infos;
    };

    struct input_backend
    {
        Uint8                   curr_frame_keyboard_state[SDL_NUM_SCANCODES];
        mouse                   mouse_info;
    };

    struct gamepad_backend
    {
        gamepad_info            info;
        SDL_GameController*     device;
    };
}

#endif // _LNA_BACKENDS_SDL_SDL_BACKEND_HPP_
