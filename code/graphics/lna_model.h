#ifndef LNA_GRAPHICS_LNA_MODEL_H
#define LNA_GRAPHICS_LNA_MODEL_H

#include <stdint.h>
#include "maths/lna_vec2.h"
#include "maths/lna_vec3.h"
#include "maths/lna_vec4.h"

typedef struct lna_memory_pool_s lna_memory_pool_t;

typedef struct lna_model_vertex_s
{
    lna_vec3_t                      position;
    lna_vec4_t                      color;
    lna_vec2_t                      uv;
    lna_vec3_t                      normal;
} lna_model_vertex_t;

typedef struct lna_model_vertex_array_s
{
    lna_model_vertex_t*             data;
    uint32_t                        count;
} lna_model_vertex_array_t;

typedef struct lna_model_index_array_s
{
    uint32_t*                       data;
    uint32_t                        count;
} lna_model_index_array_t;

typedef struct lna_model_s
{
    lna_model_vertex_array_t        vertices;
    lna_model_index_array_t         indices;
} lna_model_t;

typedef struct lna_model_config_s
{
    const char*                     filename;
    lna_memory_pool_t*              temp_lifetime_mem_pool;     //! for temporary object: can be a frame lifetime memory pool
    lna_memory_pool_t*              object_lifetime_mem_pool;   //! for object lifetime: memory pool must have the same lifetime than the object whom will used the initialized model
} lna_model_config_t;

//! -----------------------------------------------------------------------------
//! BLENDER EXPORT PARAM:
//! -----------------------------------------------------------------------------
//! [X] apply modifiers
//! [X] include edges
//! [X] write normals
//! [X] include UVs
//! [X] write materials
//! [X] triangulate faces
//! [X] objects as OBJ objects
//! About smooth/cel shading: To have shared smooth normals I must active:
//! OBJECT MODE => OBJECT => SHADE SMOOTH
//! -----------------------------------------------------------------------------
extern void lna_model_init_dev_mode(lna_model_t* model, const lna_model_config_t* config);

#endif
