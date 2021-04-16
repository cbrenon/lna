#ifndef _LAN_BACKENDS_VULKAN_VULKAN_BACKEND_HPP_
#define _LAN_BACKENDS_VULKAN_VULKAN_BACKEND_HPP_

#include <vulkan/vulkan.h>
#include "backends/vulkan/vulkan_texture.hpp"
#include "backends/vulkan/vulkan_mesh.hpp"
#include "backends/vulkan/vulkan_imgui.hpp"
#include "core/heap_array.hpp"

namespace lna
{
    constexpr uint32_t VULKAN_MAX_FRAMES_IN_FLIGHT = 2;

    struct backend_renderer;

    typedef void (*vulkan_on_swap_chain_cleanup)    (void* owner);
    typedef void (*vulkan_on_swap_chain_recreate)   (void* owner);
    typedef void (*vulkan_on_draw)                  (void* owner, uint32_t command_buffer_image_index);

    struct backend_renderer
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
        VkSemaphore                     image_available_semaphores[VULKAN_MAX_FRAMES_IN_FLIGHT];
        VkSemaphore                     render_finished_semaphores[VULKAN_MAX_FRAMES_IN_FLIGHT];
        VkFence                         in_flight_fences[VULKAN_MAX_FRAMES_IN_FLIGHT];
        VkImage                         depth_image;
        VkDeviceMemory                  depth_image_memory;
        VkImageView                     depth_image_view;

        //! PERSISTENT MEMORY POOL
        heap_array<VkFence>             images_in_flight_fences {};

        //! SWAP CHAIN MEMORY POOL
        heap_array<VkImage>             swap_chain_images;
        heap_array<VkImageView>         swap_chain_image_views;
        heap_array<VkFramebuffer>       swap_chain_framebuffers;
        heap_array<VkCommandBuffer>     command_buffers;

        enum memory_pool_id
        {
            FRAME_LIFETIME_MEMORY_POOL,
            PERSISTENT_LIFETIME_MEMORY_POOL,
            SWAP_CHAIN_LIFETIME_MEMORY_POOL,
            MEMORY_POOL_COUNT,
        };

        memory_pool                     memory_pools[MEMORY_POOL_COUNT];

        enum
        {
            MAX_SWAP_CHAIN_CALLBACKS = 10,
        };

        // TODO: for the moment we use the callback system to manage graphics objet type by backend struct to progress but later we will need to review and change the architecture to avoid function pointer.
        vulkan_on_swap_chain_cleanup    swap_chain_cleanup_callbacks[MAX_SWAP_CHAIN_CALLBACKS];
        vulkan_on_swap_chain_recreate   swap_chain_recreate_callbacks[MAX_SWAP_CHAIN_CALLBACKS];
        vulkan_on_draw                  draw_callbacks[MAX_SWAP_CHAIN_CALLBACKS];
        void*                           callback_owners[MAX_SWAP_CHAIN_CALLBACKS];
    };

    constexpr size_t MEMORY_POOL_SIZES[backend_renderer::MEMORY_POOL_COUNT] =
    {
        256, // SIZE IN MEGABYTES
        256, // SIZE IN MEGABYTES
        256, // SIZE IN MEGABYTES
    };

    void vulkan_renderer_backend_register_swap_chain_callbacks(
        backend_renderer&               backend,
        vulkan_on_swap_chain_cleanup    on_clean_up,
        vulkan_on_swap_chain_recreate   on_recreate,
        vulkan_on_draw                  on_draw,
        void*                           owner
        );
}

#endif // _LAN_BACKENDS_VULKAN_VULKAN_BACKEND_HPP_