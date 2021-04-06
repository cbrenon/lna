#ifndef _LNA_BACKENDS_VULKAN_VULKAN_PRIMITIVE_HPP_
#define _LNA_BACKENDS_VULKAN_VULKAN_PRIMITIVE_HPP_

#include <vulkan/vulkan.h>

namespace lna
{
    struct renderer_backend;
    struct memory_pool;
    struct mat4;

    struct primitive
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
        mat4*                       model_mat_ptr;
        mat4*                       view_mat_ptr;
        mat4*                       projection_mat_ptr;
    };

    struct primitive_backend
    {
        renderer_backend*           renderer_backend_ptr;
        VkDescriptorSetLayout       descriptor_set_layout;
        VkDescriptorPool            descriptor_pool;
        VkPipelineLayout            pipeline_layout;
        VkPipeline                  pipeline;
        primitive*                  primitives;
        uint32_t                    cur_primitive_count;
        uint32_t                    max_primitive_count;
    };
}

#endif // _LNA_BACKENDS_VULKAN_VULKAN_PRIMITIVE_HPP_
