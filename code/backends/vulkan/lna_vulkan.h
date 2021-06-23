#ifndef LNA_BACKENDS_VULKAN_LNA_VULKAN_H
#define LNA_BACKENDS_VULKAN_LNA_VULKAN_H

#include <stdbool.h>
#include <vulkan/vulkan.h>
#include "core/lna_log.h"
#include "core/lna_assert.h"

extern const char*      lna_vulkan_error_string                 (VkResult error_code);
extern void             lna_vulkan_check                        (VkResult result);
extern uint32_t         lna_vulkan_find_memory_type             (VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties);
extern void             lna_vulkan_create_buffer                (VkDevice device, VkPhysicalDevice physical_device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* buffer_memory);
extern void             lna_vulkan_create_image                 (VkDevice device, VkPhysicalDevice physical_device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* image_memory);
extern VkImageView      lna_vulkan_create_image_view            (VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags);
extern VkCommandBuffer  lna_vulkan_begin_single_time_commands   (VkDevice device, VkCommandPool command_pool);
extern void             lna_vulkan_end_single_time_commands     (VkDevice device, VkCommandPool command_pool, VkCommandBuffer command_buffer, VkQueue graphics_queue);
extern void             lna_vulkan_transition_image_layout      (VkDevice device, VkCommandPool command_pool, VkQueue graphics_queue, VkImage image, VkImageLayout old_layout, VkImageLayout new_layout);
extern void             lna_vulkan_copy_buffer_to_image         (VkDevice device, VkCommandPool command_pool, VkBuffer buffer, VkQueue graphics_queue, VkImage image, uint32_t width, uint32_t height);
extern void             lna_vulkan_copy_buffer                  (VkDevice device, VkCommandPool command_pool, VkQueue graphics_queue, VkBuffer src, VkBuffer dst, VkDeviceSize size);
extern VkShaderModule   lna_vulkan_create_shader_module         (VkDevice device, const uint32_t* code, size_t code_size);
extern VkFormat         lna_vulkan_find_supported_format        (VkPhysicalDevice physical_device, VkFormat* candidate_formats, uint32_t candidate_format_count, VkImageTiling tiling, VkFormatFeatureFlags features);
extern VkFormat         lna_vulkan_find_depth_format            (VkPhysicalDevice physical_device);
extern bool             lna_vulkan_has_stencil_component        (VkFormat format);

#endif
