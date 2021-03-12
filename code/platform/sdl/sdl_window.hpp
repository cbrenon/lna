#ifndef _NLA_PLATFORM_SDL_SDL_WINDOW_HPP_
#define _NLA_PLATFORM_SDL_SDL_WINDOW_HPP_

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
    struct sdl_window
    {
        SDL_Window*             handle;
        uint32_t                width;
        uint32_t                height;
        bool                    fullscreen;
        int                     display_index;
        heap_array<const char*> extensions;
    };

    template<>
    void window_init<sdl_window>(
        sdl_window& window
        );

    template<>
    void window_configure(
        sdl_window& window,
        const window_config& config
        );

    template<>
    void window_resolution_info_update<sdl_window>(
        sdl_window& window
        );

    template<>
    uint32_t window_width<sdl_window>(
        const sdl_window& window
        );

    template<>
    uint32_t window_height<sdl_window>(
        const sdl_window& window
        );

    template<>
    const heap_array<const char*>& window_extensions<sdl_window>(
        const sdl_window& window
        );

    template<>
    SDL_Window* window_handle<sdl_window, SDL_Window*>(
        sdl_window& window
        );

    template<>
    void window_release<sdl_window>(
        sdl_window& window
        );
}

#endif // _NLA_PLATFORM_WINDOWS_WINDOW_SDL_HPP_
