#ifndef _LNA_BACKENDS_VULKAN_VULKAN_IMGUI_HPP_
#define _LNA_BACKENDS_VULKAN_VULKAN_IMGUI_HPP_

#include <vulkan/vulkan.h>
#include "maths/vec2.hpp"

namespace lna
{
    struct memory_pool;
    struct texture_backend;
    struct texture;

    struct vulkan_imgui_push_const_block
    {
		vec2                            scale;
		vec2                            translate;
	};

    struct imgui_backend
    {
        renderer_backend*               renderer_backend_ptr;
        VkBuffer                        vertex_buffer;
        VkDeviceMemory                  vertex_buffer_memory;
        VkBuffer                        index_buffer;
        VkDeviceMemory                  index_buffer_memory;
        texture*                        font_texture_ptr;
        VkPipelineCache                 pipeline_cache;
        VkPipelineLayout                pipeline_layout;
        VkPipeline                      pipeline;
        VkDescriptorPool                descriptor_pool;
        VkDescriptorSetLayout           descriptor_set_layout;
        VkDescriptorSet                 descriptor_set;
        vulkan_imgui_push_const_block   push_const_block;
        int32_t                         vertex_count;
        int32_t                         index_count;
        void*                           vertex_data_mapped;
        void*                           index_data_mapped;
    };
}

#endif // _LNA_BACKENDS_VULKAN_VULKAN_IMGUI_HPP_
