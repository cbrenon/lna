#ifndef _LNA_BACKENDS_VULKAN_VULKAN_MESH_HPP_
#define _LNA_BACKENDS_VULKAN_VULKAN_MESH_HPP_

#include <vulkan/vulkan.h>

namespace lna
{
    class memory_pool;
    struct renderer_backend;
    struct mesh_vertex;
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
        VkBuffer*                   mvp_uniform_buffers;
        VkDeviceMemory*             mvp_uniform_buffers_memory;
        VkBuffer*                   light_uniform_buffers;
        VkDeviceMemory*             light_uniform_buffers_memory;
        VkDescriptorSet*            descriptor_sets;
        uint32_t                    swap_chain_image_count;
        texture*                    texture_ptr;
        mat4*                       model_mat_ptr;
        mat4*                       view_mat_ptr;
        mat4*                       projection_mat_ptr;
    };

    struct mesh_backend
    {
        renderer_backend*           renderer_backend_ptr;
        VkDescriptorSetLayout       descriptor_set_layout;
        VkDescriptorPool            descriptor_pool;
        VkPipelineLayout            pipeline_layout;
        VkPipeline                  pipeline;
        mesh*                       meshes;
        uint32_t                    cur_mesh_count;
        uint32_t                    max_mesh_count;
        VkPolygonMode               polygon_mode;
    };
}

#endif // _LNA_BACKENDS_VULKAN_VULKAN_MESH_HPP_
