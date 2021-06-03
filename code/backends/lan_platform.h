#ifndef LNA_BACKENDS_LNA_PLATFORM_H
#define LNA_BACKENDS_LNA_PLATFORM_H

#ifdef _WIN32
#include "backends/sdl/lna_window_sdl.h"
#include "backends/sdl/lna_input_sdl.h"
#include "backends/sdl/lna_timer_sdl.h"
#include "backends/vulkan/lna_renderer_vulkan.h"
#include "backends/vulkan/lna_ui_vulkan.h"
#include "backends/vulkan/lna_texture_vulkan.h"
#include "backends/vulkan/lna_sprite_vulkan.h"
#include "backends/vulkan/lna_primitive_vulkan.h"
#include "backends/vulkan/lna_mesh_vulkan.h"
#endif

#endif
