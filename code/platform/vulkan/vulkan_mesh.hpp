#ifndef _LNA_PLATFORM_VULKAN_VULKAN_MESH_HPP_
#define _LNA_PLATFORM_VULKAN_VULKAN_MESH_HPP_

#include <vulkan.h>
#include "graphics/vertex.hpp"

namespace lna
{
    struct vulkan_mesh
    {
        VkBuffer        vertex_buffer;
        VkDeviceMemory  vertex_buffer_memory;
        VkBuffer        index_buffer;
        VkDeviceMemory  index_buffer_memory;
        uint32_t        vertex_count;
        uint32_t        index_count;
    };

    void vulkan_mesh_init(
        vulkan_mesh& mesh
        );

    void vulkan_mesh_create(
        vulkan_mesh& mesh,
        VkDevice device,
        VkPhysicalDevice physical_device,
        const lna::vertex* vertices,
        const uint16_t* indices,
        uint32_t vertex_count,
        uint32_t index_count
        );
}

#endif // _LNA_PLATFORM_VULKAN_VULKAN_MESH_HPP_
