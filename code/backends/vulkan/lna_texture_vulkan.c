#include "backends/vulkan/lna_texture_vulkan.h"
#include "backends/vulkan/lna_renderer_vulkan.h"
#include "backends/vulkan/lna_vulkan.h"
#include "core/lna_assert.h"
#include "core/lna_memory_pool.h"

// static VkFormat lna_texture_format_to_vulkan(lna_texture_format_t f)
// {
//     switch (f)
//     {
//         case LNA_TEXTURE_FORMAT_R8G8B8A8_SRGB:
//             return VK_FORMAT_R8G8B8A8_SRGB;
//         case LNA_TEXTURE_FORMAT_R8G8B8A8_UNORM:
//             return VK_FORMAT_R8G8B8A8_UNORM;
//     }
// }

void lna_texture_system_init(lna_texture_system_t* texture_system, const lna_texture_system_config_t* config)
{
    lna_assert(texture_system)
    lna_assert(lna_vector_max_capacity(&texture_system->textures) == 0)
    lna_assert(config)
    lna_assert(config->renderer)
    lna_assert(config->memory_pool)
    lna_assert(config->max_texture_count > 0)

    texture_system->renderer = config->renderer;

    lna_vector_init(
        &texture_system->textures,
        config->memory_pool,
        lna_texture_t,
        config->max_texture_count
        );
}

lna_texture_t* lna_texture_system_new_texture(lna_texture_system_t* texture_system)
{
    lna_assert(texture_system)

    lna_texture_t* texture;
    lna_vector_new_element(&texture_system->textures, texture);
    return texture;
}

void lna_texture_system_release(lna_texture_system_t* texture_system)
{
    lna_assert(texture_system)

    // TODO: do we really need this function?
}

void lna_texture_init(lna_texture_t* texture, const lna_texture_config_t* config)
{
    lna_assert(texture)
    lna_assert(config)
}

void lna_texture_release(lna_texture_t* texture)
{
    lna_assert(texture)
}
