#ifndef _LNA_BACKENDS_MESH_BACKEND_HPP_
#define _LNA_BACKENDS_MESH_BACKEND_HPP_

#include <cstdint>
#include "maths/vec2.hpp"
#include "maths/vec3.hpp"
#include "maths/vec4.hpp"

namespace lna
{
    struct renderer_backend;
    struct mesh_backend;
    struct mesh;
    struct texture;
    struct memory_pool;
    struct mat4;

    struct mesh_backend_config
    {
        uint32_t            max_mesh_count;
        renderer_backend*   renderer_backend_ptr;
        memory_pool*        persistent_memory_pool_ptr;
    };

    struct vertex
    {
        vec3                position;
        vec4                color;
        vec2                uv;
    };

    struct mesh_config
    {
        const vertex*       vertices;
        const uint16_t*     indices;
        uint32_t            vertex_count;
        uint32_t            index_count;
        texture*            texture_ptr;
        mat4*               model_mat_ptr;
        mat4*               view_mat_ptr;
        mat4*               projection_mat_ptr;
    };

    void mesh_backend_configure(
        mesh_backend& backend,
        mesh_backend_config& config
        );

    mesh* mesh_backend_new_mesh(
        mesh_backend& backend,
        mesh_config& config
        );
        
    void mesh_backend_release(
        mesh_backend& backend
        );
}

#endif // _LNA_BACKENDS_MESH_BACKEND_HPP_
