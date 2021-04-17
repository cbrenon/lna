#ifndef LNA_BACKENDS_VULKAN_LNA_RENDERER_VULKAN_H
#define LNA_BACKENDS_VULKAN_LNA_RENDERER_VULKAN_H

#include <vulkan/vulkan.h>
#include "backends/lna_renderer.h"
#include "core/lna_container.h"

#define LNA_VULKAN_MAX_FRAMES_IN_FLIGHT 2

typedef enum lna_vulkan_renderer_memory_pool_s
{
    LNA_VULKAN_RENDERER_MEMORY_POOL_FRAME,
    LNA_VULKAN_RENDERER_MEMORY_POOL_PERSISTENT,
    LNA_VULKAN_RENDERER_MEMORY_POOL_SWAP_CHAIN,
    LNA_VULKAN_RENDERER_MEMORY_POOL_COUNT
} lana_renderer_memory_pool_t;

lna_array_def(VkFence)          lna_vulkan_fence_array_t;
lna_array_def(VkImage)          lna_vulkan_image_array_t;
lna_array_def(VkImageView)      lna_vulkan_image_view_array_t;
lna_array_def(VkFramebuffer)    lna_vulkan_frame_buffer_array_t;
lna_array_def(VkCommandBuffer)  lna_vulkan_command_buffer_array_t;

typedef struct lna_renderer_s
{
    VkInstance                          instance;
    VkDebugUtilsMessengerEXT            debug_messenger;
    VkPhysicalDevice                    physical_device;
    VkDevice                            device;
    uint32_t                            graphics_family;
    VkQueue                             graphics_queue;
    VkSurfaceKHR                        surface;
    VkQueue                             present_queue;
    VkSwapchainKHR                      swap_chain;
    VkFormat                            swap_chain_image_format;
    VkExtent2D                          swap_chain_extent;
    VkRenderPass                        render_pass;
    VkCommandPool                       command_pool;
    size_t                              curr_frame;
    VkSemaphore                         image_available_semaphores[LNA_VULKAN_MAX_FRAMES_IN_FLIGHT];
    VkSemaphore                         render_finished_semaphores[LNA_VULKAN_MAX_FRAMES_IN_FLIGHT];
    VkFence                             in_flight_fences[LNA_VULKAN_MAX_FRAMES_IN_FLIGHT];
    VkImage                             depth_image;
    VkDeviceMemory                      depth_image_memory;
    VkImageView                         depth_image_view;
    lna_memory_pool_t                   memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_COUNT];
    lna_vulkan_fence_array_t            images_in_flight_fences;
    lna_vulkan_image_array_t            swap_chain_images;
    lna_vulkan_image_view_array_t       swap_chain_image_views;
    lna_vulkan_frame_buffer_array_t     swap_chain_framebuffers;
    lna_vulkan_command_buffer_array_t   command_buffers;
    uint32_t                            image_index;
} lna_renderer_t;

#endif
