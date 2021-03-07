#include <cstring>
#include <optional>
#include <set>
#include <algorithm>
#include <array>
#include <string>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlanguage-extension-token"
#pragma warning(push, 0)
#include <SDL.h>
#pragma warning(pop)
#pragma clang diagnostic pop
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-compare"
#pragma warning(push, 0)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#pragma warning(pop)
#pragma clang diagnostic pop
#include "platform/windows/renderer_vulkan.hpp"
#include "core/file.hpp"
#include "core/log.hpp"
#include "core/assert.hpp"
#include "maths/vec2.hpp"
#include "maths/vec4.hpp"
#include "maths/mat4.hpp"

#define VULKAN_CHECK_RESULT(func)                                                   \
    {                                                                               \
        VkResult _result = (func);                                                  \
        if (_result != VK_SUCCESS)                                                  \
        {                                                                           \
            lna::log::error("vulkan %s", vulkan_error_string(_result));             \
            LNA_ASSERT(0);                                                          \
        }                                                                           \
    }

namespace
{
    constexpr int VULKAN_MAX_FRAMES_IN_FLIGHT = 2;

    const std::vector<const char*> VULKAN_VALIDATION_LAYERS =
    {
        "VK_LAYER_KHRONOS_validation",
    };

    const std::vector<const char*> VULKAN_DEVICE_EXTENSIONS =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    struct vulkan_queue_family_indices
    {
        std::optional<uint32_t> graphics_family;
        std::optional<uint32_t> present_family;

        bool is_complete()
        {
            return graphics_family.has_value() && present_family.has_value();
        }
    };

    struct vulkan_swap_chain_support_details
    {
        VkSurfaceCapabilitiesKHR        capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR>   present_modes;
    };

    struct vertex
    {
        lna::vec2   position;
        lna::vec4   color;
        lna::vec2   uv;

        static VkVertexInputBindingDescription binding_description()
        {
            VkVertexInputBindingDescription binding_description_result{};
            binding_description_result.binding     = 0;
            binding_description_result.stride      = sizeof(vertex);
            binding_description_result.inputRate   = VK_VERTEX_INPUT_RATE_VERTEX;
            return binding_description_result;
        }

        static std::array<VkVertexInputAttributeDescription, 3> attribute_descriptions()
        {
            std::array<VkVertexInputAttributeDescription, 3> attribute_descriptions_result{};
            attribute_descriptions_result[0].binding    = 0;
            attribute_descriptions_result[0].location   = 0;
            attribute_descriptions_result[0].format     = VK_FORMAT_R32G32_SFLOAT;
            attribute_descriptions_result[0].offset     = offsetof(vertex, position);
            attribute_descriptions_result[1].binding    = 0;
            attribute_descriptions_result[1].location   = 1;
            attribute_descriptions_result[1].format     = VK_FORMAT_R32G32B32A32_SFLOAT;
            attribute_descriptions_result[1].offset     = offsetof(vertex, color);
            attribute_descriptions_result[2].binding    = 0;
            attribute_descriptions_result[2].location   = 2;
            attribute_descriptions_result[2].format     = VK_FORMAT_R32G32_SFLOAT;
            attribute_descriptions_result[2].offset     = offsetof(vertex, uv);
            return attribute_descriptions_result;
        }
    };

    struct uniform_buffer_object
    {
        alignas(16) lna::mat4   model;
        alignas(16) lna::mat4   view;
        alignas(16) lna::mat4   projection;
    };
    
    const std::vector<vertex> VERTICES =
    {
        {
            {-0.5f, -0.5f },
            { 1.0f, 1.0f, 1.0f, 1.0f },
            { 1.0f, 0.0f },
        },
        {
            { 0.5f, -0.5f },
            { 1.0f, 1.0f, 1.0f, 1.0f },
            { 0.0f, 0.0f },
        },
        {
            { 0.5f, 0.5f },
            { 1.0f, 1.0f, 1.0f, 1.0f },
            { 0.0f, 1.0f },
        },
        {
            { -0.5f, 0.5f },
            { 1.0f, 1.0f, 1.0f, 1.0f },
            { 1.0f, 1.0f },
        }
    };

    const std::vector<uint16_t> INDICES =
    {
        0, 1, 2, 2, 3, 0
    };

    //! VULKAN DEBUG FUNCTIONS -------------------------------------------------

    VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        VkDebugUtilsMessageTypeFlagsEXT message_type,
        const VkDebugUtilsMessengerCallbackDataEXT* call_back_data_ptr,
        void* user_data_ptr
        )
    {
        (void)message_type;
        (void)user_data_ptr;

        if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            lna::log::error("vulkan validation layer: %s", call_back_data_ptr->pMessage);
        }
        return VK_FALSE;
    }

    VkResult vulkan_create_debug_utils_messenger_EXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* create_info_ptr,
        const VkAllocationCallbacks* allocator_ptr,
        VkDebugUtilsMessengerEXT* debug_messenger_ptr
    )
    {
        LNA_ASSERT(instance);

        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance,
            "vkCreateDebugUtilsMessengerEXT"
            );
        if (func != nullptr)
        {
            return func(
                instance,
                create_info_ptr,
                allocator_ptr,
                debug_messenger_ptr
                );
        }
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    void vulkan_destroy_debug_utilis_messenger_EXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debug_messenger,
        const VkAllocationCallbacks* allocator_ptr
        )
    {
        LNA_ASSERT(instance);

        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance,
            "vkDestroyDebugUtilsMessengerEXT"
            );
        if (func)
        {
            func(
                instance,
                debug_messenger,
                allocator_ptr
                );
        }
    }

    //! VULKAN HELPERS FUNCTION ------------------------------------------------

    const char* vulkan_error_string(VkResult error_code)
    {
        switch (error_code)
		{
#define STR(r) case VK_ ##r: return #r
			STR(NOT_READY);
			STR(TIMEOUT);
			STR(EVENT_SET);
			STR(EVENT_RESET);
			STR(INCOMPLETE);
			STR(ERROR_OUT_OF_HOST_MEMORY);
			STR(ERROR_OUT_OF_DEVICE_MEMORY);
			STR(ERROR_INITIALIZATION_FAILED);
			STR(ERROR_DEVICE_LOST);
			STR(ERROR_MEMORY_MAP_FAILED);
			STR(ERROR_LAYER_NOT_PRESENT);
			STR(ERROR_EXTENSION_NOT_PRESENT);
			STR(ERROR_FEATURE_NOT_PRESENT);
			STR(ERROR_INCOMPATIBLE_DRIVER);
			STR(ERROR_TOO_MANY_OBJECTS);
			STR(ERROR_FORMAT_NOT_SUPPORTED);
			STR(ERROR_SURFACE_LOST_KHR);
			STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
			STR(SUBOPTIMAL_KHR);
			STR(ERROR_OUT_OF_DATE_KHR);
			STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
			STR(ERROR_VALIDATION_FAILED_EXT);
			STR(ERROR_INVALID_SHADER_NV);
#undef STR
			default:
				return "UNKNOWN_ERROR";
		}
    }

    uint32_t vulkan_find_memory_type(
        lna::renderer_vulkan& renderer,
        uint32_t type_filter,
        VkMemoryPropertyFlags properties
        )
    {
        VkPhysicalDeviceMemoryProperties memory_properties;
        vkGetPhysicalDeviceMemoryProperties(
            renderer._vulkan_physical_device,
            &memory_properties
            );
        for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i)
        {
            if (
                (type_filter & (1 << i))
                && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties
                )
            {
                return i;
            }
        }
        LNA_ASSERT(0);
        return (uint32_t)-1;
    }

    bool vulkan_check_validation_layer_support()
    {
        uint32_t layer_count;

        VULKAN_CHECK_RESULT(
            vkEnumerateInstanceLayerProperties(
                &layer_count,
                nullptr
                )
            )

        std::vector<VkLayerProperties> available_layers(layer_count);

        VULKAN_CHECK_RESULT(
            vkEnumerateInstanceLayerProperties(
                &layer_count,
                available_layers.data()
                )
            )

        for (auto layer_name : VULKAN_VALIDATION_LAYERS)
        {
            auto layer_found = false;
            for (const auto& layer_properties : available_layers)
            {
                if (strcmp(layer_name, layer_properties.layerName) == 0)
                {
                    layer_found = true;
                    break;
                }
            }

            if (!layer_found)
            {
                return false;
            }
        }
        return true;
    }

    vulkan_queue_family_indices vulkan_find_queue_families(
        VkPhysicalDevice device,
        VkSurfaceKHR surface
        )
    {
        LNA_ASSERT(device);
        LNA_ASSERT(surface);

        vulkan_queue_family_indices indices;
        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(
            device,
            &queue_family_count,
            nullptr
            );

        std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(
            device,
            &queue_family_count,
            queue_families.data()
            );

        uint32_t index = 0;
        for (const auto& queue_family : queue_families)
        {
            if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphics_family = index;
            }

            VkBool32 present_support = false;
            VULKAN_CHECK_RESULT(
                vkGetPhysicalDeviceSurfaceSupportKHR(
                    device,
                    index,
                    surface,
                    &present_support
                    )
                )
            if (present_support)
            {
                indices.present_family = index;
            }

            if (indices.is_complete())
            {
                break;
            }

            ++index;
        }
        return indices;
    }

    bool vulkan_check_device_extension_support(
        VkPhysicalDevice device
        )
    {
        LNA_ASSERT(device);

        uint32_t extension_count = 0;
        VULKAN_CHECK_RESULT(
            vkEnumerateDeviceExtensionProperties(
                device,
                nullptr,
                &extension_count,
                nullptr
                )
            )
        std::vector<VkExtensionProperties> available_extensions(extension_count);
        VULKAN_CHECK_RESULT(
            vkEnumerateDeviceExtensionProperties(
                device,
                nullptr,
                &extension_count,
                available_extensions.data()
                )
            )

        std::set<std::string> required_extensions(
            VULKAN_DEVICE_EXTENSIONS.begin(),
            VULKAN_DEVICE_EXTENSIONS.end()
            );
        for (const auto& extension : available_extensions)
        {
            required_extensions.erase(extension.extensionName);
        }
        return required_extensions.empty();
    }

    vulkan_swap_chain_support_details vulkan_query_swap_chain_support(
        VkPhysicalDevice device,
        VkSurfaceKHR surface
        )
    {
        LNA_ASSERT(device);
        LNA_ASSERT(surface);

        vulkan_swap_chain_support_details details;
        VULKAN_CHECK_RESULT(
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                device,
                surface,
                &details.capabilities
                )
            )

        uint32_t format_count = 0;
        VULKAN_CHECK_RESULT(
            vkGetPhysicalDeviceSurfaceFormatsKHR(
                device,
                surface,
                &format_count,
                nullptr
                )
            )
        if (format_count != 0)
        {
            details.formats.resize(format_count);
            VULKAN_CHECK_RESULT(
                vkGetPhysicalDeviceSurfaceFormatsKHR(
                    device,
                    surface,
                    &format_count,
                    details.formats.data()
                    )
                )
        }

        uint32_t present_mode_count = 0;
        VULKAN_CHECK_RESULT(
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                device,
                surface,
                &present_mode_count,
                nullptr
                )
            )
        if (present_mode_count != 0)
        {
            details.present_modes.resize(present_mode_count);
            VULKAN_CHECK_RESULT(
                vkGetPhysicalDeviceSurfacePresentModesKHR(
                    device,
                    surface,
                    &present_mode_count,
                    details.present_modes.data()
                    )
                )
        }

        return details;
    }

    VkSurfaceFormatKHR vulkan_choose_swap_surface_format(
        const std::vector<VkSurfaceFormatKHR>& available_formats
        )
    {
        for (const auto& available_format : available_formats)
        {
            if (
                available_format.format == VK_FORMAT_B8G8R8A8_SRGB
                && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
                )
            {
                return available_format;
            }
        }
        return available_formats[0];
    }

    VkPresentModeKHR vulkan_choose_swap_present_mode(
        const std::vector<VkPresentModeKHR>& available_present_modes
        )
    {
        for (const auto& available_present_mode : available_present_modes)
        {
            if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return available_present_mode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D vulkan_choose_swap_extent(
        const VkSurfaceCapabilitiesKHR& capabilities,
        uint32_t framebuffer_width,
        uint32_t framebuffer_height
        )
    {
        if (capabilities.currentExtent.width != UINT32_MAX)
        {
            return capabilities.currentExtent;
        }
        VkExtent2D actual_extent =
        {
            std::clamp(
                framebuffer_width,
                capabilities.minImageExtent.width,
                capabilities.maxImageExtent.width
                ),
            std::clamp(
                framebuffer_height,
                capabilities.minImageExtent.height,
                capabilities.maxImageExtent.height
                ),
        };
        return actual_extent;
    }

    bool vulkan_is_physical_device_suitable(
        VkPhysicalDevice device,
        VkSurfaceKHR surface
        )
    {
        vulkan_queue_family_indices indices = vulkan_find_queue_families(
            device,
            surface
            );
        bool supported_extensions = vulkan_check_device_extension_support(device);
        bool swap_chain_adequate = false;

        if (supported_extensions)
        {
            vulkan_swap_chain_support_details swap_chain_support = vulkan_query_swap_chain_support(
                device,
                surface
                );
            swap_chain_adequate = !swap_chain_support.formats.empty() && !swap_chain_support.present_modes.empty();
        }

        VkPhysicalDeviceFeatures supported_features{};
        vkGetPhysicalDeviceFeatures(
            device,
            &supported_features
            );

        //? NOTE: we can pick a physical device without supported_features.samplerAnisotropy
        //? in this case we must indicate somewhere that the choosen physical device does not managed sampler anisotropy and when
        //? we will create a sampler create info we must check it and set to:
        //? sampler_create_info.anisotropyEnable    = VK_FALSE;
        //? sampler_create_info.maxAnisotropy       = 1.0f;
        //? see the end of https://vulkan-tutorial.com/Texture_mapping/Image_view_and_sampler for more information
        return indices.is_complete() && supported_extensions && swap_chain_adequate && supported_features.samplerAnisotropy;
    }

    VkCommandBuffer vulkan_begin_single_time_commands(
        lna::renderer_vulkan& renderer
        )
    {
        LNA_ASSERT(renderer._vulkan_device);

        VkCommandBufferAllocateInfo allocate_info{};
        allocate_info.sType                 = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocate_info.level                 = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocate_info.commandPool           = renderer._vulkan_command_pool;
        allocate_info.commandBufferCount    = 1;

        VkCommandBuffer command_buffer = nullptr;
        VULKAN_CHECK_RESULT(
            vkAllocateCommandBuffers(
                renderer._vulkan_device,
                &allocate_info,
                &command_buffer
                )
            )

        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VULKAN_CHECK_RESULT(
            vkBeginCommandBuffer(
                command_buffer,
                &begin_info
                )
            )

        return command_buffer;
    }

    void vulkan_end_single_time_commands(
        lna::renderer_vulkan& renderer,
        VkCommandBuffer command_buffer
        )
    {
        LNA_ASSERT(renderer._vulkan_device);
        LNA_ASSERT(command_buffer);

        VULKAN_CHECK_RESULT(
            vkEndCommandBuffer(
                command_buffer
                )
            )

        VkSubmitInfo submit_info{};
        submit_info.sType               = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount  = 1;
        submit_info.pCommandBuffers     = &command_buffer;

        VULKAN_CHECK_RESULT(
            vkQueueSubmit(
                renderer._vulkan_graphics_queue,
                1,
                &submit_info,
                VK_NULL_HANDLE
                )
            )
        VULKAN_CHECK_RESULT(
            vkQueueWaitIdle(
                renderer._vulkan_graphics_queue
                )
            )
        vkFreeCommandBuffers(
            renderer._vulkan_device,
            renderer._vulkan_command_pool,
            1,
            &command_buffer
            );
    }

    void vulkan_transition_image_layout(
        lna::renderer_vulkan& renderer,
        VkImage image,
        VkFormat format,
        VkImageLayout old_layout,
        VkImageLayout new_layout
        )
    {
        LNA_ASSERT(renderer._vulkan_device);
        LNA_ASSERT(image);
        (void)format; //TODO: not used for the moment, we will see later if we still need it.

        VkCommandBuffer command_buffer = vulkan_begin_single_time_commands(
            renderer
            );

        VkImageMemoryBarrier barrier{};
        barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout                       = old_layout;
        barrier.newLayout                       = new_layout;
        barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.image                           = image;
        barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel   = 0;
        barrier.subresourceRange.levelCount     = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount     = 1;
        barrier.srcAccessMask                   = 0;
        barrier.dstAccessMask                   = 0;
        
        VkPipelineStageFlags src_stage = 0;
        VkPipelineStageFlags dst_stage = 0;

        if (
                old_layout == VK_IMAGE_LAYOUT_UNDEFINED
            &&  new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
            )
        {
            barrier.srcAccessMask   = 0;
            barrier.dstAccessMask   = VK_ACCESS_TRANSFER_WRITE_BIT;
            src_stage               = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dst_stage               = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (
                old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
            &&  new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            )
        {
            barrier.srcAccessMask   = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
            src_stage               = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dst_stage               = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else
        {
            LNA_ASSERT(0);
        }

        vkCmdPipelineBarrier(
            command_buffer,
            src_stage,
            dst_stage,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &barrier
            );

        vulkan_end_single_time_commands(
            renderer,
            command_buffer
            );
    }

    void vulkan_copy_buffer(
        lna::renderer_vulkan& renderer,
        VkBuffer src,
        VkBuffer dst,
        VkDeviceSize size
        )
    {
        LNA_ASSERT(renderer._vulkan_command_pool);
        LNA_ASSERT(renderer._vulkan_device);
        LNA_ASSERT(renderer._vulkan_graphics_queue);

        VkCommandBuffer command_buffer = vulkan_begin_single_time_commands(
            renderer
            );
        LNA_ASSERT(command_buffer);

        VkBufferCopy copy_region{};
        copy_region.size = size;

        vkCmdCopyBuffer(
            command_buffer,
            src,
            dst,
            1,
            &copy_region
            );

        vulkan_end_single_time_commands(
            renderer,
            command_buffer
            );
    }

    VkShaderModule vulkan_create_shader_module(
        lna::renderer_vulkan& renderer,
        const std::vector<char>& code
        )
    {
        LNA_ASSERT(renderer._vulkan_device);

        VkShaderModuleCreateInfo shader_module_create_info{};
        shader_module_create_info.sType     = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shader_module_create_info.codeSize  = code.size();
        shader_module_create_info.pCode     = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shader_module = nullptr;
        VULKAN_CHECK_RESULT(
            vkCreateShaderModule(
                renderer._vulkan_device,
                &shader_module_create_info,
                nullptr,
                &shader_module
                )
            )
        return shader_module;
    }

    void vulkan_create_buffer(
        lna::renderer_vulkan& renderer,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer& buffer,
        VkDeviceMemory& buffer_memory
        )
    {
        VkBufferCreateInfo buffer_create_info{};
        buffer_create_info.sType        = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.size         = size;
        buffer_create_info.usage        = usage;
        buffer_create_info.sharingMode  = VK_SHARING_MODE_EXCLUSIVE;

        VULKAN_CHECK_RESULT(
            vkCreateBuffer(
                renderer._vulkan_device,
                &buffer_create_info,
                nullptr,
                &buffer
                )
            )

        VkMemoryRequirements memory_requirements;
        vkGetBufferMemoryRequirements(
            renderer._vulkan_device,
            buffer,
            &memory_requirements
            );

        VkMemoryAllocateInfo memory_allocate_info{};
        memory_allocate_info.sType              = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memory_allocate_info.allocationSize     = memory_requirements.size;
        memory_allocate_info.memoryTypeIndex    = vulkan_find_memory_type(
            renderer,
            memory_requirements.memoryTypeBits,
            properties
            );
        
        VULKAN_CHECK_RESULT(
            vkAllocateMemory(
                renderer._vulkan_device,
                &memory_allocate_info,
                nullptr,
                &buffer_memory
                )
            )
        
        VULKAN_CHECK_RESULT(
            vkBindBufferMemory(
                renderer._vulkan_device,
                buffer,
                buffer_memory,
                0
                )
            )
    }

    void vulkan_copy_buffer_to_image(
        lna::renderer_vulkan& renderer,
        VkBuffer buffer,
        VkImage image,
        uint32_t width,
        uint32_t height
        )
    {
        VkCommandBuffer command_buffer = vulkan_begin_single_time_commands(
            renderer
            );

        VkBufferImageCopy region{};
        region.bufferOffset                     = 0;
        region.bufferRowLength                  = 0;
        region.bufferImageHeight                = 0;
        region.imageSubresource.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel        = 0;
        region.imageSubresource.baseArrayLayer  = 0;
        region.imageSubresource.layerCount      = 1;
        region.imageOffset                      = { 0, 0, 0 };
        region.imageExtent                      = { width, height, 1 };

        vkCmdCopyBufferToImage(
            command_buffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
            );

        vulkan_end_single_time_commands(
            renderer,
            command_buffer
            );
    }

    void vulkan_create_image(
        lna::renderer_vulkan& renderer,
        uint32_t width,
        uint32_t height,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkImage& image,
        VkDeviceMemory& image_memory
        )
    {
        VkImageCreateInfo image_create_info{};
        image_create_info.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_create_info.imageType     = VK_IMAGE_TYPE_2D;
        image_create_info.extent.width  = width;
        image_create_info.extent.height = height;
        image_create_info.extent.depth  = 1;
        image_create_info.mipLevels     = 1;
        image_create_info.arrayLayers   = 1;
        image_create_info.format        = format;
        image_create_info.tiling        = tiling;
        image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_create_info.usage         = usage;
        image_create_info.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
        image_create_info.samples       = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.flags         = 0;

        VULKAN_CHECK_RESULT(
            vkCreateImage(
                renderer._vulkan_device,
                &image_create_info,
                nullptr,
                &image
                )
            )

        VkMemoryRequirements memory_requirements;
        vkGetImageMemoryRequirements(
            renderer._vulkan_device,
            image,
            &memory_requirements
            );

        VkMemoryAllocateInfo allocate_info{};
        allocate_info.sType             = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocate_info.allocationSize    = memory_requirements.size;
        allocate_info.memoryTypeIndex   = vulkan_find_memory_type(
            renderer,
            memory_requirements.memoryTypeBits,
            properties
            );

        VULKAN_CHECK_RESULT(
            vkAllocateMemory(
                renderer._vulkan_device,
                &allocate_info,
                nullptr,
                &image_memory
                )
            )

        VULKAN_CHECK_RESULT(
            vkBindImageMemory(
                renderer._vulkan_device,
                image,
                image_memory,
                0
                )
            )
    }

    VkImageView vulkan_create_image_view(
        lna::renderer_vulkan& renderer,
        VkImage image,
        VkFormat format
        )
    {
        LNA_ASSERT(renderer._vulkan_device);
        LNA_ASSERT(image);

        VkImageViewCreateInfo view_create_info{};
        view_create_info.sType                              = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_create_info.image                              = image;
        view_create_info.viewType                           = VK_IMAGE_VIEW_TYPE_2D;
        view_create_info.format                             = format;
        view_create_info.subresourceRange.aspectMask        = VK_IMAGE_ASPECT_COLOR_BIT;
        view_create_info.subresourceRange.baseMipLevel      = 0;
        view_create_info.subresourceRange.levelCount        = 1;
        view_create_info.subresourceRange.baseArrayLayer    = 0;
        view_create_info.subresourceRange.layerCount        = 1;

        VkImageView image_view = nullptr;
        VULKAN_CHECK_RESULT(
            vkCreateImageView(
                renderer._vulkan_device,
                &view_create_info,
                nullptr,
                &image_view
                )
            )

        return image_view;
    }

    //! VULKAN INIT PROCESS FUNCTION -------------------------------------------

    void vulkan_create_instance(
        lna::renderer_vulkan& renderer,
        const lna::renderer_config<lna::window_sdl>& config
        )
    {
        LNA_ASSERT(renderer._vulkan_instance == nullptr);
        LNA_ASSERT(config.window_ptr);

        if (
            config.enable_validation_layers
            && !vulkan_check_validation_layer_support()
            )
        {
            //TODO: find a better way to return error
            LNA_ASSERT(0);
            return;
        }

        VkApplicationInfo application_info {};
        application_info.sType                          = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        application_info.pApplicationName               = config.application_name ? config.application_name : "LNA FRAMEWORK";
        application_info.applicationVersion             = VK_MAKE_VERSION(config.application_major_ver, config.application_minor_ver, config.application_patch_ver);
        application_info.pEngineName                    = config.engine_name ? config.engine_name : "LNA FRAMEWORK";
        application_info.engineVersion                  = VK_MAKE_VERSION(config.engine_major_ver, config.engine_minor_ver, config.engine_patch_ver);

        const std::vector<const char*>& extensions      = lna::window_extensions(*config.window_ptr);

        VkInstanceCreateInfo instance_create_info {};
        instance_create_info.sType                      = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.pApplicationInfo           = &application_info;
        instance_create_info.enabledExtensionCount      = static_cast<uint32_t>(extensions.size());
        instance_create_info.ppEnabledExtensionNames    = extensions.data();

        if (config.enable_validation_layers)
        {
            VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info{};
            debug_messenger_create_info.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debug_messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debug_messenger_create_info.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debug_messenger_create_info.pfnUserCallback = vulkan_debug_callback;
            debug_messenger_create_info.pUserData       = nullptr;
            instance_create_info.enabledLayerCount      = static_cast<uint32_t>(VULKAN_VALIDATION_LAYERS.size());
            instance_create_info.ppEnabledLayerNames    = VULKAN_VALIDATION_LAYERS.data();
            instance_create_info.pNext                  = &debug_messenger_create_info;
        }
        else
        {
            instance_create_info.enabledLayerCount      = 0;
            instance_create_info.ppEnabledLayerNames    = nullptr;
            instance_create_info.pNext                  = nullptr;
        }

        VULKAN_CHECK_RESULT(
            vkCreateInstance(
                &instance_create_info,
                nullptr,
                &renderer._vulkan_instance
                )
            )
    }

    void vulkan_setup_debug_messenger(
        lna::renderer_vulkan& renderer
        )
    {
        LNA_ASSERT(renderer._vulkan_instance);
        LNA_ASSERT(renderer._vulkan_debug_messenger == nullptr);

        VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info{};
        debug_messenger_create_info.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debug_messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debug_messenger_create_info.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debug_messenger_create_info.pfnUserCallback = vulkan_debug_callback;
        debug_messenger_create_info.pUserData       = nullptr;

        VULKAN_CHECK_RESULT(
            vulkan_create_debug_utils_messenger_EXT(
                renderer._vulkan_instance,
                &debug_messenger_create_info,
                nullptr,
                &renderer._vulkan_debug_messenger
                )
            )
    }

    void vulkan_create_surface(
        lna::renderer_vulkan& renderer,
        const lna::renderer_config<lna::window_sdl>& config
        )
    {
        LNA_ASSERT(renderer._vulkan_surface == nullptr);
        LNA_ASSERT(renderer._vulkan_instance);
        LNA_ASSERT(config.window_ptr);

        auto result = SDL_Vulkan_CreateSurface(
            lna::window_handle<lna::window_sdl, SDL_Window*>(*config.window_ptr),
            renderer._vulkan_instance,
            &renderer._vulkan_surface
            );
        LNA_ASSERT(result == SDL_TRUE);
    }

    void vulkan_pick_physical_device(
        lna::renderer_vulkan& renderer
        )
    {
        LNA_ASSERT(renderer._vulkan_instance);
        LNA_ASSERT(renderer._vulkan_physical_device == nullptr);

        uint32_t device_count = 0;
        VULKAN_CHECK_RESULT(
            vkEnumeratePhysicalDevices(
                renderer._vulkan_instance,
                &device_count,
                nullptr
                )
            )
        LNA_ASSERT(device_count > 0);

        std::vector<VkPhysicalDevice> devices(device_count);
        VULKAN_CHECK_RESULT(
            vkEnumeratePhysicalDevices(
                renderer._vulkan_instance,
                &device_count,
                devices.data()
                )
            )
        for (const auto& device : devices)
        {
            if (
                vulkan_is_physical_device_suitable(
                    device,
                    renderer._vulkan_surface
                    )
                )
            {
                renderer._vulkan_physical_device = device;
                break;
            }
        }
    }

    void vulkan_create_logical_device(
        lna::renderer_vulkan& renderer,
        const lna::renderer_config<lna::window_sdl>& config
        )
    {
        LNA_ASSERT(renderer._vulkan_physical_device);
        LNA_ASSERT(renderer._vulkan_device == nullptr);
        LNA_ASSERT(renderer._vulkan_graphics_queue == nullptr);

        vulkan_queue_family_indices             indices = vulkan_find_queue_families(
            renderer._vulkan_physical_device,
            renderer._vulkan_surface
            );
        std::vector<VkDeviceQueueCreateInfo>    queue_create_infos{};
        std::set<uint32_t>                      unique_queue_families =
        {
            indices.graphics_family.value(),
            indices.present_family.value()
        };
        float queue_priority = 1.0f;
        for (uint32_t queue_family : unique_queue_families)
        {
            VkDeviceQueueCreateInfo queue_create_info{};
            queue_create_info.sType             = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex  = queue_family;
            queue_create_info.queueCount        = 1;
            queue_create_info.pQueuePriorities  = &queue_priority;
            queue_create_infos.push_back(queue_create_info);
        }

        VkPhysicalDeviceFeatures device_features{};
        device_features.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo device_create_info{};
        device_create_info.sType                    = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.pQueueCreateInfos        = queue_create_infos.data();
        device_create_info.queueCreateInfoCount     = static_cast<uint32_t>(queue_create_infos.size());
        device_create_info.pEnabledFeatures         = &device_features;
        device_create_info.enabledExtensionCount    = static_cast<uint32_t>(VULKAN_DEVICE_EXTENSIONS.size());
        device_create_info.ppEnabledExtensionNames  = VULKAN_DEVICE_EXTENSIONS.data();
        device_create_info.enabledLayerCount        = config.enable_validation_layers ? static_cast<uint32_t>(VULKAN_VALIDATION_LAYERS.size()) : 0;
        device_create_info.ppEnabledLayerNames      = config.enable_validation_layers ? VULKAN_VALIDATION_LAYERS.data() : nullptr;

        VULKAN_CHECK_RESULT(
            vkCreateDevice(
                renderer._vulkan_physical_device,
                &device_create_info,
                nullptr,
                &renderer._vulkan_device
                )
            )
        if (renderer._vulkan_device)
        {
            vkGetDeviceQueue(
                renderer._vulkan_device,
                indices.graphics_family.value(),
                0,
                &renderer._vulkan_graphics_queue
                );
            vkGetDeviceQueue(
                renderer._vulkan_device,
                indices.present_family.value(),
                0,
                &renderer._vulkan_present_queue
                );
        }
    }

    void vulkan_create_swap_chain(
        lna::renderer_vulkan& renderer,
        uint32_t framebuffer_width,
        uint32_t framebuffer_height
        )
    {
        LNA_ASSERT(renderer._vulkan_device);
        LNA_ASSERT(renderer._vulkan_swap_chain == nullptr);

        vulkan_swap_chain_support_details   swap_chain_support  = vulkan_query_swap_chain_support(renderer._vulkan_physical_device, renderer._vulkan_surface);
        VkSurfaceFormatKHR                  surface_format      = vulkan_choose_swap_surface_format(swap_chain_support.formats);
        VkPresentModeKHR                    present_mode        = vulkan_choose_swap_present_mode(swap_chain_support.present_modes);
        VkExtent2D                          extent              = vulkan_choose_swap_extent(swap_chain_support.capabilities, framebuffer_width, framebuffer_height);

        // simply sticking to this minimum means that we may sometimes have to wait on the driver to complete internal operations before we can acquire another image to render to.
        // Therefore it is recommended to request at least one more image than the minimum:
        uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
        if (
            swap_chain_support.capabilities.maxImageCount > 0
            && image_count > swap_chain_support.capabilities.maxImageCount
            )
        {
            // do not exceed the maximum number of image:
            image_count = swap_chain_support.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR swap_chain_create_info{};
        swap_chain_create_info.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swap_chain_create_info.surface          = renderer._vulkan_surface;
        swap_chain_create_info.minImageCount    = image_count;
        swap_chain_create_info.imageFormat      = surface_format.format;
        swap_chain_create_info.imageColorSpace  = surface_format.colorSpace;
        swap_chain_create_info.imageExtent      = extent;
        swap_chain_create_info.imageArrayLayers = 1;
        swap_chain_create_info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        vulkan_queue_family_indices indices = vulkan_find_queue_families(renderer._vulkan_physical_device, renderer._vulkan_surface);
        uint32_t queue_family_indices[]     =
        {
            indices.graphics_family.value(),
            indices.present_family.value()
        };
        if (indices.graphics_family != indices.present_family)
        {
            swap_chain_create_info.imageSharingMode         = VK_SHARING_MODE_CONCURRENT;
            swap_chain_create_info.queueFamilyIndexCount    = 2;
            swap_chain_create_info.pQueueFamilyIndices      = queue_family_indices;
        }
        else
        {
            swap_chain_create_info.imageSharingMode         = VK_SHARING_MODE_EXCLUSIVE;
            swap_chain_create_info.queueFamilyIndexCount    = 0;
            swap_chain_create_info.pQueueFamilyIndices      = nullptr;
        }
        swap_chain_create_info.preTransform     = swap_chain_support.capabilities.currentTransform;
        swap_chain_create_info.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swap_chain_create_info.presentMode      = present_mode;
        swap_chain_create_info.clipped          = VK_TRUE;
        swap_chain_create_info.oldSwapchain     = VK_NULL_HANDLE;
        VULKAN_CHECK_RESULT(
            vkCreateSwapchainKHR(
                renderer._vulkan_device,
                &swap_chain_create_info,
                nullptr,
                &renderer._vulkan_swap_chain
                )
            )
        VULKAN_CHECK_RESULT(
            vkGetSwapchainImagesKHR(
                renderer._vulkan_device,
                renderer._vulkan_swap_chain,
                &image_count,
                nullptr
                )
            )
        renderer._vulkan_swap_chain_images.resize(image_count);
        VULKAN_CHECK_RESULT(
            vkGetSwapchainImagesKHR(
                renderer._vulkan_device,
                renderer._vulkan_swap_chain,
                &image_count,
                renderer._vulkan_swap_chain_images.data()
                )
            )
        renderer._vulkan_swap_chain_image_format    = surface_format.format;
        renderer._vulkan_swap_chain_extent          = extent;
    }

    void vulkan_create_image_views(
        lna::renderer_vulkan& renderer
        )
    {
        renderer._vulkan_swap_chain_image_views.resize(renderer._vulkan_swap_chain_images.size());
        for (size_t i = 0; i < renderer._vulkan_swap_chain_images.size(); ++i)
        {
            renderer._vulkan_swap_chain_image_views[i] = vulkan_create_image_view(
                renderer,
                renderer._vulkan_swap_chain_images[i],
                renderer._vulkan_swap_chain_image_format
                );
        }
    }

    void vulkan_create_render_pass(
        lna::renderer_vulkan& renderer
        )
    {
        LNA_ASSERT(renderer._vulkan_device);

        VkAttachmentDescription color_attachment{};
        color_attachment.format                     = renderer._vulkan_swap_chain_image_format;
        color_attachment.samples                    = VK_SAMPLE_COUNT_1_BIT;
        color_attachment.loadOp                     = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment.storeOp                    = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment.stencilLoadOp              = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp             = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment.initialLayout              = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment.finalLayout                = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference color_attachment_reference{};
        color_attachment_reference.attachment       = 0;
        color_attachment_reference.layout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass_description{};
        subpass_description.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_description.colorAttachmentCount    = 1;
        subpass_description.pColorAttachments       = &color_attachment_reference;

        VkSubpassDependency subpass_dependancy{};
        subpass_dependancy.srcSubpass               = VK_SUBPASS_EXTERNAL;
        subpass_dependancy.srcStageMask             = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependancy.srcAccessMask            = 0;
        subpass_dependancy.dstSubpass               = 0;
        subpass_dependancy.dstStageMask             = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependancy.dstAccessMask            = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo render_pass_create_info{};
        render_pass_create_info.sType               = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_create_info.attachmentCount     = 1;
        render_pass_create_info.pAttachments        = &color_attachment;
        render_pass_create_info.subpassCount        = 1;
        render_pass_create_info.pSubpasses          = &subpass_description;
        render_pass_create_info.dependencyCount     = 1;
        render_pass_create_info.pDependencies       = &subpass_dependancy;

        VULKAN_CHECK_RESULT(
            vkCreateRenderPass(
                renderer._vulkan_device,
                &render_pass_create_info,
                nullptr,
                &renderer._vulkan_render_pass
                )
            )
    }

    void vulkan_create_descriptor_set_layout(
        lna::renderer_vulkan& renderer
        )
    {
        LNA_ASSERT(renderer._vulkan_device);

        VkDescriptorSetLayoutBinding ubo_layout_binding{};
        ubo_layout_binding.binding                  = 0;
        ubo_layout_binding.descriptorCount          = 1;
        ubo_layout_binding.descriptorType           = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ubo_layout_binding.stageFlags               = VK_SHADER_STAGE_VERTEX_BIT;
        ubo_layout_binding.pImmutableSamplers       = nullptr;

        VkDescriptorSetLayoutBinding sampler_layout_binding{};
        sampler_layout_binding.binding              = 1;
        sampler_layout_binding.descriptorCount      = 1;
        sampler_layout_binding.descriptorType       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        sampler_layout_binding.stageFlags           = VK_SHADER_STAGE_FRAGMENT_BIT;
        sampler_layout_binding.pImmutableSamplers   = nullptr;

        std::array<VkDescriptorSetLayoutBinding, 2> bindings =
        {
            ubo_layout_binding,
            sampler_layout_binding,
        };

        VkDescriptorSetLayoutCreateInfo layout_create_info{};
        layout_create_info.sType                    = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_create_info.bindingCount             = static_cast<uint32_t>(bindings.size());
        layout_create_info.pBindings                = bindings.data();

        VULKAN_CHECK_RESULT(
            vkCreateDescriptorSetLayout(
                renderer._vulkan_device,
                &layout_create_info,
                nullptr,
                &renderer._vulkan_descriptor_set_layout
                )
            )
    }

    void vulkan_create_graphics_pipeline(
        lna::renderer_vulkan& renderer
        )
    {
        LNA_ASSERT(renderer._vulkan_device);
        LNA_ASSERT(renderer._vulkan_render_pass);

        VkShaderModule vertex_shader_module = vulkan_create_shader_module(
            renderer,
            lna::file_debug_load("shaders/default_vert.spv", true) // TODO: remove when we will have the file system and shader "manager"
            );
        VkShaderModule fragment_shader_module = vulkan_create_shader_module(
            renderer,
            lna::file_debug_load("shaders/default_frag.spv", true) // TODO: remove when we will have the file system and shader "manager"
            );

        VkPipelineShaderStageCreateInfo vertex_shader_stage_create_info{};
        vertex_shader_stage_create_info.sType       = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertex_shader_stage_create_info.stage       = VK_SHADER_STAGE_VERTEX_BIT;
        vertex_shader_stage_create_info.module      = vertex_shader_module;
        vertex_shader_stage_create_info.pName       = "main";

        VkPipelineShaderStageCreateInfo fragment_shader_stage_create_info{};
        fragment_shader_stage_create_info.sType     = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragment_shader_stage_create_info.stage     = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragment_shader_stage_create_info.module    = fragment_shader_module;
        fragment_shader_stage_create_info.pName     = "main";

        VkPipelineShaderStageCreateInfo shader_stage_create_infos[] =
        {
            vertex_shader_stage_create_info,
            fragment_shader_stage_create_info
        };

        auto vertex_binding_description     = vertex::binding_description();
        auto vertex_attribute_descriptions  = vertex::attribute_descriptions();

        VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
        vertex_input_state_create_info.sType                            = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_state_create_info.vertexBindingDescriptionCount    = 1;
        vertex_input_state_create_info.pVertexBindingDescriptions       = &vertex_binding_description;
        vertex_input_state_create_info.vertexAttributeDescriptionCount  = static_cast<uint32_t>(vertex_attribute_descriptions.size());
        vertex_input_state_create_info.pVertexAttributeDescriptions     = vertex_attribute_descriptions.data();

        VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info{};
        input_assembly_state_create_info.sType                          = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly_state_create_info.topology                       = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly_state_create_info.primitiveRestartEnable         = VK_FALSE;

        VkViewport viewport{};
        viewport.x          = 0.0f;
        viewport.y          = 0.0f;
        viewport.width      = static_cast<float>(renderer._vulkan_swap_chain_extent.width);
        viewport.height     = static_cast<float>(renderer._vulkan_swap_chain_extent.height);
        viewport.minDepth   = 0.0f;
        viewport.maxDepth   = 1.0f;

        VkRect2D scissor{};
        scissor.offset      = { 0, 0 };
        scissor.extent      = renderer._vulkan_swap_chain_extent;

        VkPipelineViewportStateCreateInfo viewport_state_create_info{};
        viewport_state_create_info.sType            = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state_create_info.viewportCount    = 1;
        viewport_state_create_info.pViewports       = &viewport;
        viewport_state_create_info.scissorCount     = 1;
        viewport_state_create_info.pScissors        = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterization_state_create_info{};
        rasterization_state_create_info.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterization_state_create_info.depthClampEnable        = VK_FALSE;
        rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
        rasterization_state_create_info.polygonMode             = VK_POLYGON_MODE_FILL;
        rasterization_state_create_info.lineWidth               = 1.0f;
        rasterization_state_create_info.cullMode                = VK_CULL_MODE_BACK_BIT;
        rasterization_state_create_info.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterization_state_create_info.depthBiasEnable         = VK_FALSE;
        rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
        rasterization_state_create_info.depthBiasClamp          = 0.0f;
        rasterization_state_create_info.depthBiasSlopeFactor    = 0.0f;

        VkPipelineMultisampleStateCreateInfo multisample_state_create_info{};
        multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample_state_create_info.sampleShadingEnable   = VK_FALSE;
        multisample_state_create_info.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
        multisample_state_create_info.minSampleShading      = 1.0f;
        multisample_state_create_info.pSampleMask           = nullptr;
        multisample_state_create_info.alphaToCoverageEnable = VK_FALSE;
        multisample_state_create_info.alphaToOneEnable      = VK_FALSE;

        VkPipelineColorBlendAttachmentState color_blend_attachment_state{};
        color_blend_attachment_state.colorWriteMask =
              VK_COLOR_COMPONENT_R_BIT
            | VK_COLOR_COMPONENT_G_BIT
            | VK_COLOR_COMPONENT_B_BIT
            | VK_COLOR_COMPONENT_A_BIT
            ;
        color_blend_attachment_state.blendEnable            = VK_FALSE;
        color_blend_attachment_state.srcColorBlendFactor    = VK_BLEND_FACTOR_ONE;
        color_blend_attachment_state.dstColorBlendFactor    = VK_BLEND_FACTOR_ZERO;
        color_blend_attachment_state.colorBlendOp           = VK_BLEND_OP_ADD;
        color_blend_attachment_state.srcAlphaBlendFactor    = VK_BLEND_FACTOR_ONE;
        color_blend_attachment_state.dstAlphaBlendFactor    = VK_BLEND_FACTOR_ZERO;
        color_blend_attachment_state.alphaBlendOp           = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo color_blender_state_create_info{};
        color_blender_state_create_info.sType               = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blender_state_create_info.logicOpEnable       = VK_FALSE;
        color_blender_state_create_info.logicOp             = VK_LOGIC_OP_COPY;
        color_blender_state_create_info.attachmentCount     = 1;
        color_blender_state_create_info.pAttachments        = &color_blend_attachment_state;
        color_blender_state_create_info.blendConstants[0]   = 0.0f;
        color_blender_state_create_info.blendConstants[1]   = 0.0f;
        color_blender_state_create_info.blendConstants[2]   = 0.0f;
        color_blender_state_create_info.blendConstants[3]   = 0.0f;

        VkDynamicState dynamic_states[] =
        {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_LINE_WIDTH
        };

        VkPipelineDynamicStateCreateInfo dynamic_state_create_info{};
        dynamic_state_create_info.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state_create_info.dynamicStateCount = 2;
        dynamic_state_create_info.pDynamicStates    = dynamic_states;

        VkPipelineLayoutCreateInfo  layout_create_info{};
        layout_create_info.sType                    = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layout_create_info.setLayoutCount           = 1;
        layout_create_info.pSetLayouts              = &renderer._vulkan_descriptor_set_layout;
        layout_create_info.pushConstantRangeCount   = 0;
        layout_create_info.pPushConstantRanges      = nullptr;

        VULKAN_CHECK_RESULT(
            vkCreatePipelineLayout(
                renderer._vulkan_device,
                &layout_create_info,
                nullptr,
                &renderer._vulkan_pipeline_layout
                )
            )

        VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{};
        graphics_pipeline_create_info.sType                 = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        graphics_pipeline_create_info.stageCount            = 2;
        graphics_pipeline_create_info.pStages               = shader_stage_create_infos;
        graphics_pipeline_create_info.pVertexInputState     = &vertex_input_state_create_info;
        graphics_pipeline_create_info.pInputAssemblyState   = &input_assembly_state_create_info;
        graphics_pipeline_create_info.pViewportState        = &viewport_state_create_info;
        graphics_pipeline_create_info.pRasterizationState   = &rasterization_state_create_info;
        graphics_pipeline_create_info.pMultisampleState     = &multisample_state_create_info;
        graphics_pipeline_create_info.pDepthStencilState    = nullptr;
        graphics_pipeline_create_info.pColorBlendState      = &color_blender_state_create_info;
        graphics_pipeline_create_info.pDynamicState         = nullptr;
        graphics_pipeline_create_info.layout                = renderer._vulkan_pipeline_layout;
        graphics_pipeline_create_info.renderPass            = renderer._vulkan_render_pass;
        graphics_pipeline_create_info.subpass               = 0;
        graphics_pipeline_create_info.basePipelineHandle    = VK_NULL_HANDLE;
        graphics_pipeline_create_info.basePipelineIndex     = -1;

        VULKAN_CHECK_RESULT(
            vkCreateGraphicsPipelines(
                renderer._vulkan_device,
                VK_NULL_HANDLE,
                1,
                &graphics_pipeline_create_info,
                nullptr,
                &renderer._vulkan_graphics_pipeline
                )
            )

        vkDestroyShaderModule(renderer._vulkan_device, fragment_shader_module, nullptr);
        vkDestroyShaderModule(renderer._vulkan_device, vertex_shader_module, nullptr);
    }

    void vulkan_create_framebuffers(
        lna::renderer_vulkan& renderer
        )
    {
        LNA_ASSERT(renderer._vulkan_device);

        renderer._vulkan_swap_chain_framebuffers.resize(renderer._vulkan_swap_chain_image_views.size());
        for (size_t i = 0; i < renderer._vulkan_swap_chain_image_views.size(); ++i)
        {
            VkImageView attachments[] =
            {
                renderer._vulkan_swap_chain_image_views[i]
            };

            VkFramebufferCreateInfo framebuffer_create_info{};
            framebuffer_create_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_create_info.renderPass      = renderer._vulkan_render_pass;
            framebuffer_create_info.attachmentCount = 1;
            framebuffer_create_info.pAttachments    = attachments;
            framebuffer_create_info.width           = renderer._vulkan_swap_chain_extent.width;
            framebuffer_create_info.height          = renderer._vulkan_swap_chain_extent.height;
            framebuffer_create_info.layers          = 1;

            VULKAN_CHECK_RESULT(
                vkCreateFramebuffer(
                    renderer._vulkan_device,
                    &framebuffer_create_info,
                    nullptr,
                    &renderer._vulkan_swap_chain_framebuffers[i]
                    )
                )
        }
    }

    void vulkan_create_command_pool(
        lna::renderer_vulkan& renderer
        )
    {
        LNA_ASSERT(renderer._vulkan_device);

        vulkan_queue_family_indices indices = vulkan_find_queue_families(renderer._vulkan_physical_device, renderer._vulkan_surface);

        VkCommandPoolCreateInfo command_pool_create_info{};
        command_pool_create_info.sType              = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_create_info.queueFamilyIndex   = indices.graphics_family.value();
        command_pool_create_info.flags              = 0;

        VULKAN_CHECK_RESULT(
            vkCreateCommandPool(
                renderer._vulkan_device,
                &command_pool_create_info,
                nullptr,
                &renderer._vulkan_command_pool
                )
            )
    }

    void vulkan_create_command_buffers(
        lna::renderer_vulkan& renderer
        )
    {
        LNA_ASSERT(renderer._vulkan_device);

        renderer._vulkan_command_buffers.resize(renderer._vulkan_swap_chain_framebuffers.size());

        VkCommandBufferAllocateInfo command_buffer_allocate_info{}; 
        command_buffer_allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_allocate_info.commandPool        = renderer._vulkan_command_pool;
        command_buffer_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        command_buffer_allocate_info.commandBufferCount = static_cast<uint32_t>(renderer._vulkan_command_buffers.size());

        VULKAN_CHECK_RESULT(
            vkAllocateCommandBuffers(
                renderer._vulkan_device,
                &command_buffer_allocate_info,
                renderer._vulkan_command_buffers.data()
                )
            )

        for (size_t i = 0; i < renderer._vulkan_command_buffers.size(); ++i)
        {
            VkCommandBufferBeginInfo    command_buffer_begin_info{};
            command_buffer_begin_info.sType             = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            command_buffer_begin_info.flags             = 0;
            command_buffer_begin_info.pInheritanceInfo  = nullptr;

            VULKAN_CHECK_RESULT(
                vkBeginCommandBuffer(
                    renderer._vulkan_command_buffers[i],
                    &command_buffer_begin_info
                    )
                )

            VkRenderPassBeginInfo render_pass_begin_info{};
            render_pass_begin_info.sType                = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            render_pass_begin_info.renderPass           = renderer._vulkan_render_pass;
            render_pass_begin_info.framebuffer          = renderer._vulkan_swap_chain_framebuffers[i];
            render_pass_begin_info.renderArea.offset    = { 0, 0 };
            render_pass_begin_info.renderArea.extent    = renderer._vulkan_swap_chain_extent;
            VkClearValue clear_color = {{{ 0.0f, 0.0f, 0.0f, 1.0f }}};
            render_pass_begin_info.clearValueCount      = 1;
            render_pass_begin_info.pClearValues         = &clear_color;

            vkCmdBeginRenderPass(
                renderer._vulkan_command_buffers[i],
                &render_pass_begin_info,
                VK_SUBPASS_CONTENTS_INLINE
                );
            vkCmdBindPipeline(
                renderer._vulkan_command_buffers[i],
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                renderer._vulkan_graphics_pipeline
                );
            VkBuffer        vertex_buffers[]    = { renderer._vulkan_vertex_buffer };
            VkDeviceSize    offsets[]           = { 0 };
            vkCmdBindVertexBuffers(
                renderer._vulkan_command_buffers[i],
                0,
                1,
                vertex_buffers,
                offsets
                );
            vkCmdBindIndexBuffer(
                renderer._vulkan_command_buffers[i],
                renderer._vulkan_index_buffer,
                0,
                VK_INDEX_TYPE_UINT16
                );
            vkCmdBindDescriptorSets(
                renderer._vulkan_command_buffers[i],
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                renderer._vulkan_pipeline_layout,
                0,
                1,
                &renderer._vulkan_descriptor_sets[i],
                0,
                nullptr
                );
            vkCmdDrawIndexed(
                renderer._vulkan_command_buffers[i],
                static_cast<uint32_t>(INDICES.size()),
                1,
                0,
                0,
                0
                );
            vkCmdEndRenderPass(
                renderer._vulkan_command_buffers[i]
                );

            VULKAN_CHECK_RESULT(
                vkEndCommandBuffer(
                    renderer._vulkan_command_buffers[i]
                    )
                )
        }
    }

    void vulkan_create_sync_objects(
        lna::renderer_vulkan& renderer
        )
    {
        LNA_ASSERT(renderer._vulkan_device);

        renderer._vulkan_image_available_semaphores.resize(VULKAN_MAX_FRAMES_IN_FLIGHT);
        renderer._vulkan_render_finished_semaphores.resize(VULKAN_MAX_FRAMES_IN_FLIGHT);
        renderer._vulkan_in_flight_fences.resize(VULKAN_MAX_FRAMES_IN_FLIGHT);
        renderer._vulkan_images_in_flight_fences.resize(renderer._vulkan_swap_chain_images.size(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphore_create_info{};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fence_create_info{};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < VULKAN_MAX_FRAMES_IN_FLIGHT; ++i)
        {
            VULKAN_CHECK_RESULT(
                vkCreateSemaphore(
                    renderer._vulkan_device,
                    &semaphore_create_info,
                    nullptr,
                    &renderer._vulkan_image_available_semaphores[i]
                    )
                )
            VULKAN_CHECK_RESULT(
                vkCreateSemaphore(
                    renderer._vulkan_device,
                    &semaphore_create_info,
                    nullptr,
                    &renderer._vulkan_render_finished_semaphores[i]
                    )
                )
            VULKAN_CHECK_RESULT(
                vkCreateFence(
                    renderer._vulkan_device,
                    &fence_create_info,
                    nullptr,
                    &renderer._vulkan_in_flight_fences[i]
                    )
                )
        }
    }

    void vulkan_create_vertex_buffer(
        lna::renderer_vulkan& renderer
        )
    {
        LNA_ASSERT(renderer._vulkan_device);

        VkDeviceSize buffer_size = sizeof(VERTICES[0]) * VERTICES.size();

        VkBuffer staging_buffer;
        VkDeviceMemory staging_buffer_memory;
        vulkan_create_buffer(
            renderer,
            buffer_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            staging_buffer,
            staging_buffer_memory
            );

        void* data;
        VULKAN_CHECK_RESULT(
            vkMapMemory(
                renderer._vulkan_device,
                staging_buffer_memory,
                0,
                buffer_size,
                0,
                &data
                )
            )
        memcpy(
            data,
            VERTICES.data(),
            static_cast<size_t>(buffer_size)
            );
        vkUnmapMemory(
            renderer._vulkan_device,
            staging_buffer_memory
            );

        vulkan_create_buffer(
            renderer,
            buffer_size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            renderer._vulkan_vertex_buffer,
            renderer._vulkan_vertex_buffer_memory
            );

        vulkan_copy_buffer(
            renderer,
            staging_buffer,
            renderer._vulkan_vertex_buffer,
            buffer_size
            );

        vkDestroyBuffer(
            renderer._vulkan_device,
            staging_buffer,
            nullptr
            );
        vkFreeMemory(
            renderer._vulkan_device,
            staging_buffer_memory,
            nullptr
            );
    }

    void vulkan_create_index_buffer(
        lna::renderer_vulkan& renderer
        )
    {
        LNA_ASSERT(renderer._vulkan_device);

        VkDeviceSize buffer_size = sizeof(INDICES[0]) * INDICES.size();

        VkBuffer staging_buffer;
        VkDeviceMemory staging_buffer_memory;
        vulkan_create_buffer(
            renderer,
            buffer_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            staging_buffer,
            staging_buffer_memory
            );

        void* data;
        VULKAN_CHECK_RESULT(
            vkMapMemory(
                renderer._vulkan_device,
                staging_buffer_memory,
                0,
                buffer_size,
                0,
                &data
                )
            )
        memcpy(
            data,
            INDICES.data(),
            static_cast<size_t>(buffer_size)
            );
        vkUnmapMemory(
            renderer._vulkan_device,
            staging_buffer_memory
            );

        vulkan_create_buffer(
            renderer,
            buffer_size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            renderer._vulkan_index_buffer,
            renderer._vulkan_index_buffer_memory
            );

        vulkan_copy_buffer(
            renderer,
            staging_buffer,
            renderer._vulkan_index_buffer,
            buffer_size
            );

        vkDestroyBuffer(
            renderer._vulkan_device,
            staging_buffer,
            nullptr
            );
        vkFreeMemory(
            renderer._vulkan_device,
            staging_buffer_memory,
            nullptr
            );
    }

    void vulkan_create_uniform_buffers(
        lna::renderer_vulkan& renderer
        )
    {
        LNA_ASSERT(renderer._vulkan_device);

        VkDeviceSize buffer_size = sizeof(uniform_buffer_object);
        
        renderer._vulkan_uniform_buffers.resize(renderer._vulkan_swap_chain_images.size());
        renderer._vulkan_uniform_buffers_memory.resize(renderer._vulkan_swap_chain_images.size());

        for (size_t i = 0; i < renderer._vulkan_swap_chain_images.size(); ++i)
        {
            vulkan_create_buffer(
                renderer,
                buffer_size,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                renderer._vulkan_uniform_buffers[i],
                renderer._vulkan_uniform_buffers_memory[i]
                );
        }
    }

    void vulkan_update_uniform_buffer(
        lna::renderer_vulkan& renderer,
        uint32_t image_index
        )
    {
        uniform_buffer_object ubo{};

        const lna::vec3 eye     = { 0.0f, 0.0f, 2.0f };
        const lna::vec3 target  = { 0.0f, 0.0f, 0.0f };
        const lna::vec3 up      = { 0.0f, -1.0f, 0.0f };
        const float     fov     = 45.0f;
        const float     aspect  = static_cast<float>(renderer._vulkan_swap_chain_extent.width) / static_cast<float>(renderer._vulkan_swap_chain_extent.height);
        const float     near    = 1.0f;
        const float     far     = 10.0f;

        lna::mat4_identity(
            ubo.model
            );
        lna::mat4_loot_at(
            ubo.view,
            eye,
            target,
            up
            );
        lna::mat4_perspective(
            ubo.projection,
            fov,
            aspect,
            near,
            far
            );

        void* data;
        VULKAN_CHECK_RESULT(
            vkMapMemory(
                renderer._vulkan_device,
                renderer._vulkan_uniform_buffers_memory[image_index],
                0,
                sizeof(ubo),
                0,
                &data
                )
            )
        memcpy(
            data,
            &ubo,
            sizeof(ubo)
            );
        vkUnmapMemory(
            renderer._vulkan_device,
            renderer._vulkan_uniform_buffers_memory[image_index]
            );
    }

    void vulkan_create_descriptor_pool(
        lna::renderer_vulkan& renderer
        )
    {
        LNA_ASSERT(renderer._vulkan_device);
        LNA_ASSERT(renderer._vulkan_descriptor_pool == nullptr);

        std::array<VkDescriptorPoolSize, 2> pool_sizes{};
        pool_sizes[0].type              = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        pool_sizes[0].descriptorCount   = static_cast<uint32_t>(renderer._vulkan_swap_chain_images.size());
        pool_sizes[1].type              = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        pool_sizes[1].descriptorCount   = static_cast<uint32_t>(renderer._vulkan_swap_chain_images.size());

        VkDescriptorPoolCreateInfo pool_create_info{};
        pool_create_info.sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_create_info.poolSizeCount  = static_cast<uint32_t>(pool_sizes.size());
        pool_create_info.pPoolSizes     = pool_sizes.data();
        pool_create_info.maxSets        = static_cast<uint32_t>(renderer._vulkan_swap_chain_images.size());

        VULKAN_CHECK_RESULT(
            vkCreateDescriptorPool(
                renderer._vulkan_device,
                &pool_create_info,
                nullptr,
                &renderer._vulkan_descriptor_pool
                )
            )
    }

    void vulkan_create_descriptor_sets(
        lna::renderer_vulkan& renderer
        )
    {
        LNA_ASSERT(renderer._vulkan_device);

        std::vector<VkDescriptorSetLayout> layouts(renderer._vulkan_swap_chain_images.size(), renderer._vulkan_descriptor_set_layout);

        VkDescriptorSetAllocateInfo allocate_info{};
        allocate_info.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocate_info.descriptorPool        = renderer._vulkan_descriptor_pool;
        allocate_info.descriptorSetCount    = static_cast<uint32_t>(renderer._vulkan_swap_chain_images.size());
        allocate_info.pSetLayouts           = layouts.data();

        renderer._vulkan_descriptor_sets.resize(renderer._vulkan_swap_chain_images.size());
        
        VULKAN_CHECK_RESULT(
            vkAllocateDescriptorSets(
                renderer._vulkan_device,
                &allocate_info,
                renderer._vulkan_descriptor_sets.data()
                )
            )

        for (size_t i = 0; i < renderer._vulkan_swap_chain_images.size(); ++i)
        {
            VkDescriptorBufferInfo buffer_info{};
            buffer_info.buffer  = renderer._vulkan_uniform_buffers[i];
            buffer_info.offset  = 0;
            buffer_info.range   = sizeof(uniform_buffer_object);

            VkDescriptorImageInfo image_info{};
            image_info.imageLayout  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            image_info.imageView    = renderer._vulkan_texture_image_view;
            image_info.sampler      = renderer._vulkan_texture_sampler;

            std::array<VkWriteDescriptorSet, 2> write_descriptors{};
            write_descriptors[0].sType              = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_descriptors[0].dstSet             = renderer._vulkan_descriptor_sets[i];
            write_descriptors[0].dstBinding         = 0;
            write_descriptors[0].dstArrayElement    = 0;
            write_descriptors[0].descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            write_descriptors[0].descriptorCount    = 1;
            write_descriptors[0].pBufferInfo        = &buffer_info;
            write_descriptors[0].pImageInfo         = nullptr;
            write_descriptors[0].pTexelBufferView   = nullptr;
            write_descriptors[1].sType              = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_descriptors[1].dstSet             = renderer._vulkan_descriptor_sets[i];
            write_descriptors[1].dstBinding         = 1;
            write_descriptors[1].dstArrayElement    = 0;
            write_descriptors[1].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write_descriptors[1].descriptorCount    = 1;
            write_descriptors[1].pBufferInfo        = nullptr;
            write_descriptors[1].pImageInfo         = &image_info;
            write_descriptors[1].pTexelBufferView   = nullptr;

            vkUpdateDescriptorSets(
                renderer._vulkan_device,
                static_cast<uint32_t>(write_descriptors.size()),
                write_descriptors.data(),
                0,
                nullptr
                );
        }
    }

    void vulkan_create_texture_image(
        lna::renderer_vulkan& renderer
        )
    {
        LNA_ASSERT(renderer._vulkan_device);
        LNA_ASSERT(renderer._vulkan_texture_image == nullptr);
        LNA_ASSERT(renderer._vulkan_texture_image_memory == nullptr);

        int         texture_width;
        int         texture_height;
        int         texture_channels;
        stbi_uc*    pixels = stbi_load(
            "textures/texture.jpg",
            &texture_width,
            &texture_height,
            &texture_channels,
            STBI_rgb_alpha
            );
        VkDeviceSize image_size = texture_width * texture_height * 4;
        LNA_ASSERT(pixels);

        VkBuffer        staging_buffer;
        VkDeviceMemory  staging_buffer_memory;

        vulkan_create_buffer(
            renderer,
            image_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            staging_buffer,
            staging_buffer_memory
            );

        void* data;
        VULKAN_CHECK_RESULT(
            vkMapMemory(
                renderer._vulkan_device,
                staging_buffer_memory,
                0,
                image_size,
                0,
                &data
                )
            )
        memcpy(
            data,
            pixels,
            static_cast<size_t>(image_size)
            );
        vkUnmapMemory(
            renderer._vulkan_device,
            staging_buffer_memory
            );

        stbi_image_free(pixels);

        vulkan_create_image(
            renderer,
            static_cast<uint32_t>(texture_width),
            static_cast<uint32_t>(texture_height),
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            renderer._vulkan_texture_image,
            renderer._vulkan_texture_image_memory
            );

        vulkan_transition_image_layout(
            renderer,
            renderer._vulkan_texture_image,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
            );
        vulkan_copy_buffer_to_image(
            renderer,
            staging_buffer,
            renderer._vulkan_texture_image,
            static_cast<uint32_t>(texture_width),
            static_cast<uint32_t>(texture_height)
            );
        vulkan_transition_image_layout(
            renderer,
            renderer._vulkan_texture_image,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            );
        vkDestroyBuffer(
            renderer._vulkan_device,
            staging_buffer,
            nullptr
            );
        vkFreeMemory(
            renderer._vulkan_device,
            staging_buffer_memory,
            nullptr
            );
    }

    void vulkan_create_texture_image_view(
        lna::renderer_vulkan& renderer
        )
    {
        LNA_ASSERT(renderer._vulkan_device);
        LNA_ASSERT(renderer._vulkan_texture_image_view == nullptr);

        renderer._vulkan_texture_image_view = vulkan_create_image_view(
            renderer,
            renderer._vulkan_texture_image,
            VK_FORMAT_R8G8B8A8_SRGB
            );
    }

    void vulkan_create_texture_sampler(
        lna::renderer_vulkan& renderer
        )
    {
        LNA_ASSERT(renderer._vulkan_device);
        LNA_ASSERT(renderer._vulkan_physical_device);
        LNA_ASSERT(renderer._vulkan_texture_sampler == nullptr);

        VkPhysicalDeviceProperties gpu_properties{};
        vkGetPhysicalDeviceProperties(
            renderer._vulkan_physical_device,
            &gpu_properties
            );

        VkSamplerCreateInfo sampler_create_info{};
        sampler_create_info.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_create_info.magFilter               = VK_FILTER_LINEAR;
        sampler_create_info.minFilter               = VK_FILTER_LINEAR;
        sampler_create_info.addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_create_info.addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_create_info.addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_create_info.anisotropyEnable        = VK_TRUE;
        sampler_create_info.maxAnisotropy           = gpu_properties.limits.maxSamplerAnisotropy;
        sampler_create_info.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        sampler_create_info.unnormalizedCoordinates = VK_FALSE;
        sampler_create_info.compareEnable           = VK_FALSE;
        sampler_create_info.compareOp               = VK_COMPARE_OP_ALWAYS;
        sampler_create_info.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler_create_info.mipLodBias              = 0.0f;
        sampler_create_info.minLod                  = 0.0f;
        sampler_create_info.maxLod                  = 0.0f;

        
        VULKAN_CHECK_RESULT(
            vkCreateSampler(
                renderer._vulkan_device,
                &sampler_create_info,
                nullptr,
                &renderer._vulkan_texture_sampler
                )
            )
    }

    void vulkan_cleanup_swap_chain(
        lna::renderer_vulkan& renderer
        )
    {
        LNA_ASSERT(renderer._vulkan_device);

        for (auto framebuffer : renderer._vulkan_swap_chain_framebuffers)
        {
            vkDestroyFramebuffer(
                renderer._vulkan_device,
                framebuffer,
                nullptr
                );
        }
        vkFreeCommandBuffers(
            renderer._vulkan_device,
            renderer._vulkan_command_pool,
            static_cast<uint32_t>(renderer._vulkan_command_buffers.size()),
            renderer._vulkan_command_buffers.data()
            );
        vkDestroyPipeline(
            renderer._vulkan_device,
            renderer._vulkan_graphics_pipeline,
            nullptr
            );
        vkDestroyPipelineLayout(
            renderer._vulkan_device,
            renderer._vulkan_pipeline_layout,
            nullptr
            );
        vkDestroyRenderPass(
            renderer._vulkan_device,
            renderer._vulkan_render_pass,
            nullptr
            );
        for (auto image_view : renderer._vulkan_swap_chain_image_views)
        {
            vkDestroyImageView(
                renderer._vulkan_device,
                image_view,
                nullptr
                );
        }
        vkDestroySwapchainKHR(
            renderer._vulkan_device,
            renderer._vulkan_swap_chain,
            nullptr
            );
        for (size_t i = 0; i < renderer._vulkan_swap_chain_images.size(); ++i)
        {
            vkDestroyBuffer(
                renderer._vulkan_device,
                renderer._vulkan_uniform_buffers[i],
                nullptr
                );
            vkFreeMemory(
                renderer._vulkan_device,
                renderer._vulkan_uniform_buffers_memory[i],
                nullptr
                );
        }
        vkDestroyDescriptorPool(
            renderer._vulkan_device,
            renderer._vulkan_descriptor_pool,
            nullptr
            );
    }

    void vulkan_recreate_swap_chain(
        lna::renderer_vulkan& renderer,
        uint32_t framebuffer_width,
        uint32_t framebuffer_height
        )
    {
        LNA_ASSERT(renderer._vulkan_device);

        VULKAN_CHECK_RESULT(
            vkDeviceWaitIdle(
                renderer._vulkan_device
                )
            )

        vulkan_cleanup_swap_chain(renderer);
        vulkan_create_swap_chain(renderer, framebuffer_width, framebuffer_height);
        vulkan_create_image_views(renderer);
        vulkan_create_render_pass(renderer);
        vulkan_create_graphics_pipeline(renderer);
        vulkan_create_framebuffers(renderer);
        vulkan_create_uniform_buffers(renderer);
        vulkan_create_descriptor_pool(renderer);
        vulkan_create_descriptor_sets(renderer);
        vulkan_create_command_buffers(renderer);
    }
}

namespace lna
{
    template<>
    void renderer_init<renderer_vulkan, window_sdl>(
        renderer_vulkan& renderer,
        const renderer_config<window_sdl>& config
        )
    {
        LNA_ASSERT(config.window_ptr);
        vulkan_create_instance(renderer, config);
        if (config.enable_validation_layers)
        {
            vulkan_setup_debug_messenger(renderer);
        }
        vulkan_create_surface(renderer, config);
        vulkan_pick_physical_device(renderer);
        vulkan_create_logical_device(renderer, config);
        vulkan_create_swap_chain(renderer, lna::window_width(*config.window_ptr), lna::window_height(*config.window_ptr));
        vulkan_create_image_views(renderer);
        vulkan_create_render_pass(renderer);
        vulkan_create_descriptor_set_layout(renderer);
        vulkan_create_graphics_pipeline(renderer);
        vulkan_create_framebuffers(renderer);
        vulkan_create_command_pool(renderer);
        vulkan_create_texture_image(renderer);
        vulkan_create_texture_image_view(renderer);
        vulkan_create_texture_sampler(renderer);
        vulkan_create_vertex_buffer(renderer);
        vulkan_create_index_buffer(renderer);
        vulkan_create_uniform_buffers(renderer);
        vulkan_create_descriptor_pool(renderer);
        vulkan_create_descriptor_sets(renderer);
        vulkan_create_command_buffers(renderer);
        vulkan_create_sync_objects(renderer);
    }

    template<>
    void renderer_draw_frame<renderer_vulkan>(
        renderer_vulkan& renderer,
        bool framebuffer_resized,
        uint32_t framebuffer_width,
        uint32_t framebuffer_height
        )
    {
        VULKAN_CHECK_RESULT(
            vkWaitForFences(
                renderer._vulkan_device,
                1,
                &renderer._vulkan_in_flight_fences[renderer._curr_frame],
                VK_TRUE,
                UINT64_MAX
                )
            )

        uint32_t image_index;
        auto result = vkAcquireNextImageKHR(
            renderer._vulkan_device,
            renderer._vulkan_swap_chain,
            UINT64_MAX,
            renderer._vulkan_image_available_semaphores[renderer._curr_frame],
            VK_NULL_HANDLE,
            &image_index
            );
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            vulkan_recreate_swap_chain(
                renderer,
                framebuffer_width,
                framebuffer_height
                );
            return;
        }
        else if (
                result != VK_SUCCESS
            &&  result != VK_SUBOPTIMAL_KHR
            )
        {
            LNA_ASSERT(0);
            return;
        }

        if (renderer._vulkan_images_in_flight_fences[image_index] != VK_NULL_HANDLE)
        {
            VULKAN_CHECK_RESULT(
                vkWaitForFences(
                    renderer._vulkan_device,
                    1,
                    &renderer._vulkan_images_in_flight_fences[image_index],
                    VK_TRUE,
                    UINT64_MAX
                    )
                )
        }
        renderer._vulkan_images_in_flight_fences[image_index] = renderer._vulkan_in_flight_fences[renderer._curr_frame];

        VkSemaphore wait_semaphores[] =
        {
            renderer._vulkan_image_available_semaphores[renderer._curr_frame],
        };

        VkPipelineStageFlags wait_stages[] =
        {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        };

        VkSemaphore signal_semaphores[] =
        {
            renderer._vulkan_render_finished_semaphores[renderer._curr_frame],
        };

        vulkan_update_uniform_buffer(
            renderer,
            image_index
            );

        VkSubmitInfo submit_info{};
        submit_info.sType                   = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.waitSemaphoreCount      = 1;
        submit_info.pWaitSemaphores         = wait_semaphores;
        submit_info.pWaitDstStageMask       = wait_stages;
        submit_info.commandBufferCount      = 1;
        submit_info.pCommandBuffers         = &renderer._vulkan_command_buffers[image_index];
        submit_info.signalSemaphoreCount    = 1;
        submit_info.pSignalSemaphores       = signal_semaphores;

        VULKAN_CHECK_RESULT(
            vkResetFences(
                renderer._vulkan_device,
                1,
                &renderer._vulkan_in_flight_fences[renderer._curr_frame]
                )
            )

        VULKAN_CHECK_RESULT(
            vkQueueSubmit(
                renderer._vulkan_graphics_queue,
                1,
                &submit_info,
                renderer._vulkan_in_flight_fences[renderer._curr_frame]
                )
            )

        VkSwapchainKHR swap_chains[] =
        {
            renderer._vulkan_swap_chain,
        };

        VkPresentInfoKHR present_info{};
        present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores    = signal_semaphores;
        present_info.swapchainCount     = 1;
        present_info.pSwapchains        = swap_chains;
        present_info.pImageIndices      = &image_index;
        present_info.pResults           = nullptr;

        result = vkQueuePresentKHR(
            renderer._vulkan_present_queue,
            &present_info
            );
        if (
                result == VK_ERROR_OUT_OF_DATE_KHR
            ||  result == VK_SUBOPTIMAL_KHR
            ||  framebuffer_resized
            )
        {
            vulkan_recreate_swap_chain(
                renderer,
                framebuffer_width,
                framebuffer_height
                );
            return;
        }
        else if (result != VK_SUCCESS)
        {
            LNA_ASSERT(0);
            return;
        }

        renderer._curr_frame = (renderer._curr_frame + 1) % VULKAN_MAX_FRAMES_IN_FLIGHT;
    }

    template<>
    void renderer_release<renderer_vulkan>(
        renderer_vulkan& renderer
        )
    {
        LNA_ASSERT(renderer._vulkan_device);
        VULKAN_CHECK_RESULT(
            vkDeviceWaitIdle(
                renderer._vulkan_device
                )
            )
        vulkan_cleanup_swap_chain(renderer);
        vkDestroySampler(
            renderer._vulkan_device,
            renderer._vulkan_texture_sampler,
            nullptr
            );
        vkDestroyImageView(
            renderer._vulkan_device,
            renderer._vulkan_texture_image_view,
            nullptr
            );
        vkDestroyImage(
            renderer._vulkan_device,
            renderer._vulkan_texture_image,
            nullptr
            );
        vkFreeMemory(
            renderer._vulkan_device,
            renderer._vulkan_texture_image_memory,
            nullptr
            );
        vkDestroyDescriptorSetLayout(
            renderer._vulkan_device,
            renderer._vulkan_descriptor_set_layout,
            nullptr
            );
        vkDestroyBuffer(
            renderer._vulkan_device,
            renderer._vulkan_index_buffer,
            nullptr
            );
        vkFreeMemory(
            renderer._vulkan_device,
            renderer._vulkan_index_buffer_memory,
            nullptr
            );
        vkDestroyBuffer(
            renderer._vulkan_device,
            renderer._vulkan_vertex_buffer,
            nullptr
            );
        vkFreeMemory(
            renderer._vulkan_device,
            renderer._vulkan_vertex_buffer_memory,
            nullptr
            );
        for (size_t i = 0; i < VULKAN_MAX_FRAMES_IN_FLIGHT; ++i)
        {
            vkDestroySemaphore(
                renderer._vulkan_device,
                renderer._vulkan_render_finished_semaphores[i],
                nullptr
                );
            vkDestroySemaphore(
                renderer._vulkan_device,
                renderer._vulkan_image_available_semaphores[i],
                nullptr
                );
            vkDestroyFence(
                renderer._vulkan_device,
                renderer._vulkan_in_flight_fences[i],
                nullptr
                );
        }
        vkDestroyCommandPool(
            renderer._vulkan_device,
            renderer._vulkan_command_pool,
            nullptr    
            );
        vkDestroyDevice(
            renderer._vulkan_device,
            nullptr
            );
        if (renderer._vulkan_debug_messenger)
        {
            vulkan_destroy_debug_utilis_messenger_EXT(
                renderer._vulkan_instance,
                renderer._vulkan_debug_messenger,
                nullptr
                );
        }
        vkDestroySurfaceKHR(
            renderer._vulkan_instance,
            renderer._vulkan_surface,
            nullptr
            );
        vkDestroyInstance(
            renderer._vulkan_instance,
            nullptr
            );
    }
}
