#include <cstring>
#include "platform/vulkan/vulkan_texture.hpp"
#include "platform/vulkan/vulkan_helpers.hpp"
#include "graphics/texture.hpp"

void lna::vulkan_texture_init(
    lna::vulkan_texture& vk_texture
    )
{
    vk_texture.image        = nullptr;
    vk_texture.image_memory = nullptr;
    vk_texture.image_view   = nullptr;
    vk_texture.sampler      = nullptr;
}

void lna::vulkan_texture_configure(
    lna::vulkan_texture& vk_texture,
    vulkan_texture_config_info& config
    )
{
    LNA_ASSERT(config.command_pool);
    LNA_ASSERT(config.device);
    LNA_ASSERT(config.graphics_queue);
    LNA_ASSERT(config.physical_device);

    //! IMAGE PART

    lna::texture tex;
    lna::texture_init(tex);

    if (config.filename)
    {
        lna::texture_load_from_file(
            tex,
            config.filename
            );
    }
    else
    {
        // for the moment we only create texture with a filename
        LNA_ASSERT(0);
    }

    VkBuffer        staging_buffer;
    VkDeviceMemory  staging_buffer_memory;
    VkDeviceSize    image_size = tex.width * tex.height * 4;

    lna::vulkan_helpers::create_buffer(
        config.device,
        config.physical_device,
        image_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        staging_buffer,
        staging_buffer_memory
        );

    void* data;
    VULKAN_CHECK_RESULT(
        vkMapMemory(
            config.device,
            staging_buffer_memory,
            0,
            image_size,
            0,
            &data
            )
        )
    std::memcpy(
        data,
        tex.pixels,
        static_cast<size_t>(image_size)
        );
    vkUnmapMemory(
        config.device,
        staging_buffer_memory
        );

    lna::texture_free_pixels(
        tex
        );

    lna::vulkan_helpers::create_image(
        config.device,
        config.physical_device,
        static_cast<uint32_t>(tex.width),
        static_cast<uint32_t>(tex.height),
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        vk_texture.image,
        vk_texture.image_memory
        );
    lna::vulkan_helpers::transition_image_layout(
        config.device,
        config.command_pool,
        config.graphics_queue,
        vk_texture.image,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        );
    lna::vulkan_helpers::copy_buffer_to_image(
        config.device,
        config.command_pool,
        staging_buffer,
        config.graphics_queue,
        vk_texture.image,
        static_cast<uint32_t>(tex.width),
        static_cast<uint32_t>(tex.height)
        );
    lna::vulkan_helpers::transition_image_layout(
        config.device,
        config.command_pool,
        config.graphics_queue,
        vk_texture.image,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );
    vkDestroyBuffer(
        config.device,
        staging_buffer,
        nullptr
        );
    vkFreeMemory(
        config.device,
        staging_buffer_memory,
        nullptr
        );

    //! IMAGE VIEW PART

    vk_texture.image_view = lna::vulkan_helpers::create_image_view(
        config.device,
        vk_texture.image,
        VK_FORMAT_R8G8B8A8_SRGB
        );

    //! SAMPLER PART

    VkPhysicalDeviceProperties gpu_properties{};
    vkGetPhysicalDeviceProperties(
        config.physical_device,
        &gpu_properties
        );

    VkSamplerCreateInfo sampler_create_info{};
    sampler_create_info.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_create_info.magFilter               = VK_FILTER_LINEAR;
    sampler_create_info.minFilter               = VK_FILTER_LINEAR;
    sampler_create_info.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.anisotropyEnable        = VK_TRUE;
    sampler_create_info.maxAnisotropy           = gpu_properties.limits.maxSamplerAnisotropy;
    sampler_create_info.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_create_info.unnormalizedCoordinates = VK_FALSE;
    sampler_create_info.compareEnable           = VK_FALSE;
    sampler_create_info.compareOp               = VK_COMPARE_OP_ALWAYS;
    sampler_create_info.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_create_info.mipLodBias              = 0.0f;
    sampler_create_info.minLod                  = 0.0f;
    sampler_create_info.maxLod                  = 0.0f;

    VULKAN_CHECK_RESULT(
        vkCreateSampler(
            config.device,
            &sampler_create_info,
            nullptr,
            &vk_texture.sampler
            )
        )
}

void lna::vulkan_texture_release(
    lna::vulkan_texture& vk_texture,
    VkDevice device
    )
{
    vkDestroySampler(
        device,
        vk_texture.sampler,
        nullptr
        );
    vkDestroyImageView(
        device,
        vk_texture.image_view,
        nullptr
        );
    vkDestroyImage(
        device,
        vk_texture.image,
        nullptr
        );
    vkFreeMemory(
        device,
        vk_texture.image_memory,
        nullptr
        );
}
