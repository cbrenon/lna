#include <cstring>
#include "core/assert.hpp"
#include "platform/vulkan/vulkan_mesh.hpp"
#include "platform/vulkan/vulkan_helpers.hpp"

void lna::vulkan_mesh_init(
    lna::vulkan_mesh& mesh
    )
{
    mesh.vertex_buffer          = nullptr;
    mesh.vertex_buffer_memory   = nullptr;
    mesh.index_buffer           = nullptr;
    mesh.index_buffer_memory    = nullptr;
    mesh.vertex_count           = 0;
    mesh.index_count            = 0;
}

void lna::vulkan_mesh_create(
    lna::vulkan_mesh& mesh,
    VkDevice device,
    VkPhysicalDevice physical_device,
    const lna::vertex* vertices,
    const uint16_t* indices,
    uint32_t vertex_count,
    uint32_t index_count
    )
{
    LNA_ASSERT(mesh.vertex_buffer == nullptr);
    LNA_ASSERT(mesh.vertex_buffer_memory == nullptr);
    LNA_ASSERT(mesh.index_buffer == nullptr);
    LNA_ASSERT(mesh.index_buffer_memory == nullptr);
    LNA_ASSERT(mesh.vertex_count == 0);
    LNA_ASSERT(mesh.index_count == 0);
    LNA_ASSERT(vertices);
    LNA_ASSERT(indices);
    LNA_ASSERT(vertex_count > 0);
    LNA_ASSERT(index_count > 0);

    mesh.vertex_count  = vertex_count;
    mesh.index_count   = index_count;

    auto vertex_buffer_size = sizeof(vertices[0]) * mesh.vertex_count;
    lna::vulkan_helpers::create_buffer(
        device,
        physical_device,
        vertex_buffer_size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        mesh.vertex_buffer,
        mesh.vertex_buffer_memory
        );
    void* vertices_data;
    vkMapMemory(
        device,
        mesh.vertex_buffer_memory,
        0,
        vertex_buffer_size,
        0,
        &vertices_data
        );
	std::memcpy(
        vertices_data,
        vertices,
        vertex_buffer_size
        );
	vkUnmapMemory(
        device,
        mesh.vertex_buffer_memory
        );

    auto index_buffer_size = sizeof(indices[0]) * mesh.index_count;
    lna::vulkan_helpers::create_buffer(
        device,
        physical_device,
        index_buffer_size,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        mesh.index_buffer,
        mesh.index_buffer_memory
        );
    void* indices_data;
    vkMapMemory(
        device,
        mesh.index_buffer_memory,
        0,
        index_buffer_size,
        0,
        &indices_data
        );
    std::memcpy(
        indices_data,
        indices,
        index_buffer_size
        );
    vkUnmapMemory(
        device,
        mesh.index_buffer_memory
        );
}