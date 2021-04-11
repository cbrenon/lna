#ifndef _LNA_BACKENDS_VULKAN_VULKAN_MESH_HPP_
#define _LNA_BACKENDS_VULKAN_VULKAN_MESH_HPP_

#include <vulkan/vulkan.h>
#include "core/heap_array.hpp"
#include "core/fixed_size_vector.hpp"

namespace lna
{
    struct backend_renderer;
    struct texture;
    struct mat4;

    struct mesh
    {
        VkBuffer                    vertex_buffer;
        VkDeviceMemory              vertex_buffer_memory;
        VkBuffer                    index_buffer;
        VkDeviceMemory              index_buffer_memory;
        uint32_t                    vertex_count;
        uint32_t                    index_count;
        heap_array<VkBuffer>        mvp_uniform_buffers;
        heap_array<VkDeviceMemory>  mvp_uniform_buffers_memory;
        heap_array<VkBuffer>        light_uniform_buffers;
        heap_array<VkDeviceMemory>  light_uniform_buffers_memory;
        heap_array<VkDescriptorSet> descriptor_sets;
        texture*                    texture_ptr;
        mat4*                       model_mat_ptr;
        mat4*                       view_mat_ptr;
        mat4*                       projection_mat_ptr;
    };

    struct mesh_backend
    {
        backend_renderer*           renderer_backend_ptr;
        VkDescriptorSetLayout       descriptor_set_layout;
        VkDescriptorPool            descriptor_pool;
        VkPipelineLayout            pipeline_layout;
        VkPipeline                  pipeline;
        fixed_size_vector<mesh>     meshes;
        VkPolygonMode               polygon_mode;
    };
}

#endif // _LNA_BACKENDS_VULKAN_VULKAN_MESH_HPP_
