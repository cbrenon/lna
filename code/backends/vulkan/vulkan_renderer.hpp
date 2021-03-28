#ifndef _LAN_BACKENDS_VULKAN_VULKAN_RENDERER_HPP_
#define _LAN_BACKENDS_VULKAN_VULKAN_RENDERER_HPP_

#include <vulkan/vulkan.h>
#include "backends/vulkan/vulkan_texture.hpp"
#include "backends/vulkan/vulkan_mesh.hpp"
#include "backends/vulkan/vulkan_imgui.hpp"

namespace lna
{
    constexpr uint32_t VULKAN_MAX_FRAMES_IN_FLIGHT = 2;

    struct vulkan_texture_system
    {
        vulkan_texture*                 textures;
        uint32_t                        cur_texture_count;
        uint32_t                        max_texture_count;
    };

    struct vulkan_mesh_system
    {
        vulkan_mesh*                    meshes;
        vec3*                           mesh_positions;
        uint32_t                        cur_mesh_count;
        uint32_t                        max_mesh_count;
        VkDescriptorSetLayout           descriptor_set_layout;
        VkDescriptorPool                descriptor_pool;
    };

    struct memory_pool;

    struct renderer_api
    {
        VkInstance                      instance;
        VkDebugUtilsMessengerEXT        debug_messenger;
        VkPhysicalDevice                physical_device;
        VkDevice                        device;
        uint32_t                        graphics_family;
        VkQueue                         graphics_queue;
        VkSurfaceKHR                    surface;
        VkQueue                         present_queue;
        VkSwapchainKHR                  swap_chain;
        VkFormat                        swap_chain_image_format;
        VkExtent2D                      swap_chain_extent;
        VkRenderPass                    render_pass;
        VkPipeline                      graphics_pipeline;
        VkPipelineLayout                pipeline_layout;
        VkCommandPool                   command_pool;
        size_t                          curr_frame;

        vulkan_texture_system           texture_system;
        vulkan_mesh_system              mesh_system;

        VkSemaphore                     image_available_semaphores[VULKAN_MAX_FRAMES_IN_FLIGHT];
        VkSemaphore                     render_finished_semaphores[VULKAN_MAX_FRAMES_IN_FLIGHT];
        VkFence                         in_flight_fences[VULKAN_MAX_FRAMES_IN_FLIGHT];

        //! PERSISTENT MEMORY POOL
        VkFence*                        images_in_flight_fences;

        //! SWAP CHAIN MEMORY POOL
        VkImage*                        swap_chain_images;
        VkImageView*                    swap_chain_image_views;
        VkFramebuffer*                  swap_chain_framebuffers;
        VkCommandBuffer*                command_buffers;
        uint32_t                        swap_chain_image_count;

        enum memory_pool_id
        {
            FRAME_LIFETIME_MEMORY_POOL,
            PERSISTENT_LIFETIME_MEMORY_POOL,
            SWAP_CHAIN_LIFETIME_MEMORY_POOL,
            MEMORY_POOL_COUNT,
        };

        memory_pool*                    memory_pools[MEMORY_POOL_COUNT];

        vulkan_imgui_wrapper            imgui_wrapper;
    };

    constexpr size_t MEMORY_POOL_SIZES[renderer_api::MEMORY_POOL_COUNT] =
    {
        256, // SIZE IN MEGABYTES
        256, // SIZE IN MEGABYTES
        256, // SIZE IN MEGABYTES
    };
}

#endif // _LAN_BACKENDS_VULKAN_VULKAN_RENDERER_HPP_
