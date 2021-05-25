#ifndef LNA_BACKENDS_SDL_LNA_WINDOW_SDL_H
#define LNA_BACKENDS_SDL_LNA_WINDOW_SDL_H

#include <stdbool.h>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#pragma warning(push, 0)
#include <SDL.h>
#include <SDL_vulkan.h>
#pragma warning(pop)
#pragma clang diagnostic pop

typedef struct lna_window_s
{
    SDL_Window* handle;
    uint32_t    width;
    uint32_t    height;
    int         display_index;
} lna_window_t;

extern void lna_window_vulkan_extension_count(const lna_window_t* window, uint32_t* extension_count);
extern void lna_window_vulkan_extension_names(const lna_window_t* window, uint32_t* extension_count, const char** extension_names);
extern void lna_window_vulkan_create_surface(const lna_window_t* window, VkInstance instance, VkSurfaceKHR* surface);

#endif
