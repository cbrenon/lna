#ifndef _LNA_BAKCENDS_VULKAN_VULKAN_TEXTURE_HPP_
#define _LNA_BAKCENDS_VULKAN_VULKAN_TEXTURE_HPP_

#include <vulkan/vulkan.h>
#include "backends/texture_backend.hpp"

namespace lna
{
    struct renderer_backend;

    struct texture
    {
        VkImage                 image;
        VkDeviceMemory          image_memory;
        VkImageView             image_view;
        VkSampler               sampler;
    };

    struct texture_backend
    {
        renderer_backend*       renderer_backend_ptr;
        texture*                textures;
        uint32_t                cur_texture_count;
        uint32_t                max_texture_count;
    };

    // struct vulkan_texture_config_info
    // {
    //     //! choose one of these options:
    //     //! 1: specify filename to load a file
    //     const char*             filename;
    //     //! 2: specify pixels
    //     unsigned char*          pixels;
    //     uint32_t                width;
    //     uint32_t                height;

    //     // VkDevice                device;
    //     // VkPhysicalDevice        physical_device;
    //     // VkCommandPool           command_pool;
    //     // VkQueue                 graphics_queue;
    //     VkFormat                format;
    //     VkFilter                mag_filter;
    //     VkFilter                min_filter;
    //     VkSamplerMipmapMode     mipmap_mode;
    //     VkSamplerAddressMode    address_mode_u;
    //     VkSamplerAddressMode    address_mode_v;
    //     VkSamplerAddressMode    address_mode_w;
    // };

    // void vulkan_texture_configure(
    //     vulkan_texture& texture,
    //     vulkan_texture_config_info& config
    //     );
    
    // void vulkan_texture_release(
    //     vulkan_texture& texture,
    //     VkDevice device
    //     );
}

#endif // _LNA_BAKCENDS_VULKAN_VULKAN_TEXTURE_HPP_
