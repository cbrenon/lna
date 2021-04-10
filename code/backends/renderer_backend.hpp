#ifndef _LNA_BACKENDS_RENDERER_BACKEND_HPP_
#define _LNA_BACKENDS_RENDERER_BACKEND_HPP_

#include <cstdint>
#include "backends/texture_backend.hpp"

namespace lna
{
    class based_allocator;
    struct window_backend;
    struct renderer_backend;

    struct renderer_backend_config
    {
        const char*         application_name;
        uint8_t             application_major_ver;
        uint8_t             application_minor_ver;
        uint8_t             application_patch_ver;
        const char*         engine_name;
        uint8_t             engine_major_ver;
        uint8_t             engine_minor_ver;
        uint8_t             engine_patch_ver;
        bool                enable_validation_layers;
        window_backend*     window_ptr;
        texture_backend*    texture_backend_ptr;
        based_allocator*    allocator_ptr;
    };

    uint32_t renderer_memory_pool_count();

    void renderer_backend_init(
        renderer_backend& renderer
        );

    bool renderer_backend_configure(
        renderer_backend& renderer,
        const renderer_backend_config& config
        );
 
    void renderer_backend_draw_frame(
        renderer_backend& renderer,
        bool framebuffer_resized,
        uint32_t framebuffer_width,
        uint32_t framebuffer_height
        );

    void renderer_backend_release(
        renderer_backend& renderer
        );
}

#endif // _LNA_BACKENDS_RENDERER_BACKEND_HPP_
