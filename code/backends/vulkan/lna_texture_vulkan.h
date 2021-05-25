#ifndef LNA_BACKENDS_VULKAN_LNA_TEXTURE_VULKAN_H
#define LNA_BACKENDS_VULKAN_LNA_TEXTURE_VULKAN_H

#include <vulkan/vulkan.h>
#include "core/lna_container.h"

typedef struct lna_renderer_s lna_renderer_t;

typedef struct lna_texture_s
{
    VkImage         image;
    VkDeviceMemory  image_memory;
    VkImageView     image_view;
    VkSampler       image_sampler;
    uint32_t        width;
    uint32_t        height;
    uint32_t        atlas_col_count;    //! set to 0 if it is not an atlas texture
    uint32_t        atlas_row_count;    //! set to 0 if it is not an atlas texture
} lna_texture_t;

lna_vector_def(lna_texture_t) lna_texture_vec_t;

typedef struct lna_texture_system_s
{
    lna_texture_vec_t   textures;
    lna_renderer_t*     renderer;
} lna_texture_system_t;

#endif
