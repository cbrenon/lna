#ifndef _LNA_BACKENDS_PRIMITIVE_BACKEND_HPP_
#define _LNA_BACKENDS_PRIMITIVE_BACKEND_HPP_

#include <cstdint>
#include "maths/vec3.hpp"
#include "maths/vec4.hpp"

namespace lna
{
    class memory_pool;
    struct backend_renderer;
    struct primitive_backend;
    struct primitive;
    struct mat4;

    struct primitive_backend_config
    {
        uint32_t            max_primitive_count;
        backend_renderer*   renderer_backend_ptr;
        memory_pool*        persistent_memory_pool_ptr;
    };

    struct primitive_vertex
    {
        vec3                position;
        vec4                color;
    };

    struct primitive_config
    {
        //! the only type managed for the moment, except of raw data, are lines
        vec3                pos_a;      // TODO: we will see later if we need to add more primitive types but for the moment we will manage only line type
        vec3                pos_b;      // TODO: we will see later if we need to add more primitive types but for the moment we will manage only line type
        vec4                color;

        //! use to managed raw data
        primitive_vertex*   vertices;
        uint16_t*           indices;
        uint32_t            vertex_count;
        uint32_t            index_count;

        mat4*               model_mat_ptr;
        mat4*               view_mat_ptr;
        mat4*               projection_mat_ptr;
    };

    void primitive_backend_configure(
        primitive_backend& backend,
        primitive_backend_config& config
        );

    primitive* primitive_backend_new_primitive(
        primitive_backend& backend,
        primitive_config& config
        );
        
    void primitive_backend_release(
        primitive_backend& backend
        );
}

#endif // _LNA_BACKENDS_PRIMITIVE_BACKEND_HPP_
