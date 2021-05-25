#ifndef LNA_GRAPHICS_LNA_SPRITE_H
#define LNA_GRAPHICS_LNA_SPRITE_H

#include <stdint.h>

typedef struct lna_sprite_s         lna_sprite_t;
typedef struct lna_sprite_system_s  lna_sprite_system_t;
typedef struct lna_memory_pool_s    lna_memory_pool_t;
typedef struct lna_renderer_s       lna_renderer_t;
typedef struct lna_texture_s        lna_texture_t;
typedef union lna_vec2_u            lna_vec2_t;
typedef union lna_vec3_u            lna_vec3_t;
typedef union lna_vec4_u            lna_vec4_t;
typedef struct lna_mat4_s           lna_mat4_t;

typedef struct lna_sprite_system_config_s
{
    uint32_t                max_sprite_count;
    lna_renderer_t*         renderer;
    lna_memory_pool_t*      memory_pool;
} lna_sprite_system_config_t;

typedef struct lna_sprite_config_s
{
    const lna_texture_t*    texture;
    const lna_vec3_t*       local_position;
    const lna_vec2_t*       size;
    const lna_vec2_t*       uv_offset_position;
    const lna_vec2_t*       uv_offset_size;
    const lna_vec4_t*       blend_color;
    const lna_mat4_t*       model_matrix;
    const lna_mat4_t*       view_matrix;
    const lna_mat4_t*       projection_matrix;
} lna_sprite_config_t;

extern void             lna_sprite_system_init          (lna_sprite_system_t* sprite_system, const lna_sprite_system_config_t* config);
extern lna_sprite_t*    lna_sprite_system_new_sprite    (lna_sprite_system_t* sprite_system, const lna_sprite_config_t* config);
extern void             lna_sprite_system_draw          (lna_sprite_system_t* sprite_system);
extern void             lna_sprite_system_release       (lna_sprite_system_t* sprite_system);

#endif
