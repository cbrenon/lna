#ifndef _LNA_PLATFORM_SDL_SDL_WINDOW_HPP_
#define _LNA_PLATFORM_SDL_SDL_WINDOW_HPP_

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlanguage-extension-token"
#pragma warning(push, 0)
#define GL3_PROTOTYPES 1
#include <SDL.h>
#include <SDL_vulkan.h>
#pragma warning(pop)
#pragma clang diagnostic pop
#include "platform/window.hpp"

namespace lna
{
    struct window_api
    {
        SDL_Window*             handle;
        uint32_t                width;
        uint32_t                height;
        bool                    fullscreen;
        int                     display_index;
        window_extension_infos  extension_infos;
    };
}

#endif // _LNA_PLATFORM_SDL_SDL_WINDOW_HPP_
