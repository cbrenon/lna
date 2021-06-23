#include <string.h>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#pragma warning(push, 0)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#pragma warning(pop)

#include "graphics/lna_texture.h"
#include "backends/vulkan/lna_texture_vulkan.h"
#include "backends/vulkan/lna_renderer_vulkan.h"
#include "backends/vulkan/lna_vulkan.h"
#include "core/lna_assert.h"
#include "core/lna_memory_pool.h"

static VkFormat lna_texture_format_to_vulkan(lna_texture_format_t f)
{
    switch (f)
    {
        case LNA_TEXTURE_FORMAT_R8G8B8A8_SRGB:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case LNA_TEXTURE_FORMAT_R8G8B8A8_UNORM:
            return VK_FORMAT_R8G8B8A8_UNORM;
    }
}

static VkSamplerMipmapMode lna_texture_mimap_mode_to_vulkan(lna_texture_mipmap_mode_t m)
{
    switch (m)
    {
        case LNA_TEXTURE_MIPMAP_MODE_LINEAR:
            return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        case LNA_TEXTURE_MIPMAP_MODE_NEAREST:
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    }
}

static VkFilter lna_texture_filter_to_vulkan(lna_texture_filter_t f)
{
    switch (f)
    {
        case LNA_TEXTURE_FILTER_LINEAR:
            return VK_FILTER_LINEAR;
        case LNA_TEXTURE_FILTER_NEAREST:
            return VK_FILTER_NEAREST;
    }
}

static VkSamplerAddressMode lna_texture_sampler_address_mode_to_vulkan(lna_texture_sampler_address_mode_t a)
{
    switch (a)
    {
        case LNA_TEXTURE_SAMPLER_ADDRESS_MODE_REPEAT:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case LNA_TEXTURE_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        case LNA_TEXTURE_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    }
}

static void lna_texture_init(lna_texture_t* texture, const lna_texture_config_t* config, lna_renderer_t* renderer)
{
    lna_assert(texture)
    lna_assert(texture->image == VK_NULL_HANDLE)
    lna_assert(texture->image_memory == VK_NULL_HANDLE)
    lna_assert(texture->image_view == VK_NULL_HANDLE)
    lna_assert(texture->image_sampler == VK_NULL_HANDLE)
    lna_assert(config)
    lna_assert(renderer)
    lna_assert(renderer->device)
    lna_assert(renderer->physical_device)

    int             texture_width       = 0;
    int             texture_height      = 0;
    int             texture_channels    = 0;
    unsigned char*  texture_pixels      = NULL;
    VkDeviceSize    texture_size        = 0;

    //! IMAGE PART

    if (config->filename)
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
        texture_pixels = stbi_load(
            config->filename,
            &texture_width,
            &texture_height,
            &texture_channels,
            STBI_rgb_alpha
            );
#pragma clang diagnostic pop
    }
    texture_size = texture_width * texture_height * 4 * sizeof(char);

    lna_assert(texture_pixels)
    lna_assert(texture_size > 0)

    VkBuffer        staging_buffer;
    VkDeviceMemory  staging_buffer_memory;

    lna_vulkan_create_buffer(
        renderer->device,
        renderer->physical_device,
        texture_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &staging_buffer,
        &staging_buffer_memory
        );

    void* data;
    lna_vulkan_check(
        vkMapMemory(
            renderer->device,
            staging_buffer_memory,
            0,
            texture_size,
            0,
            &data
            )
        );
    memcpy(
        data,
        texture_pixels,
        (size_t)texture_size
        );
    vkUnmapMemory(
        renderer->device,
        staging_buffer_memory
        );

    if (config->filename)
    {
        stbi_image_free(texture_pixels);
    }

    lna_vulkan_create_image(
        renderer->device,
        renderer->physical_device,
        (uint32_t)texture_width,
        (uint32_t)texture_height,
        lna_texture_format_to_vulkan(config->format),
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &texture->image,
        &texture->image_memory
        );
    lna_vulkan_transition_image_layout(
        renderer->device,
        renderer->command_pool,
        renderer->graphics_queue,
        texture->image,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        );
    lna_vulkan_copy_buffer_to_image(
        renderer->device,
        renderer->command_pool,
        staging_buffer,
        renderer->graphics_queue,
        texture->image,
        (uint32_t)texture_width,
        (uint32_t)texture_height
        );
    lna_vulkan_transition_image_layout(
        renderer->device,
        renderer->command_pool,
        renderer->graphics_queue,
        texture->image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );
    vkDestroyBuffer(
        renderer->device,
        staging_buffer,
        NULL
        );
    vkFreeMemory(
        renderer->device,
        staging_buffer_memory,
        NULL
        );

    //! IMAGE VIEW PART

    texture->image_view = lna_vulkan_create_image_view(
        renderer->device,
        texture->image,
        lna_texture_format_to_vulkan(config->format),
        VK_IMAGE_ASPECT_COLOR_BIT
        );

    //! SAMPLER PART

    VkPhysicalDeviceProperties gpu_properties = { 0 };
    vkGetPhysicalDeviceProperties(
        renderer->physical_device,
        &gpu_properties
        );

    const VkSamplerCreateInfo sampler_create_info =
    {
        .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter               = lna_texture_filter_to_vulkan(config->mag),
        .minFilter               = lna_texture_filter_to_vulkan(config->min),
        .addressModeU            = lna_texture_sampler_address_mode_to_vulkan(config->u),
        .addressModeV            = lna_texture_sampler_address_mode_to_vulkan(config->v),
        .addressModeW            = lna_texture_sampler_address_mode_to_vulkan(config->w),
        .anisotropyEnable        = VK_TRUE,
        .maxAnisotropy           = gpu_properties.limits.maxSamplerAnisotropy,
        .borderColor             = VK_BORDER_COLOR_INT_OPAQUE_WHITE,
        .unnormalizedCoordinates = VK_FALSE,
        .compareEnable           = VK_FALSE,
        .compareOp               = VK_COMPARE_OP_ALWAYS,
        .mipmapMode              = lna_texture_mimap_mode_to_vulkan(config->mimap_mode),
        .mipLodBias              = 0.0f,
        .minLod                  = 0.0f,
        .maxLod                  = 0.0f,
    };

    lna_vulkan_check(
        vkCreateSampler(
            renderer->device,
            &sampler_create_info,
            NULL,
            &texture->image_sampler
            )
        );

    texture->width              = (uint32_t)texture_width;
    texture->height             = (uint32_t)texture_height;
    texture->atlas_col_count    = config->atlas_col_count;
    texture->atlas_row_count    = config->atlas_row_count;
}

