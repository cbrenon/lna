#ifndef _LNA_BAKCENDS_VULKAN_VULKAN_TEXTURE_HPP_
#define _LNA_BAKCENDS_VULKAN_VULKAN_TEXTURE_HPP_

#include <vulkan/vulkan.h>
#include "backends/texture_backend.hpp"

namespace lna
{
    struct backend_renderer;

    struct texture
    {
        VkImage                 image;
        VkDeviceMemory          image_memory;
        VkImageView             image_view;
        VkSampler               sampler;
    };

    struct texture_backend
    {
        backend_renderer*       renderer_backend_ptr;
        texture*                textures;
        uint32_t                cur_texture_count;
        uint32_t                max_texture_count;
    };
}

#endif // _LNA_BAKCENDS_VULKAN_VULKAN_TEXTURE_HPP_
