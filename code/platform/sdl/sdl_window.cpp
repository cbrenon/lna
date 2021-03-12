#include <vulkan.h>
#include "platform/sdl/sdl_window.hpp"
#include "core/assert.hpp"
#include "core/memory_pool.hpp"

namespace lna
{
    template<>
    void window_init(
        sdl_window& window
        )
    {
        window.handle           = nullptr;
        window.width            = 0;
        window.height           = 0;
        window.fullscreen       = false;
        window.display_index    = 0;
        lna::heap_array_init(
            window.extensions
            );
    }

    template<>
    void window_configure<sdl_window>(
        sdl_window& window,
        const window_config& config
        )
    {
        LNA_ASSERT(window.handle == nullptr);
        LNA_ASSERT(config.persistent_pool_ptr);

        {
            auto result = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_TIMER);
            LNA_ASSERT(result == 0);
        }

        window.display_index   = 0; // TODO: let player choose display if multiple monitor?
        window.fullscreen      = config.fullscreen;
        SDL_DisplayMode display_mode;
        SDL_GetCurrentDisplayMode(window.display_index, &display_mode);
        window.width    = config.fullscreen ? static_cast<uint32_t>(display_mode.w) : config.width;
        window.height   = config.fullscreen ? static_cast<uint32_t>(display_mode.h) : config.height;
            

        Uint32 window_create_flags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_VULKAN;
        window_create_flags |= config.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;

        window.handle = SDL_CreateWindow(
            config.title ? config.title : "LNA FRAMEWORK",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            window.width,
            window.height,
            window_create_flags
            );
        LNA_ASSERT(window.handle);

        {
            uint32_t extension_count = 0;
            auto result = SDL_Vulkan_GetInstanceExtensions(
                window.handle,
                &extension_count,
                nullptr
                );
            LNA_ASSERT(result == SDL_TRUE);
            heap_array_set_max_element_count(
                window.extensions,
                *config.persistent_pool_ptr,
                extension_count + 1
                );
            result = SDL_Vulkan_GetInstanceExtensions(
                window.handle,
                &extension_count,
                window.extensions.elements
                );
            LNA_ASSERT(result == SDL_TRUE);
            if (config.enable_validation_layers)
            {
                window.extensions.elements[extension_count] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
            }
        }
    }

    template<>
    uint32_t window_width<sdl_window>(
        const sdl_window& window
        )
    {
        return window.width;
    }

    template<>
    uint32_t window_height<sdl_window>(
        const sdl_window& window
        )
    {
        return window.height;
    }

    template<>
    const heap_array<const char*>& window_extensions<sdl_window>(
        const sdl_window& window
        )
    {
        return window.extensions;
    }

    template<>
    SDL_Window* window_handle<sdl_window, SDL_Window*>(
        sdl_window& window
        )
    {
        return window.handle;
    }

    template<>
    void window_resolution_info_update<sdl_window>(
        sdl_window& window
        )
    {
        SDL_DisplayMode display_mode;
        SDL_GetCurrentDisplayMode(window.display_index, &display_mode);
        window.width  = static_cast<uint32_t>(display_mode.w);
        window.height = static_cast<uint32_t>(display_mode.h);
    }

    template<>
    void window_release<sdl_window>(
        sdl_window& window
        )
    {
        if (window.handle)
        {
            SDL_DestroyWindow(window.handle);
            window.handle   = nullptr;
            window.width    = 0;
            window.height   = 0;
        }
        SDL_Quit();            
    }
}
