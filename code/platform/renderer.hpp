#ifndef _LNA_PLATFORM_RENDERER_HPP_
#define _LNA_PLATFORM_RENDERER_HPP_

#include "platform/window.hpp"

namespace lna
{
    struct memory_pool_system;

    struct renderer_config
    {
        const char* application_name;
        uint8_t     application_major_ver;
        uint8_t     application_minor_ver;
        uint8_t     application_patch_ver;
        const char* engine_name;
        uint8_t     engine_major_ver;
        uint8_t     engine_minor_ver;
        uint8_t     engine_patch_ver;
        bool        enable_validation_layers;
        uint32_t    max_texture_count;
        uint32_t    max_mesh_count;
        window_api* window_ptr;
    };

    struct renderer_api;

    void renderer_init(
        renderer_api& renderer
        );

    void renderer_configure(
        renderer_api& renderer,
        const renderer_config& config
        );

    void renderer_draw_frame(
        renderer_api& renderer,
        bool framebuffer_resized,
        uint32_t framebuffer_width,
        uint32_t framebuffer_height
        );

    void renderer_release(
        renderer_api& renderer
        );
}

#endif // _LNA_GRAPHICS_RENDERER_HPP_
