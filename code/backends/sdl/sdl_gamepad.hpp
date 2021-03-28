#ifndef _LNA_BACKENDS_SDL_SDL_GAMEPAD_HPP_
#define _LNA_BACKENDS_SDL_SDL_GAMEPAD_HPP_

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlanguage-extension-token"
#pragma warning(push, 0)
#define GL3_PROTOTYPES 1
#include <SDL.h>
#pragma warning(pop)
#pragma clang diagnostic pop
#include "backends/gamepad.hpp"

namespace lna
{
    struct gamepad_api
    {
        gamepad_info        info;
        SDL_GameController* device;
    };
}

#endif // _LNA_BACKENDS_SDL_SDL_GAMEPAD_HPP_
