#ifndef _LAN_PLATFORM_VULKAN_VULKAN_RENDERER_HPP_
#define _LAN_PLATFORM_VULKAN_VULKAN_RENDERER_HPP_

#include <vulkan.h>
#include "platform/renderer.hpp"
#include "platform/sdl/sdl_window.hpp"
#include "core/memory_pool.hpp"
#include "platform/vulkan/vulkan_mesh.hpp"
#include "platform/vulkan/vulkan_texture.hpp"
namespace lna
{
    struct vulkan_renderer
    {
        VkInstance                      instance;
        VkDebugUtilsMessengerEXT        debug_messenger;
        VkPhysicalDevice                physical_device;
        VkDevice                        device;
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
        // VkBuffer                        vertex_buffer;
        // VkDeviceMemory                  vertex_buffer_memory;
        // VkBuffer                        index_buffer;
        // VkDeviceMemory                  index_buffer_memory;

        // TODO: to remove after test validation
        vulkan_texture                  vk_texture;
        vulkan_mesh                     vk_mesh;

        // TODO: for the moment we only manage textured mesh but in the futur we will have to manage just colored primitive. So we will have to create specific descriptor_set_layout and descriptor pool for them.
        VkDescriptorSetLayout           descriptor_set_layout;
        VkDescriptorPool                descriptor_pool;

        // VkImage                         texture_image;
        // VkDeviceMemory                  texture_image_memory;
        // VkImageView                     texture_image_view;
        // VkSampler                       texture_sampler;

        //! PERSISTENT MEMORY POOL
        heap_array<VkSemaphore>         image_available_semaphores;
        heap_array<VkSemaphore>         render_finished_semaphores;
        heap_array<VkFence>             in_flight_fences;
        heap_array<VkFence>             images_in_flight_fences;

        //! SWAP CHAIN MEMORY POOL
        heap_array<VkImage>             swap_chain_images;
        heap_array<VkImageView>         swap_chain_image_views;
        heap_array<VkFramebuffer>       swap_chain_framebuffers;
        heap_array<VkCommandBuffer>     command_buffers;
        // heap_array<VkBuffer>            uniform_buffers;
        // heap_array<VkDeviceMemory>      uniform_buffers_memory;
        // heap_array<VkDescriptorSet>     descriptor_sets;

        enum memory_pool_id
        {
            FRAME_LIFETIME_MEMORY_POOL,
            PERSISTENT_LIFETIME_MEMORY_POOL,
            SWAP_CHAIN_LIFETIME_MEMORY_POOL,
            MEMORY_POOL_COUNT,
        };

        memory_pool                     memory_pools[MEMORY_POOL_COUNT];
    };

    constexpr size_t MEMORY_POOL_SIZES[vulkan_renderer::MEMORY_POOL_COUNT] =
    {
        256, // SIZE IN MEGABYTES
        256, // SIZE IN MEGABYTES
        256, // SIZE IN MEGABYTES
    };

    template<>
    void renderer_init<vulkan_renderer>(
        vulkan_renderer& renderer
        );

    template<>
    void renderer_configure<vulkan_renderer, sdl_window>(
        vulkan_renderer& renderer,
        const renderer_config<sdl_window>& config
        );

    template<>
    void renderer_draw_frame<vulkan_renderer>(
        vulkan_renderer& renderer,
        bool framebuffer_resized,
        uint32_t framebuffer_width,
        uint32_t framebuffer_height
        );

    template<>
    void renderer_release<vulkan_renderer>(
        vulkan_renderer& renderer
        );
}

#endif // _LAN_PLATFORM_VULKAN_VULKAN_RENDERER_HPP_
