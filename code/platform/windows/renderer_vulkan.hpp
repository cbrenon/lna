#ifndef _LAN_PLATFORM_WINDOWS_RENDERER_VULKAN_HPP_
#define _LAN_PLATFORM_WINDOWS_RENDERER_VULKAN_HPP_

#include <vulkan.h>
#include "platform/renderer.hpp"
#include "platform/windows/window_sdl.hpp"
#include "core/memory_pool.hpp"

namespace lna
{
    struct renderer_vulkan
    {
        VkInstance                      _vulkan_instance                    { nullptr };
        VkDebugUtilsMessengerEXT        _vulkan_debug_messenger             { nullptr };
        VkPhysicalDevice                _vulkan_physical_device             { nullptr };
        VkDevice                        _vulkan_device                      { nullptr };
        VkQueue                         _vulkan_graphics_queue              { nullptr };
        VkSurfaceKHR                    _vulkan_surface                     { nullptr };
        VkQueue                         _vulkan_present_queue               { nullptr };
        VkSwapchainKHR                  _vulkan_swap_chain                  { nullptr };
        VkFormat                        _vulkan_swap_chain_image_format;
        VkExtent2D                      _vulkan_swap_chain_extent;
        VkRenderPass                    _vulkan_render_pass                 { nullptr };
        VkPipeline                      _vulkan_graphics_pipeline           { nullptr };
        VkDescriptorSetLayout           _vulkan_descriptor_set_layout       { nullptr };
        VkPipelineLayout                _vulkan_pipeline_layout             { nullptr };
        VkCommandPool                   _vulkan_command_pool                { nullptr };
        size_t                          _curr_frame                         { 0 };
        VkBuffer                        _vulkan_vertex_buffer               { nullptr };
        VkDeviceMemory                  _vulkan_vertex_buffer_memory        { nullptr };
        VkBuffer                        _vulkan_index_buffer                { nullptr };
        VkDeviceMemory                  _vulkan_index_buffer_memory         { nullptr };
        VkDescriptorPool                _vulkan_descriptor_pool             { nullptr };
        VkImage                         _vulkan_texture_image               { nullptr };
        VkDeviceMemory                  _vulkan_texture_image_memory        { nullptr };
        VkImageView                     _vulkan_texture_image_view          { nullptr };
        VkSampler                       _vulkan_texture_sampler             { nullptr };

        //! PERSISTENT MEMORY POOL
        heap_array<VkSemaphore>         _vulkan_image_available_semaphores;
        heap_array<VkSemaphore>         _vulkan_render_finished_semaphores;
        heap_array<VkFence>             _vulkan_in_flight_fences;
        heap_array<VkFence>             _vulkan_images_in_flight_fences;

        //! SWAP CHAIN MEMORY POOL
        heap_array<VkImage>             _vulkan_swap_chain_images;
        heap_array<VkImageView>         _vulkan_swap_chain_image_views;
        heap_array<VkFramebuffer>       _vulkan_swap_chain_framebuffers;
        heap_array<VkCommandBuffer>     _vulkan_command_buffers;
        heap_array<VkBuffer>            _vulkan_uniform_buffers;
        heap_array<VkDeviceMemory>      _vulkan_uniform_buffers_memory;
        heap_array<VkDescriptorSet>     _vulkan_descriptor_sets;

        enum memory_pool_id
        {
            FRAME_LIFETIME_MEMORY_POOL,
            PERSISTENT_LIFETIME_MEMORY_POOL,
            SWAP_CHAIN_LIFETIME_MEMORY_POOL,
            MEMORY_POOL_COUNT,
        };

        stack_array<memory_pool, MEMORY_POOL_COUNT> _memory_pools;
    };

    constexpr size_t MEMORY_POOL_SIZES[renderer_vulkan::MEMORY_POOL_COUNT] =
    {
        256, // SIZE IN MEGABYTES
        256, // SIZE IN MEGABYTES
        256, // SIZE IN MEGABYTES
    };

    template<>
    void renderer_init<renderer_vulkan, window_sdl>(
        renderer_vulkan& renderer,
        const renderer_config<window_sdl>& config
        );

    template<>
    void renderer_draw_frame<renderer_vulkan>(
        renderer_vulkan& renderer,
        bool framebuffer_resized,
        uint32_t framebuffer_width,
        uint32_t framebuffer_height
        );

    template<>
    void renderer_release<renderer_vulkan>(
        renderer_vulkan& renderer
        );
}

#endif // _LAN_PLATFORM_WINDOWS_RENDERER_VULKAN_HPP_
