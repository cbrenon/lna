#ifndef _LNA_PLATFORM_VULKAN_VULKAN_TEXTURE_HPP_
#define _LNA_PLATFORM_VULKAN_VULKAN_TEXTURE_HPP_

#include <vulkan.h>

namespace lna
{
    struct vulkan_texture
    {
        VkImage             image;
        VkDeviceMemory      image_memory;
        VkImageView         image_view;
        VkSampler           sampler;
    };

    struct vulkan_texture_config_info
    {
        const char*         filename;
        VkDevice            device;
        VkPhysicalDevice    physical_device;
        VkCommandPool       command_pool;
        VkQueue             graphics_queue;
    };

    void vulkan_texture_init(
        vulkan_texture& vk_texture
        );

    void vulkan_texture_configure(
        vulkan_texture& vk_texture,
        vulkan_texture_config_info& config
        );
    
    void vulkan_texture_release(
        vulkan_texture& vk_texture,
        VkDevice device
        );
}

#endif // _LNA_PLATFORM_VULKAN_VULKAN_TEXTURE_HPP_
