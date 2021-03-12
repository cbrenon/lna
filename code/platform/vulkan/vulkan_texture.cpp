#include "platform/vulkan/vulkan_texture.hpp"

void lna::vulkan_texture_init(
    lna::vulkan_texture& texture
    )
{
    texture.image           = nullptr;
    texture.image_memory    = nullptr;
    texture.image_view      = nullptr;
    texture.sampler         = nullptr;
}
