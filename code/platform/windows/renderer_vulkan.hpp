#ifndef _LAN_PLATFORM_WINDOWS_RENDERER_VULKAN_HPP_
#define _LAN_PLATFORM_WINDOWS_RENDERER_VULKAN_HPP_

#include <vulkan.h>
#include "platform/renderer.hpp"
#include "platform/windows/window_sdl.hpp"

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
        std::vector<VkImage>            _vulkan_swap_chain_images;
        VkFormat                        _vulkan_swap_chain_image_format;
        VkExtent2D                      _vulkan_swap_chain_extent;
        std::vector<VkImageView>        _vulkan_swap_chain_image_views;
        VkRenderPass                    _vulkan_render_pass                 { nullptr };
        VkPipeline                      _vulkan_graphics_pipeline           { nullptr };
        VkDescriptorSetLayout           _vulkan_descriptor_set_layout       { nullptr };
        VkPipelineLayout                _vulkan_pipeline_layout             { nullptr };
        std::vector<VkFramebuffer>      _vulkan_swap_chain_framebuffers;
        VkCommandPool                   _vulkan_command_pool                { nullptr };
        std::vector<VkCommandBuffer>    _vulkan_command_buffers;
        std::vector<VkSemaphore>        _vulkan_image_available_semaphores;
        std::vector<VkSemaphore>        _vulkan_render_finished_semaphores;
        std::vector<VkFence>            _vulkan_in_flight_fences;
        std::vector<VkFence>            _vulkan_images_in_flight_fences;
        size_t                          _curr_frame                         { 0 };
        VkBuffer                        _vulkan_vertex_buffer               { nullptr };
        VkDeviceMemory                  _vulkan_vertex_buffer_memory        { nullptr };
        VkBuffer                        _vulkan_index_buffer                { nullptr };
        VkDeviceMemory                  _vulkan_index_buffer_memory         { nullptr };
        std::vector<VkBuffer>           _vulkan_uniform_buffers;
        std::vector<VkDeviceMemory>     _vulkan_uniform_buffers_memory;
        VkDescriptorPool                _vulkan_descriptor_pool             { nullptr };
        std::vector<VkDescriptorSet>    _vulkan_descriptor_sets;
        VkImage                         _vulkan_texture_image               { nullptr };
        VkDeviceMemory                  _vulkan_texture_image_memory        { nullptr };
        VkImageView                     _vulkan_texture_image_view          { nullptr };
        VkSampler                       _vulkan_texture_sampler             { nullptr };
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
