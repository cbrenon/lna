#ifndef LNA_GRAPHICS_LNA_MESH_H
#define LNA_GRAPHICS_LNA_MESH_H

#include <stdint.h>
#include "maths/lna_vec2.h"
#include "maths/lna_vec3.h"
#include "maths/lna_vec4.h"

typedef struct lna_mesh_system_s    lna_mesh_system_t;
typedef struct lna_mesh_s           lna_mesh_t;
typedef struct lna_memory_pool_s    lna_memory_pool_t;
typedef struct lna_renderer_s       lna_renderer_t;
typedef struct lna_material_s       lna_material_t;
typedef struct lna_mat4_s           lna_mat4_t;

typedef struct lna_mesh_system_config_s
{
    uint32_t                        max_mesh_count;
    lna_renderer_t*                 renderer;
    lna_memory_pool_t*              memory_pool;
} lna_mesh_system_config_t;

typedef struct lna_mesh_vertex_s
{
    lna_vec3_t                      position;
    lna_vec4_t                      color;
    lna_vec2_t                      uv;
    lna_vec3_t                      normal;
} lna_mesh_vertex_t;

typedef struct lna_mesh_config_s
{
    const lna_material_t*           material;
    const lna_mesh_vertex_t*        vertices;
    uint32_t                        vertex_count;
    const uint32_t*                 indices;
    uint32_t                        index_count;
    const lna_mat4_t*               model_matrix;
    const lna_mat4_t*               view_matrix;
    const lna_mat4_t*               projection_matrix;
} lna_mesh_config_t;

extern void         lna_mesh_system_init    (lna_mesh_system_t* mesh_system, const lna_mesh_system_config_t* config);
extern lna_mesh_t*  lna_mesh_system_new_mesh(lna_mesh_system_t* mesh_system, const lna_mesh_config_t* config);
extern void         lna_mesh_system_draw    (lna_mesh_system_t* mesh_system);
extern void         lna_mesh_system_release (lna_mesh_system_t* mesh_system);

#endif
