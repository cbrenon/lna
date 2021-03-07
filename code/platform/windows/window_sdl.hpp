#ifndef _NLA_PLATFORM_WINDOWS_WINDOW_SDL_HPP_
#define _NLA_PLATFORM_WINDOWS_WINDOW_SDL_HPP_

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlanguage-extension-token"
#pragma warning(push, 0)
#define GL3_PROTOTYPES 1
#include <SDL.h>
#include <SDL_vulkan.h>
#pragma warning(pop)
#pragma clang diagnostic pop

#include <vector>
#include "platform/window.hpp"

namespace lna
{
    struct window_sdl
    {
        SDL_Window*                 _sdl_window     { nullptr };
        uint32_t                    _width          { 0 };
        uint32_t                    _height         { 0 };
        bool                        _fullscreen     { false };
        int                         _display_index  { 0 };
        std::vector<const char*>    _extensions;
    };

    template<>
    void window_init<window_sdl>(
        window_sdl& window,
        const window_config& config
        );

    template<>
    void window_resolution_info_update<window_sdl>(
        window_sdl& window
        );

    template<>
    uint32_t window_width<window_sdl>(
        const window_sdl& window
        );

    template<>
    uint32_t window_height<window_sdl>(
        const window_sdl& window
        );

    template<>
    const std::vector<const char*>& window_extensions<window_sdl>(
        const window_sdl& window
        );

    template<>
    SDL_Window* window_handle<window_sdl, SDL_Window*>(
        window_sdl& window
        );

    template<>
    void window_release<window_sdl>(
        window_sdl& window
        );
}

#endif // _NLA_PLATFORM_WINDOWS_WINDOW_SDL_HPP_
