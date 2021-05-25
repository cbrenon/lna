#ifndef LNA_GRAPHICS_LNA_MATERIAL_H
#define LNA_GRAPHICS_LNA_MATERIAL_H

typedef struct lna_texture_s lna_texture_t;

typedef struct lna_material_s
{
    const lna_texture_t* texture;
} lna_material_t;

#endif
