#ifndef _LAN_BACKENDS_VULKAN_VULKAN_BACKEND_HPP_
#define _LAN_BACKENDS_VULKAN_VULKAN_BACKEND_HPP_

#include <vulkan/vulkan.h>
//#include "backends/vulkan/vulkan_imgui.hpp"

namespace lna
{
    constexpr uint32_t VULKAN_MAX_FRAMES_IN_FLIGHT = 2;

    struct memory_pool;

    // TODO: to remove when vulkan_texture_backend is implemented
    // struct vulkan_texture_system
    // {
    //     vulkan_texture*                 textures;
    //     uint32_t                        cur_texture_count;
    //     uint32_t                        max_texture_count;
    // };

    // TODO: to remove when vulkan_mesh_backend is implemented
    // struct vulkan_mesh_system
    // {
    //     vulkan_mesh*                    meshes;
    //     vec3*                           mesh_positions;
    //     uint32_t                        cur_mesh_count;
    //     uint32_t                        max_mesh_count;
    //     VkDescriptorSetLayout           descriptor_set_layout;
    //     VkDescriptorPool                descriptor_pool;
    //     mat4                            projection;
    //     mat4                            view;
    // };

    struct renderer_backend;

    typedef void (*vulkan_on_swap_chain_cleanup) (void* owner);
    typedef void (*vulkan_on_swap_chain_recreate)(void* owner);

    struct renderer_backend
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
        VkCommandPool                   command_pool;
        size_t                          curr_frame;

        //VkPipeline                      graphics_pipeline;  // TODO: to remove when vulkan_mesh_backend is implemented
        //VkPipelineLayout                pipeline_layout;    // TODO: to remove when vulkan_mesh_backend is implemented

        //vulkan_texture_system           texture_system;     // TODO: to remove when vulkan_texture_backend is implemented
        //vulkan_mesh_system              mesh_system;        // TODO: to remove when vulkan_mesh_backend is implemented

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

        enum
        {
            MAX_SWAP_CHAIN_CALLBACKS = 10,
        };

        vulkan_on_swap_chain_cleanup    swap_chain_cleanup_callbacks[MAX_SWAP_CHAIN_CALLBACKS];
        vulkan_on_swap_chain_recreate   swap_chain_recreate_callbacks[MAX_SWAP_CHAIN_CALLBACKS];
        void*                           swap_chain_callback_owners[MAX_SWAP_CHAIN_CALLBACKS];

        //vulkan_imgui_wrapper            imgui_wrapper;      // TODO: remove from vulkan_backend
    };

    constexpr size_t MEMORY_POOL_SIZES[renderer_backend::MEMORY_POOL_COUNT] =
    {
        256, // SIZE IN MEGABYTES
        256, // SIZE IN MEGABYTES
        256, // SIZE IN MEGABYTES
    };

    void vulkan_renderer_backend_register_swap_chain_callbacks(
        renderer_backend& backend,
        vulkan_on_swap_chain_cleanup on_clean_up,
        vulkan_on_swap_chain_recreate on_recreate,
        void* owner
        );
}

#endif // _LAN_BACKENDS_VULKAN_VULKAN_BACKEND_HPP_
