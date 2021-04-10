#ifndef _LNA_BACKENDS_VULKAN_VULKAN_HELPERS_HPP_
#define _LNA_BACKENDS_VULKAN_VULKAN_HELPERS_HPP_

#include <vulkan/vulkan.h>
#include "core/log.hpp"
#include "core/assert.hpp"

#define VULKAN_CHECK_RESULT(func)                                                       \
    {                                                                                   \
        VkResult _result = (func);                                                      \
        if (_result != VK_SUCCESS)                                                      \
        {                                                                               \
            lna::log::error("vulkan %s", lna::vulkan_helpers::error_string(_result));   \
            LNA_ASSERT(0);                                                              \
        }                                                                               \
    }

namespace lna
{
    class memory_pool;

    namespace vulkan_helpers
    {
        const char* error_string(
            VkResult error_code
            );

        uint32_t find_memory_type(
            VkPhysicalDevice physical_device,
            uint32_t type_filter,
            VkMemoryPropertyFlags properties
            );

        void create_buffer(
            VkDevice device,
            VkPhysicalDevice physical_device,
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer& buffer,
            VkDeviceMemory& buffer_memory
            );

        void create_image(
            VkDevice device,
            VkPhysicalDevice physical_device,
            uint32_t width,
            uint32_t height,
            VkFormat format,
            VkImageTiling tiling,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkImage& image,
            VkDeviceMemory& image_memory
            );

        VkImageView create_image_view(
            VkDevice device,
            VkImage image,
            VkFormat format,
            VkImageAspectFlags aspect_flags
            );

        VkCommandBuffer begin_single_time_commands(
            VkDevice device,
            VkCommandPool command_pool
            );

        void end_single_time_commands(
            VkDevice device,
            VkCommandPool command_pool,
            VkCommandBuffer command_buffer,
            VkQueue graphics_queue
            );

        void transition_image_layout(
            VkDevice device,
            VkCommandPool command_pool,
            VkQueue graphics_queue,
            VkImage image,
            VkImageLayout old_layout,
            VkImageLayout new_layout
            );

        void copy_buffer_to_image(
            VkDevice device,
            VkCommandPool command_pool,
            VkBuffer buffer,
            VkQueue graphics_queue,
            VkImage image,
            uint32_t width,
            uint32_t height
            );

        void copy_buffer(
            VkDevice device,
            VkCommandPool command_pool,
            VkQueue graphics_queue,
            VkBuffer src,
            VkBuffer dst,
            VkDeviceSize size
            );

        VkShaderModule load_shader(
            VkDevice device,
            const char* filename,
            memory_pool& pool
            );

        VkFormat find_supported_format(
            VkPhysicalDevice physical_device,
            VkFormat* candidate_formats,
            uint32_t candidate_format_count,
            VkImageTiling tiling,
            VkFormatFeatureFlags features
            );

        VkFormat find_depth_format(
            VkPhysicalDevice physical_device
            );

        bool has_stencil_component(
            VkFormat format
            );
    }
}

#endif // _LNA_BACKENDS_VULKAN_VULKAN_HELPERS_HPP_