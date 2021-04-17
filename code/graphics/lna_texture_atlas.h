#ifndef LNA_GRAPHICS_LNA_TEXTURE_ATLAS_H
#define LNA_GRAPHICS_LNA_TEXTURE_ATLAS_H

#include <stdint.h>

typedef struct lna_texture_atlas_info_s
{
    //! texture atlas is like a matrix.
    //! fixed sub image size which are oganized in a 2D array where each array cel contains a different image
    uint32_t    col_count;
    uint32_t    row_count;
    uint32_t    width;
    uint32_t    height;
} lna_texture_atlas_info_t;

#endif
