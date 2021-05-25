#ifndef LNA_GRAPHICS_LNA_MESH_H
#define LNA_GRAPHICS_LNA_MESH_H

#include <stdint.h>
#include "maths/lna_vec3.h"

typedef struct lna_mesh_system_s    lna_mesh_system_t;
typedef struct lna_mesh_s           lna_mesh_t;
typedef struct lna_memory_pool_s    lna_memory_pool_t;
typedef struct lna_renderer_s       lna_renderer_t;
typedef struct lna_texture_s        lna_texture_t;

typedef struct lna_mesh_system_config_s
{
    uint32_t                        max_mesh_count;
    lna_renderer_t*                 renderer;
    lna_memory_pool_t*              memory_pool;
} lna_mesh_system_config_t;

typedef struct lna_mesh_material_s
{
    lna_texture_t*                  texture;
} lna_mesh_material_t;

extern void         lna_mesh_system_init    (lna_mesh_system_t* mesh_system, const lna_mesh_system_config_t* config);
extern lna_mesh_t*  lna_mesh_system_new_mesh(lna_mesh_system_t* mesh_system);
extern void         lna_mesh_system_draw    (lna_mesh_system_t* mesh_system);
extern void         lna_mesh_system_release (lna_mesh_system_t* mesh_system);

#endif
