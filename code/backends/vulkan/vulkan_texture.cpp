#include <cstring>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-compare"
#pragma warning(push, 0)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#pragma warning(pop)
#pragma clang diagnostic pop
#include "backends/vulkan/vulkan_texture.hpp"
#include "backends/vulkan/vulkan_helpers.hpp"
#include "backends/vulkan/vulkan_backend.hpp"
#include "backends/texture_backend.hpp"
#include "core/memory_pool.hpp"

namespace
{
    VkFormat lna_format_to_vulkan(lna::texture_config::format f)
    {
        switch (f)
        {
            case lna::texture_config::format::R8G8B8A8_SRGB:
                return VK_FORMAT_R8G8B8A8_SRGB;
            case lna::texture_config::format::R8G8B8A8_UNORM:
                return VK_FORMAT_R8G8B8A8_UNORM;
            default:
                return VK_FORMAT_UNDEFINED;
        }
    }

    VkSamplerMipmapMode lna_mipmap_mode_to_vulkan(lna::texture_config::mipmap_mode mipmap)
    {
        switch (mipmap)
        {
            case lna::texture_config::mipmap_mode::LINEAR:
                return VK_SAMPLER_MIPMAP_MODE_LINEAR;
            default:
                return VK_SAMPLER_MIPMAP_MODE_MAX_ENUM;
        }
    }

    VkFilter lna_filter_to_vulkan_filter(lna::texture_config::filter f)
    {
        switch (f)
        {
            case lna::texture_config::filter::LINEAR:
                return VK_FILTER_LINEAR;
            default:
                return VK_FILTER_MAX_ENUM;
        }
    }

    VkSamplerAddressMode lna_sampler_address_mode_to_vulkan(lna::texture_config::sampler_address_mode m)
    {
        switch (m)
        {
            case lna::texture_config::sampler_address_mode::REPEAT:
                return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            case lna::texture_config::sampler_address_mode::CLAMP_TO_EDGE:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            default:
                return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
        }
    }
}

namespace lna
{
    void texture_backend_configure(
        texture_backend& backend,
        texture_backend_config& config
        )
    {
        LNA_ASSERT(backend.renderer_backend_ptr == nullptr);
        LNA_ASSERT(backend.textures == nullptr);
        LNA_ASSERT(backend.cur_texture_count == 0);
        LNA_ASSERT(backend.max_texture_count == 0);
        LNA_ASSERT(config.renderer_backend_ptr);
        LNA_ASSERT(config.persistent_memory_pool_ptr);
        LNA_ASSERT(config.max_texture_count > 0);

        backend.renderer_backend_ptr    = config.renderer_backend_ptr;
        backend.max_texture_count       = config.max_texture_count;
        backend.textures                = (texture*)memory_pool_reserve_memory(
            *config.persistent_memory_pool_ptr,
            config.max_texture_count * sizeof(texture)
            );
        LNA_ASSERT(backend.textures);

        for (uint32_t i = 0; i < backend.max_texture_count; ++i)
        {
            backend.textures[i].image           = VK_NULL_HANDLE;
            backend.textures[i].image_memory    = VK_NULL_HANDLE;
            backend.textures[i].image_view      = VK_NULL_HANDLE;
            backend.textures[i].sampler         = VK_NULL_HANDLE;
        }
    }

