#ifndef _LNA_PLATFORM_VULKAN_VULKAN_GRAPHICS_OBJECT_HPP_
#define _LNA_PLATFORM_VULKAN_VULKAN_GRAPHICS_OBJECT_HPP_

#include "platform/vulkan/vulkan_mesh.hpp"

namespace lna
{
    struct vulkan_graphics_object
    {
        vulkan_mesh vk_mesh;
        vec3        position;
    };
}

#endif // _LNA_PLATFORM_VULKAN_VULKAN_GRAPHICS_OBJECT_HPP_
