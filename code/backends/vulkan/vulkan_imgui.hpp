#ifndef _LNA_BACKENDS_VULKAN_VULKAN_IMGUI_HPP_
#define _LNA_BACKENDS_VULKAN_VULKAN_IMGUI_HPP_

#include <vulkan/vulkan.h>
#include "imgui.h"
#include "maths/vec2.hpp"
#include "maths/vec4.hpp"
#include "backends/vulkan/vulkan_texture.hpp"

namespace lna
{
    struct memory_pool;

    struct vulkan_imgui_push_const_block
    {
		vec2                            scale;
		vec2                            translate;
	};

    struct vulkan_imgui_wrapper
    {
        VkBuffer                        vertex_buffer;
        VkDeviceMemory                  vertex_buffer_memory;
        VkBuffer                        index_buffer;
        VkDeviceMemory                  index_buffer_memory;
        vulkan_texture                  font_texture;
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

    struct vulkan_imgui_wrapper_config
    {
        float                           window_width;
        float                           window_height;
        VkDevice                        device;
        VkPhysicalDevice                physical_device;
        VkCommandPool                   command_pool;
        VkQueue                         graphics_queue;
        VkRenderPass                    render_pass;
        memory_pool*                    temp_memory_pool_ptr;
    };

    void vulkan_imgui_wrapper_init(
        vulkan_imgui_wrapper& imgui_wrapper
        );

    void vulkan_imgui_wrapper_configure(
        vulkan_imgui_wrapper& imgui_wrapper,
        vulkan_imgui_wrapper_config& config
        );

    void vulkan_imgui_wrapper_update(
        vulkan_imgui_wrapper& imgui_wrapper,
        VkDevice device,
        VkPhysicalDevice physical_device
        );

    void vulkan_imgui_wrapper_render_frame(
        vulkan_imgui_wrapper& imgui_wrapper,
        VkCommandBuffer command_buffer
        );

    void vulkan_imgui_wrapper_release(
        vulkan_imgui_wrapper& imgui_wrapper,
        VkDevice device
        );
}

#endif // _LNA_BACKENDS_VULKAN_VULKAN_IMGUI_HPP_
