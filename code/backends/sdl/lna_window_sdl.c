#include "system/lna_window.h"
#include "backends/sdl/lna_window_sdl.h"
#include "core/lna_assert.h"
#include "core/lna_memory_pool.h"

void lna_window_init(lna_window_t* window, const lna_window_config_t* config)
{
    lna_assert(window)
    lna_assert(window->handle == 0)
    lna_assert(window->width == 0)
    lna_assert(window->height == 0)
    lna_assert(config)

    int result = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_TIMER);
    lna_assert(result == 0)

    window->display_index = 0; // TODO: let player choose display if multiple monitor?
    SDL_DisplayMode display_mode;
    SDL_GetCurrentDisplayMode(window->display_index, &display_mode);
    window->width   = config->fullscreen ? (uint32_t)(display_mode.w) : config->width;
    window->height  = config->fullscreen ? (uint32_t)(display_mode.h) : config->height;

    Uint32 window_create_flags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_VULKAN;
    window_create_flags |= config->fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;

    window->handle = SDL_CreateWindow(
        config->title ? config->title : "LNA FRAMEWORK",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        (int)window->width,
        (int)window->height,
        window_create_flags
        );
    lna_assert(window->handle)
}

void lna_window_set_title(lna_window_t* window, const char* title)
{
    lna_assert(window)
    lna_assert(window->handle)
    SDL_SetWindowTitle(window->handle, title);
}

void lna_window_on_resize(lna_window_t* window)
{
    lna_assert(window)
    SDL_DisplayMode display_mode;
    SDL_GetCurrentDisplayMode(window->display_index, &display_mode);
    window->width  = (uint32_t)(display_mode.w);
    window->height = (uint32_t)(display_mode.h);
}

uint32_t lna_window_width(const lna_window_t* window)
{
    lna_assert(window)
    return window->width;
}

uint32_t lna_window_height(const lna_window_t* window)
{
    lna_assert(window)
    return window->height;
}

void lna_window_release(lna_window_t* window)
{
    lna_assert(window)

    if (window->handle)
    {
        SDL_DestroyWindow(window->handle);
        window->handle  = NULL;
        window->width   = 0;
        window->height  = 0;
    }
    SDL_Quit();  
}

void lna_window_vulkan_extension_count(const lna_window_t* window, uint32_t* extension_count)
{
    lna_assert(window)
    lna_assert(window->handle)
    lna_assert(extension_count)

    SDL_bool result = SDL_Vulkan_GetInstanceExtensions(
        window->handle,
        extension_count,
        NULL
        );
    lna_assert(result == SDL_TRUE)
}

void lna_window_vulkan_extension_names(const lna_window_t* window, uint32_t* extension_count, const char** extension_names)
{
    lna_assert(window)
    lna_assert(window->handle)
    lna_assert(extension_count)
    lna_assert(extension_names)

    SDL_bool result = SDL_Vulkan_GetInstanceExtensions(
        window->handle,
        extension_count,
        extension_names
        );
    lna_assert(result == SDL_TRUE)
}

void lna_window_vulkan_create_surface(const lna_window_t* window, VkInstance instance, VkSurfaceKHR* surface)
{
    lna_assert(window)
    lna_assert(instance)
    lna_assert(surface)

    SDL_bool result = SDL_Vulkan_CreateSurface(
        window->handle,
        instance,
        surface
        );
    lna_assert(result == SDL_TRUE)
}
