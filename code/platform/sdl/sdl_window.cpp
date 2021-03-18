#include <vulkan.h>
#include "platform/sdl/sdl_window.hpp"
#include "core/assert.hpp"
#include "core/memory_pool.hpp"

namespace lna
{
    void window_init(
        window_api& window
        )
    {
        window.handle                   = nullptr;
        window.width                    = 0;
        window.height                   = 0;
        window.fullscreen               = false;
        window.display_index            = 0;
        window.extension_infos.names    = nullptr;
        window.extension_infos.count    = 0;
    }

    void window_configure(
        window_api& window,
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

            window.extension_infos.count    = config.enable_validation_layers ? extension_count + 1 : extension_count;
            window.extension_infos.names    = (const char **)memory_pool_reserve(
                *config.persistent_pool_ptr,
                window.extension_infos.count * sizeof(const char*)
                );
            LNA_ASSERT(window.extension_infos.names);

            result = SDL_Vulkan_GetInstanceExtensions(
                window.handle,
                &extension_count,
                window.extension_infos.names
                );
            LNA_ASSERT(result == SDL_TRUE);
            if (config.enable_validation_layers)
            {
                window.extension_infos.names[extension_count] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
            }
        }
    }

    uint32_t window_width(
        const window_api& window
        )
    {
        return window.width;
    }

    uint32_t window_height(
        const window_api& window
        )
    {
        return window.height;
    }

    const window_extension_infos& window_extensions(
        const window_api& window
        )
    {
        return window.extension_infos;
    }

    void window_resolution_info_update(
        window_api& window
        )
    {
        SDL_DisplayMode display_mode;
        SDL_GetCurrentDisplayMode(window.display_index, &display_mode);
        window.width  = static_cast<uint32_t>(display_mode.w);
        window.height = static_cast<uint32_t>(display_mode.h);
    }

    void window_release(
        window_api& window
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
