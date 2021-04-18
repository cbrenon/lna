#ifndef LNA_BACKENDS_VULKAN_LNA_TEXTURE_VULKAN_H
#define LNA_BACKENDS_VULKAN_LNA_TEXTURE_VULKAN_H

#include <vulkan/vulkan.h>
#include "backends/lna_texture.h"
#include "core/lna_container.h"

typedef struct lna_texture_s
{
    VkImage         image;
    VkDeviceMemory  image_memory;
    VkImageView     image_view;
    VkSampler       image_sampler;
} lna_texture_t;

lna_vector_def(lna_texture_t) lna_texture_vec_t;

typedef struct lna_texture_system_s
{
    lna_texture_vec_t   textures;
    lna_renderer_t*     renderer;
} lna_texture_system_t;

#endif
