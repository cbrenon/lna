#ifndef _LNA_BACKENDS_VULKAN_VULKAN_MESH_HPP_
#define _LNA_BACKENDS_VULKAN_VULKAN_MESH_HPP_

#include <vulkan/vulkan.h>
#include "graphics/vertex.hpp"
#include "maths/mat4.hpp"

namespace lna
{
    struct vulkan_texture;

    struct vulkan_mesh
    {
        VkBuffer                    vertex_buffer;
        VkDeviceMemory              vertex_buffer_memory;
        VkBuffer                    index_buffer;
        VkDeviceMemory              index_buffer_memory;
        uint32_t                    vertex_count;
        uint32_t                    index_count;
        VkBuffer*                   uniform_buffers;
        VkDeviceMemory*             uniform_buffers_memory;
        VkDescriptorSet*            descriptor_sets;
        uint32_t                    swap_chain_image_count;
        vulkan_texture*             texture_ptr;
    };

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

    struct memory_pool;

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
        memory_pool*                swap_chain_memory_pool_ptr;
        memory_pool*                temp_memory_pool_ptr;
    };

    struct vulkan_mesh_update_uniform_buffer_info
    {
        VkDevice                    device;
        uint32_t                    image_index;
        lna::mat4*                  model_matrix_ptr;
        lna::mat4*                  view_matrix_ptr;
        lna::mat4*                  projection_matrix_ptr;
    };

    struct vulkan_mesh_vertex_description
    {
        enum
        {
            MAX_BINDING     = 1,
            MAX_ATTRIBUTES  = 3,
        };

        VkVertexInputBindingDescription     bindings[MAX_BINDING];
        VkVertexInputAttributeDescription   attributes[MAX_ATTRIBUTES];
    };

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

    vulkan_mesh_vertex_description vulkan_default_mesh_vertex_description();
}

#endif // _LNA_BACKENDS_VULKAN_VULKAN_MESH_HPP_
