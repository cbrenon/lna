#ifndef _LNA_PLATFORM_VULKAN_VULKAN_TEXTURE_HPP_
#define _LNA_PLATFORM_VULKAN_VULKAN_TEXTURE_HPP_

#include <vulkan.h>

namespace lna
{
    struct vulkan_texture
    {
        VkImage         image;
        VkDeviceMemory  image_memory;
        VkImageView     image_view;
        VkSampler       sampler;
    };

    void vulkan_texture_init(
        vulkan_texture& texture
        );

    void vulkan_texture_create(
        vulkan_texture& texture,
        const char* filename,
        VkDevice device,
        VkPhysicalDevice physical_device
        );
}

#endif // _LNA_PLATFORM_VULKAN_VULKAN_TEXTURE_HPP_
