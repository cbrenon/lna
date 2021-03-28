#ifndef _LNA_BACKENDS_RENDERER_HPP_
#define _LNA_BACKENDS_RENDERER_HPP_

#include <cstdint>
#include "graphics/vertex.hpp"

namespace lna
{
    struct memory_pool_manager;
    struct window_api;
    struct renderer_api;

    typedef uint32_t texture_handle;
    typedef uint32_t mesh_handle;

    constexpr uint32_t INVALID_TEXTURE_HANDLE   = (uint32_t)-1;
    constexpr uint32_t INVALID_MESH_HANDLE      = (uint32_t)-1;

    struct renderer_config
    {
        const char*             application_name;
        uint8_t                 application_major_ver;
        uint8_t                 application_minor_ver;
        uint8_t                 application_patch_ver;
        const char*             engine_name;
        uint8_t                 engine_major_ver;
        uint8_t                 engine_minor_ver;
        uint8_t                 engine_patch_ver;
        bool                    enable_validation_layers;
        uint32_t                max_texture_count;
        uint32_t                max_mesh_count;
        window_api*             window_ptr;
        memory_pool_manager*    mem_pool_manager_ptr;
    };

    struct texture_config
    {
        const char* filename;
    };

    struct mesh_config
    {
        const vertex*   vertices;
        const uint16_t* indices;
        uint32_t        vertex_count;
        uint32_t        index_count;
        vec3            position;
        texture_handle  texture;
    };

    uint32_t renderer_memory_pool_count();

    void renderer_init(
        renderer_api& renderer
        );

    void renderer_configure(
        renderer_api& renderer,
        const renderer_config& config
        );

    texture_handle renderer_new_texture(
        renderer_api& renderer,
        const texture_config& config
        );

    mesh_handle renderer_new_mesh(
        renderer_api& renderer,
        const mesh_config& config
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

#endif // _LNA_BACKENDS_RENDERER_HPP_
