#include <vulkan.h>
#include "platform/windows/window_sdl.hpp"
#include "core/assert.hpp"
#include "core/memory_pool.hpp"

namespace lna
{
    template<>
    void window_init<window_sdl>(
        window_sdl& window,
        const window_config& config
        )
    {
        LNA_ASSERT(window._sdl_window == nullptr);
        LNA_ASSERT(config.persistent_pool_ptr);

        {
            auto result = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_TIMER);
            LNA_ASSERT(result == 0);
        }

        window._display_index   = 0; // TODO: let player choose display if multiple monitor?
        window._fullscreen      = config.fullscreen;
        SDL_DisplayMode display_mode;
        SDL_GetCurrentDisplayMode(window._display_index, &display_mode);
        window._width           = config.fullscreen ? static_cast<uint32_t>(display_mode.w) : config.width;
        window._height          = config.fullscreen ? static_cast<uint32_t>(display_mode.h) : config.height;
            

        Uint32 window_create_flags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_VULKAN;
        window_create_flags |= config.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;

        window._sdl_window = SDL_CreateWindow(
            config.title ? config.title : "LNA FRAMEWORK",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            window._width,
            window._height,
            window_create_flags
            );
        LNA_ASSERT(window._sdl_window);

        {
            uint32_t extension_count = 0;
            auto result = SDL_Vulkan_GetInstanceExtensions(
                window._sdl_window,
                &extension_count,
                nullptr
                );
            LNA_ASSERT(result == SDL_TRUE);
            heap_array_set_max_element_count(
                window._extensions,
                *config.persistent_pool_ptr,
                extension_count + 1
                );
            result = SDL_Vulkan_GetInstanceExtensions(window._sdl_window, &extension_count, window._extensions._elements);
            LNA_ASSERT(result == SDL_TRUE);
            if (config.enable_validation_layers)
            {
                window._extensions._elements[extension_count] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
            }
        }
    }

    template<>
    void window_resolution_info_update<window_sdl>(
        window_sdl& window
        )
    {
        SDL_DisplayMode display_mode;
        SDL_GetCurrentDisplayMode(window._display_index, &display_mode);
        window._width  = static_cast<uint32_t>(display_mode.w);
        window._height = static_cast<uint32_t>(display_mode.h);
    }

    template<>
    uint32_t window_width<window_sdl>(
        const window_sdl& window
        )
    {
        return window._width;
    }

    template<>
    uint32_t window_height<window_sdl>(
        const window_sdl& window
        )
    {
        return window._height;
    }

    template<>
    const heap_array<const char*>& window_extensions<window_sdl>(
        const window_sdl& window
        )
    {
        return window._extensions;
    }

    template<>
    SDL_Window* window_handle<window_sdl, SDL_Window*>(
        window_sdl& window
        )
    {
        return window._sdl_window;
    }

    template<>
    void window_release<window_sdl>(
        window_sdl& window
        )
    {
        if (window._sdl_window)
        {
            SDL_DestroyWindow(window._sdl_window);
            window._sdl_window = nullptr;
            window._width = 0;
            window._height = 0;
        }
        SDL_Quit();            
    }
}
