#ifndef _LNA_BACKENDS_RENDERER_BACKEND_HPP_
#define _LNA_BACKENDS_RENDERER_BACKEND_HPP_

#include <cstdint>
#include "backends/texture_backend.hpp"

namespace lna
{
    struct memory_pool_manager;
    struct window_backend;
    struct renderer_backend;

    //typedef uint32_t texture_handle; // TODO: remove
    //typedef uint32_t mesh_handle; // TODO: remove
    //constexpr uint32_t INVALID_TEXTURE_HANDLE   = (uint32_t)-1; // TODO: remove
    //constexpr uint32_t INVALID_MESH_HANDLE      = (uint32_t)-1; // TODO: remove

    struct renderer_backend_config
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
        //uint32_t                max_texture_count;  // TODO: remove
        //uint32_t                max_mesh_count;     // TODO: remove
        window_backend*         window_ptr;
        memory_pool_manager*    mem_pool_manager_ptr;
        texture_backend*        texture_backend_ptr;
    };

    // struct mesh_config // TODO: remove
    // {
    //     const vertex*           vertices;
    //     const uint16_t*         indices;
    //     uint32_t                vertex_count;
    //     uint32_t                index_count;
    //     vec3                    position;
    //     texture_backend_handle  texture_handle;
    // };

    uint32_t renderer_memory_pool_count();

    void renderer_backend_init(
        renderer_backend& renderer
        );

    bool renderer_backend_configure(
        renderer_backend& renderer,
        const renderer_backend_config& config
        );

    // TODO: for this one we will pass by a texture_manager instead of renderer
    // texture_handle renderer_backend_new_texture(
    //     renderer_backend& renderer,
    //     const texture_config& config
    //     );

    // TODO: for this one we will pass by a mesh_manager instead of renderer
    // mesh_handle renderer_backend_new_mesh(
    //     renderer_backend& renderer,
    //     const mesh_config& config
    //     );
 
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
