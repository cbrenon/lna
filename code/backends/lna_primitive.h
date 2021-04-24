#ifndef LNA_BACKENDS_LNA_PRIMITIVE_H
#define LNA_BACKENDS_LNA_PRIMITIVE_H

#include <stdint.h>
#include "maths/lna_vec3.h"
#include "maths/lna_vec4.h"

typedef struct lna_primitive_s          lna_primitive_t;
typedef struct lna_primitive_system_s   lna_primitive_system_t;
typedef struct lna_memory_pool_s        lna_memory_pool_t;
typedef struct lna_renderer_s           lna_renderer_t;
typedef struct lna_mat4_s               lna_mat4_t;

typedef struct lna_primitive_system_config_s
{
    uint32_t                            max_primitive_count;
    lna_memory_pool_t*                  memory_pool;
    lna_renderer_t*                     renderer;
} lna_primitive_system_config_t;

typedef struct lna_primitive_vertex_s
{
    lna_vec3_t                          position;
    lna_vec4_t                          color;
} lna_primitive_vertex_t;

typedef struct lna_primitive_line_config_s
{
    const lna_vec3_t*                   pos_a;
    const lna_vec3_t*                   pos_b;
    const lna_vec4_t*                   col_a;
    const lna_vec4_t*                   col_b;
    const lna_mat4_t*                   model_matrix;
    const lna_mat4_t*                   view_matrix;
    const lna_mat4_t*                   projection_matrix;
} lna_primitive_line_config_t;

extern void             lna_primitive_system_init       (lna_primitive_system_t* primitive_system, const lna_primitive_system_config_t* config);
extern lna_primitive_t* lna_primitive_system_new_line   (lna_primitive_system_t* primitive_system, const lna_primitive_line_config_t* config);
extern void             lna_primitive_system_draw       (lna_primitive_system_t* primitive_system);
extern void             lna_primitive_system_release    (lna_primitive_system_t* primitive_system);

#endif