    texture* texture_backend_new_texture(
        texture_backend& backend,
        texture_config& config
        )
    {
        LNA_ASSERT(backend.renderer_backend_ptr);
        LNA_ASSERT(backend.renderer_backend_ptr->command_pool);
        LNA_ASSERT(backend.renderer_backend_ptr->device);
        LNA_ASSERT(backend.renderer_backend_ptr->graphics_queue);
        LNA_ASSERT(backend.renderer_backend_ptr->physical_device);
        LNA_ASSERT(backend.textures);
        LNA_ASSERT(backend.cur_texture_count < backend.max_texture_count);
        LNA_ASSERT((config.filename || config.pixels) && (config.filename == nullptr || config.pixels == nullptr));

        texture& new_texture = backend.textures[backend.cur_texture_count++];

        LNA_ASSERT(new_texture.image == VK_NULL_HANDLE);
        LNA_ASSERT(new_texture.image_memory == VK_NULL_HANDLE);
        LNA_ASSERT(new_texture.image_view == VK_NULL_HANDLE);
        LNA_ASSERT(new_texture.sampler == VK_NULL_HANDLE);

        //! IMAGE PART

        int             texture_width       = 0;
        int             texture_height      = 0;
        int             texture_channels    = 0;
        unsigned char*  texture_pixels      = nullptr;
        VkDeviceSize    image_size          = 0;

        if (config.filename)
        {
            texture_pixels = stbi_load(
                config.filename,
                &texture_width,
                &texture_height,
                &texture_channels,
                STBI_rgb_alpha
                );
        }
        else if (config.pixels)
        {
            texture_pixels  = config.pixels;
            texture_width   = config.width;
            texture_height  = config.height;
        }
        image_size = texture_width * texture_height * 4 * sizeof(char);
    
        LNA_ASSERT(texture_pixels);
        LNA_ASSERT(image_size > 0);

        VkBuffer        staging_buffer;
        VkDeviceMemory  staging_buffer_memory;

        lna::vulkan_helpers::create_buffer(
            backend.renderer_backend_ptr->device,
            backend.renderer_backend_ptr->physical_device,
            image_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            staging_buffer,
            staging_buffer_memory
            );

        void* data;
        VULKAN_CHECK_RESULT(
            vkMapMemory(
                backend.renderer_backend_ptr->device,
                staging_buffer_memory,
                0,
                image_size,
                0,
                &data
                )
            )
        std::memcpy(
            data,
            texture_pixels,
            static_cast<size_t>(image_size)
            );
        vkUnmapMemory(
            backend.renderer_backend_ptr->device,
            staging_buffer_memory
            );

        if (config.filename)
        {
            stbi_image_free(texture_pixels);
        }

        lna::vulkan_helpers::create_image(
            backend.renderer_backend_ptr->device,
            backend.renderer_backend_ptr->physical_device,
            static_cast<uint32_t>(texture_width),
            static_cast<uint32_t>(texture_height),
            lna_format_to_vulkan(config.fmt),
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            new_texture.image,
            new_texture.image_memory
            );
        lna::vulkan_helpers::transition_image_layout(
            backend.renderer_backend_ptr->device,
            backend.renderer_backend_ptr->command_pool,
            backend.renderer_backend_ptr->graphics_queue,
            new_texture.image,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
            );
        lna::vulkan_helpers::copy_buffer_to_image(
            backend.renderer_backend_ptr->device,
            backend.renderer_backend_ptr->command_pool,
            staging_buffer,
            backend.renderer_backend_ptr->graphics_queue,
            new_texture.image,
            static_cast<uint32_t>(texture_width),
            static_cast<uint32_t>(texture_height)
            );
        lna::vulkan_helpers::transition_image_layout(
            backend.renderer_backend_ptr->device,
            backend.renderer_backend_ptr->command_pool,
            backend.renderer_backend_ptr->graphics_queue,
            new_texture.image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            );
        vkDestroyBuffer(
            backend.renderer_backend_ptr->device,
            staging_buffer,
            nullptr
            );
        vkFreeMemory(
            backend.renderer_backend_ptr->device,
            staging_buffer_memory,
            nullptr
            );

        //! IMAGE VIEW PART

        new_texture.image_view = lna::vulkan_helpers::create_image_view(
            backend.renderer_backend_ptr->device,
            new_texture.image,
            lna_format_to_vulkan(config.fmt),
            VK_IMAGE_ASPECT_COLOR_BIT
            );

        //! SAMPLER PART

        VkPhysicalDeviceProperties gpu_properties{};
        vkGetPhysicalDeviceProperties(
            backend.renderer_backend_ptr->physical_device,
            &gpu_properties
            );

        VkSamplerCreateInfo sampler_create_info{};
        sampler_create_info.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_create_info.magFilter               = lna_filter_to_vulkan_filter(config.mag);
        sampler_create_info.minFilter               = lna_filter_to_vulkan_filter(config.min);
        sampler_create_info.addressModeU            = lna_sampler_address_mode_to_vulkan(config.u);
        sampler_create_info.addressModeV            = lna_sampler_address_mode_to_vulkan(config.v);
        sampler_create_info.addressModeW            = lna_sampler_address_mode_to_vulkan(config.w);
        sampler_create_info.anisotropyEnable        = VK_TRUE;
        sampler_create_info.maxAnisotropy           = gpu_properties.limits.maxSamplerAnisotropy;
        sampler_create_info.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
        sampler_create_info.unnormalizedCoordinates = VK_FALSE;
        sampler_create_info.compareEnable           = VK_FALSE;
        sampler_create_info.compareOp               = VK_COMPARE_OP_ALWAYS;
        sampler_create_info.mipmapMode              = lna_mipmap_mode_to_vulkan(config.mipmap);
        sampler_create_info.mipLodBias              = 0.0f;
        sampler_create_info.minLod                  = 0.0f;
        sampler_create_info.maxLod                  = 0.0f;
        VULKAN_CHECK_RESULT(
            vkCreateSampler(
                backend.renderer_backend_ptr->device,
                &sampler_create_info,
                nullptr,
                &new_texture.sampler
                )
            )

        return &new_texture;
    }

    void texture_backend_release(
        texture_backend& backend
        )
    {
        if (backend.textures)
        {
            for (uint32_t i = 0; i < backend.max_texture_count; ++i)
            {
                texture&    texture_to_del = backend.textures[i];
                VkDevice    device  = backend.renderer_backend_ptr->device;

                LNA_ASSERT(device);

                vkDestroySampler(
                    device,
                    texture_to_del.sampler,
                    nullptr
                    );
                vkDestroyImageView(
                    device,
                    texture_to_del.image_view,
                    nullptr
                    );
                vkDestroyImage(
                    device,
                    texture_to_del.image,
                    nullptr
                    );
                vkFreeMemory(
                    device,
                    texture_to_del.image_memory,
                    nullptr
                    );
                texture_to_del.image           = nullptr;
                texture_to_del.image_memory    = nullptr;
                texture_to_del.image_view      = nullptr;
                texture_to_del.sampler         = nullptr;
            }
        }
    }
}
