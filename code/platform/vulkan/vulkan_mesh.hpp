#ifndef _LNA_PLATFORM_VULKAN_VULKAN_MESH_HPP_
#define _LNA_PLATFORM_VULKAN_VULKAN_MESH_HPP_

#include <vulkan.h>
#include "graphics/vertex.hpp"
#include "core/container.hpp"
#include "maths/mat4.hpp"

namespace lna
{
    struct vulkan_mesh
    {
        VkBuffer                    vertex_buffer;
        VkDeviceMemory              vertex_buffer_memory;
        VkBuffer                    index_buffer;
        VkDeviceMemory              index_buffer_memory;
        uint32_t                    vertex_count;
        uint32_t                    index_count;
        heap_array<VkBuffer>        uniform_buffers;
        heap_array<VkDeviceMemory>  uniform_buffers_memory;
        heap_array<VkDescriptorSet> descriptor_sets;
    };

    struct vulkan_uniform_buffer_object
    {
        alignas(16) lna::mat4       model;
        alignas(16) lna::mat4       view;
        alignas(16) lna::mat4       projection;
    };

    struct vulkan_texture;

    struct vulkan_mesh_create_vertex_and_index_info
    {
        VkDevice                    device;
        VkPhysicalDevice            physical_device;
        VkCommandPool               command_pool;
        VkQueue                     graphics_queue;
        const lna::vertex*          vertices;
        const uint16_t*             indices;
        uint32_t                    vertex_count;
        uint32_t                    index_count;
    };

    struct vulkan_mesh_create_uniform_buffer_info
    {
        VkDevice                    device;
        VkPhysicalDevice            physical_device;
        uint32_t                    swap_chain_image_count;
        memory_pool*                swap_chain_memory_pool_ptr;
    };

    struct vulkan_mesh_create_descriptor_sets_info
    {
        VkDevice                    device;
        VkPhysicalDevice            physical_device;
        VkDescriptorPool            descriptor_pool;
        VkDescriptorSetLayout       descriptor_set_layout;
        uint32_t                    swap_chain_image_count;
        memory_pool*                swap_chain_memory_pool_ptr;
        memory_pool*                temp_memory_pool_ptr;
        vulkan_texture*             texture_ptr;
    };

    struct vulkan_mesh_update_uniform_buffer_info
    {
        VkDevice                    device;
        uint32_t                    image_index;
        VkExtent2D                  swap_chain_extent; // TODO: to remove 

        // TODO: add model matrix pointer
        // TODO: add view matrix pointer
        // TODO: add projection matrix pointer
    };

    void vulkan_mesh_init(
        vulkan_mesh& mesh
        );

    void vulkan_mesh_create_vertex_and_index_buffer(
        vulkan_mesh& mesh,
        vulkan_mesh_create_vertex_and_index_info& config
        );

    void vulkan_mesh_create_uniform_buffer(
        vulkan_mesh& mesh,
        vulkan_mesh_create_uniform_buffer_info& config
        );

    void vulkan_mesh_upate_uniform_buffer(
        vulkan_mesh& mesh,
        vulkan_mesh_update_uniform_buffer_info& config
        );

    void vulkan_mesh_create_descriptor_sets(
        vulkan_mesh& mesh,
        vulkan_mesh_create_descriptor_sets_info& config
        );

    void vulkan_mesh_clean_uniform_buffer(
        vulkan_mesh& mesh,
        VkDevice device
        );

    void vulkan_mesh_clean_descriptor_sets(
        vulkan_mesh& mesh
        );

    void vulkan_mesh_release(
        vulkan_mesh& mesh,
        VkDevice device
        );
}

#endif // _LNA_PLATFORM_VULKAN_VULKAN_MESH_HPP_