static void lna_texture_release(lna_texture_t* texture, VkDevice device)
{
    lna_assert(texture)
    lna_assert(texture->image_sampler)
    lna_assert(texture->image_view)
    lna_assert(texture->image)
    lna_assert(texture->image_memory)
    lna_assert(device)

    vkDestroySampler(
        device,
        texture->image_sampler,
        NULL
        );
    vkDestroyImageView(
        device,
        texture->image_view,
        NULL
        );
    vkDestroyImage(
        device,
        texture->image,
        NULL
        );
    vkFreeMemory(
        device,
        texture->image_memory,
        NULL
        );

    texture->image_sampler  = VK_NULL_HANDLE;
    texture->image_view     = VK_NULL_HANDLE;
    texture->image          = VK_NULL_HANDLE;
    texture->image_memory   = VK_NULL_HANDLE;
}

void lna_texture_system_init(lna_texture_system_t* texture_system, const lna_texture_system_config_t* config)
{
    lna_assert(texture_system)
    lna_assert(texture_system->textures.cur_element_count == 0)
    lna_assert(texture_system->textures.max_element_count == 0)
    lna_assert(texture_system->textures.elements == NULL)
    lna_assert(config)
    lna_assert(config->renderer)
    lna_assert(config->memory_pool)
    lna_assert(config->max_texture_count > 0)

    texture_system->renderer                    = config->renderer;
    texture_system->textures.cur_element_count  = 0;
    texture_system->textures.max_element_count  = config->max_texture_count;
    texture_system->textures.elements           = lna_memory_pool_reserve(
        config->memory_pool,
        config->max_texture_count * sizeof(lna_texture_t)
        );
}

lna_texture_t* lna_texture_system_new_texture(lna_texture_system_t* texture_system, const lna_texture_config_t* config)
{
    lna_assert(texture_system)

    lna_texture_t* texture = &texture_system->textures.elements[texture_system->textures.cur_element_count++];

    lna_texture_init(
        texture,
        config,
        texture_system->renderer
        );

    return texture;
}

void lna_texture_system_release(lna_texture_system_t* texture_system)
{
    lna_assert(texture_system)
    lna_assert(texture_system->renderer)

    for (uint32_t index = 0; index < texture_system->textures.cur_element_count; ++index)
    {
        lna_texture_release(
            &texture_system->textures.elements[index],
            texture_system->renderer->device
            );
    }
    texture_system->textures.cur_element_count  = 0;
    texture_system->textures.max_element_count  = 0;
    texture_system->textures.elements           = NULL;
    texture_system->renderer = NULL;
}

uint32_t lna_texture_width(lna_texture_t* texture)
{
    lna_assert(texture)
    lna_assert(texture->image)
    return texture->width;    
}

uint32_t lna_texture_height(lna_texture_t* texture)
{
    lna_assert(texture)
    lna_assert(texture->image)
    return texture->height;
}

uint32_t lna_texture_atlas_col_count(lna_texture_t* texture)
{
    lna_assert(texture)
    lna_assert(texture->image)
    return texture->atlas_col_count;
}

uint32_t lna_texture_atlas_row_count(lna_texture_t* texture)
{
    lna_assert(texture)
    lna_assert(texture->image)
    return texture->atlas_row_count;
}

