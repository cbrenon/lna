#include <cstring>
#include <algorithm>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlanguage-extension-token"
#pragma warning(push, 0)
#include <SDL.h>
#pragma warning(pop)
#pragma clang diagnostic pop

#include "platform/vulkan/vulkan_renderer.hpp"
#include "platform/vulkan/vulkan_helpers.hpp"
#include "platform/vulkan/vulkan_mesh.hpp"
#include "platform/vulkan/vulkan_vertex.hpp"
#include "core/file.hpp"
#include "core/log.hpp"
#include "core/assert.hpp"
#include "maths/mat4.hpp"
#include "graphics/vertex.hpp"

namespace
{
    const char* VULKAN_VALIDATION_LAYERS[] =
    {
        "VK_LAYER_KHRONOS_validation",
    };

    const char* VULKAN_DEVICE_EXTENSIONS[] =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    struct vulkan_queue_family_indices
    {
        uint32_t    graphics_family;
        uint32_t    present_family;
    };

    bool vulkan_queue_family_indices_is_complete(
        const vulkan_queue_family_indices& indices
        )
    {
        return
                indices.graphics_family != (uint32_t)-1
            &&  indices.present_family != (uint32_t)-1
            ;
    }

    struct vulkan_swap_chain_support_details
    {
        VkSurfaceCapabilitiesKHR    capabilities;
        VkSurfaceFormatKHR*         formats;
        uint32_t                    format_count;
        VkPresentModeKHR*           present_modes;
        uint32_t                    present_mode_count;
    };
    
    // const lna::vertex VERTICES[] =
    // {
    //     {
    //         {-0.5f, -0.5f, 0.0f },
    //         { 1.0f, 1.0f, 1.0f, 1.0f },
    //         { 1.0f, 0.0f },
    //     },
    //     {
    //         { 0.5f, -0.5f, 0.0f },
    //         { 1.0f, 1.0f, 1.0f, 1.0f },
    //         { 0.0f, 0.0f },
    //     },
    //     {
    //         { 0.5f, 0.5f, 0.0f },
    //         { 1.0f, 1.0f, 1.0f, 1.0f },
    //         { 0.0f, 1.0f },
    //     },
    //     {
    //         { -0.5f, 0.5f, 0.0f },
    //         { 1.0f, 1.0f, 1.0f, 1.0f },
    //         { 1.0f, 1.0f },
    //     }
    // };

    // const uint16_t INDICES[] =
    // {
    //     0, 1, 2, 2, 3, 0
    // };

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
            LNA_ASSERT(message_severity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT);
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

    bool vulkan_check_validation_layer_support(
        lna::renderer_api& renderer
        )
    {
        uint32_t layer_count;
        VULKAN_CHECK_RESULT(
            vkEnumerateInstanceLayerProperties(
                &layer_count,
                nullptr
                )
            )
        VkLayerProperties* available_layers = (VkLayerProperties*)lna::memory_pool_reserve(
            renderer.memory_pools[lna::renderer_api::FRAME_LIFETIME_MEMORY_POOL],
            layer_count * sizeof(VkLayerProperties)
            );
        VULKAN_CHECK_RESULT(
            vkEnumerateInstanceLayerProperties(
                &layer_count,
                available_layers
                )
            )

        auto validation_layer_count = sizeof(VULKAN_VALIDATION_LAYERS) / sizeof(VULKAN_VALIDATION_LAYERS[0]);
        for (size_t i = 0; i < validation_layer_count; ++i)
        {
            auto layer_found = false;
            for (uint32_t j = 0; j < layer_count; ++j)
            {
                if (strcmp(VULKAN_VALIDATION_LAYERS[i], available_layers[j].layerName) == 0)
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
        lna::renderer_api& renderer,
        VkPhysicalDevice device,
        VkSurfaceKHR surface
        )
    {
        LNA_ASSERT(device);
        LNA_ASSERT(surface);

        vulkan_queue_family_indices indices;
        indices.graphics_family = (uint32_t)-1;
        indices.present_family  = (uint32_t)-1;
        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(
            device,
            &queue_family_count,
            nullptr
            );
        VkQueueFamilyProperties* queue_families = (VkQueueFamilyProperties*)lna::memory_pool_reserve(
            renderer.memory_pools[lna::renderer_api::FRAME_LIFETIME_MEMORY_POOL],
            queue_family_count * sizeof(VkQueueFamilyProperties)
            );
        vkGetPhysicalDeviceQueueFamilyProperties(
            device,
            &queue_family_count,
            queue_families
            );

        for (uint32_t i = 0; i < queue_family_count; ++i)
        {
            if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphics_family = i;
            }

            VkBool32 present_support = false;
            VULKAN_CHECK_RESULT(
                vkGetPhysicalDeviceSurfaceSupportKHR(
                    device,
                    i,
                    surface,
                    &present_support
                    )
                )
            if (present_support)
            {
                indices.present_family = i;
            }

            if (vulkan_queue_family_indices_is_complete(indices))
            {
                break;
            }
        }
        return indices;
    }

    bool vulkan_check_device_extension_support(
        lna::renderer_api& renderer,
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
        VkExtensionProperties* available_extensions = (VkExtensionProperties*)lna::memory_pool_reserve(
            renderer.memory_pools[lna::renderer_api::FRAME_LIFETIME_MEMORY_POOL],
            extension_count * sizeof(VkExtensionProperties)
            );
        VULKAN_CHECK_RESULT(
            vkEnumerateDeviceExtensionProperties(
                device,
                nullptr,
                &extension_count,
                available_extensions
                )
            )

        size_t required_extension_count = sizeof(VULKAN_DEVICE_EXTENSIONS) / sizeof(VULKAN_DEVICE_EXTENSIONS[0]);
        for (size_t i = 0; i < required_extension_count; ++i)
        {
            bool extension_found = false;
            for (uint32_t j = 0; j < extension_count; ++j)
            {

                if (strcmp(VULKAN_DEVICE_EXTENSIONS[i], available_extensions[j].extensionName) == 0)
                {
                    extension_found = true;
                    break;
                }
            }
            if (!extension_found)
            {
                return false;
            }
        }
        return true;
    }

    vulkan_swap_chain_support_details vulkan_query_swap_chain_support(
        lna::renderer_api& renderer,
        VkPhysicalDevice device,
        VkSurfaceKHR surface
        )
    {
        LNA_ASSERT(device);
        LNA_ASSERT(surface);

        vulkan_swap_chain_support_details details;
        details.formats             = nullptr;
        details.format_count        = 0;
        details.present_modes       = nullptr;
        details.present_mode_count  = 0;

        VULKAN_CHECK_RESULT(
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                device,
                surface,
                &details.capabilities
                )
            )

        VULKAN_CHECK_RESULT(
            vkGetPhysicalDeviceSurfaceFormatsKHR(
                device,
                surface,
                &details.format_count,
                nullptr
                )
            )
        if (details.format_count > 0)
        {
            details.formats = (VkSurfaceFormatKHR*)lna::memory_pool_reserve(
                renderer.memory_pools[lna::renderer_api::FRAME_LIFETIME_MEMORY_POOL],
                details.format_count * sizeof(VkSurfaceFormatKHR)
                );
            VULKAN_CHECK_RESULT(
                vkGetPhysicalDeviceSurfaceFormatsKHR(
                    device,
                    surface,
                    &details.format_count,
                    details.formats
                    )
                )
        }

        VULKAN_CHECK_RESULT(
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                device,
                surface,
                &details.present_mode_count,
                nullptr
                )
            )
        if (details.present_mode_count > 0)
        {
            details.present_modes = (VkPresentModeKHR*)lna::memory_pool_reserve(
                renderer.memory_pools[lna::renderer_api::FRAME_LIFETIME_MEMORY_POOL],
                details.present_mode_count * sizeof(VkPresentModeKHR)
                );
            VULKAN_CHECK_RESULT(
                vkGetPhysicalDeviceSurfacePresentModesKHR(
                    device,
                    surface,
                    &details.present_mode_count,
                    details.present_modes
                    )
                )
        }

        return details;
    }

    VkSurfaceFormatKHR vulkan_choose_swap_surface_format(
        const VkSurfaceFormatKHR* available_formats,
        uint32_t available_format_count
        )
    {
        LNA_ASSERT(available_formats);
        for (uint32_t i = 0; i < available_format_count; ++i)
        {
            if (
                available_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB
                && available_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
                )
            {
                return available_formats[i];
            }
        }
        return available_formats[0];
    }

    VkPresentModeKHR vulkan_choose_swap_present_mode(
        const VkPresentModeKHR* available_present_modes,
        uint32_t available_present_mode_count
        )
    {
        LNA_ASSERT(available_present_modes);
        for (uint32_t i = 0; i < available_present_mode_count; ++i)
        {
            if (available_present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return available_present_modes[i];
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
        lna::renderer_api& renderer,
        VkPhysicalDevice device,
        VkSurfaceKHR surface
        )
    {
        vulkan_queue_family_indices indices = vulkan_find_queue_families(
            renderer,
            device,
            surface
            );
        bool supported_extensions = vulkan_check_device_extension_support(
            renderer,
            device
            );
        bool swap_chain_adequate = false;

        if (supported_extensions)
        {
            vulkan_swap_chain_support_details swap_chain_support = vulkan_query_swap_chain_support(
                renderer,
                device,
                surface
                );
            swap_chain_adequate =
                    swap_chain_support.format_count != 0
                &&  swap_chain_support.present_mode_count != 0;
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
        return
                vulkan_queue_family_indices_is_complete(indices)
            &&  supported_extensions
            &&  swap_chain_adequate
            &&  supported_features.samplerAnisotropy;
    }

    VkShaderModule vulkan_create_shader_module(
        lna::renderer_api& renderer,
        const char* code,
        uint32_t code_size
        )
    {
        LNA_ASSERT(renderer.device);
        LNA_ASSERT(code);
        LNA_ASSERT(code_size > 0);

        VkShaderModuleCreateInfo shader_module_create_info{};
        shader_module_create_info.sType     = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shader_module_create_info.codeSize  = code_size;
        shader_module_create_info.pCode     = reinterpret_cast<const uint32_t*>(code);

        VkShaderModule shader_module = nullptr;
        VULKAN_CHECK_RESULT(
            vkCreateShaderModule(
                renderer.device,
                &shader_module_create_info,
                nullptr,
                &shader_module
                )
            )
        return shader_module;
    }

    //! VULKAN INIT PROCESS FUNCTION -------------------------------------------

    void vulkan_create_instance(
        lna::renderer_api& renderer,
        const lna::renderer_config& config
        )
    {
        LNA_ASSERT(renderer.instance == nullptr);
        LNA_ASSERT(config.window_ptr);

        if (
            config.enable_validation_layers
            && !vulkan_check_validation_layer_support(renderer)
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

        const lna::window_extension_infos& extensions   = lna::window_extensions(*config.window_ptr);

        VkInstanceCreateInfo instance_create_info {};
        instance_create_info.sType                      = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.pApplicationInfo           = &application_info;
        instance_create_info.enabledExtensionCount      = extensions.count;
        instance_create_info.ppEnabledExtensionNames    = extensions.names;

        if (config.enable_validation_layers)
        {
            VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info{};
            debug_messenger_create_info.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debug_messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debug_messenger_create_info.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debug_messenger_create_info.pfnUserCallback = vulkan_debug_callback;
            debug_messenger_create_info.pUserData       = nullptr;
            instance_create_info.enabledLayerCount      = static_cast<uint32_t>(sizeof(VULKAN_VALIDATION_LAYERS) / sizeof(VULKAN_VALIDATION_LAYERS[0]));
            instance_create_info.ppEnabledLayerNames    = VULKAN_VALIDATION_LAYERS;
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
                &renderer.instance
                )
            )
    }

    void vulkan_setup_debug_messenger(
        lna::renderer_api& renderer
        )
    {
        LNA_ASSERT(renderer.instance);
        LNA_ASSERT(renderer.debug_messenger == nullptr);

        VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info{};
        debug_messenger_create_info.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debug_messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debug_messenger_create_info.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debug_messenger_create_info.pfnUserCallback = vulkan_debug_callback;
        debug_messenger_create_info.pUserData       = nullptr;

        VULKAN_CHECK_RESULT(
            vulkan_create_debug_utils_messenger_EXT(
                renderer.instance,
                &debug_messenger_create_info,
                nullptr,
                &renderer.debug_messenger
                )
            )
    }

    void vulkan_create_surface(
        lna::renderer_api& renderer,
        const lna::renderer_config& config
        )
    {
        LNA_ASSERT(renderer.surface == nullptr);
        LNA_ASSERT(renderer.instance);
        LNA_ASSERT(config.window_ptr);

        auto result = SDL_Vulkan_CreateSurface(
            config.window_ptr->handle,
            renderer.instance,
            &renderer.surface
            );
        LNA_ASSERT(result == SDL_TRUE);
    }

    void vulkan_pick_physical_device(
        lna::renderer_api& renderer
        )
    {
        LNA_ASSERT(renderer.instance);
        LNA_ASSERT(renderer.physical_device == nullptr);

        uint32_t device_count = 0;
        VULKAN_CHECK_RESULT(
            vkEnumeratePhysicalDevices(
                renderer.instance,
                &device_count,
                nullptr
                )
            )
        LNA_ASSERT(device_count > 0);

        VkPhysicalDevice* devices = (VkPhysicalDevice*)lna::memory_pool_reserve(
            renderer.memory_pools[lna::renderer_api::FRAME_LIFETIME_MEMORY_POOL],
            device_count * sizeof(VkPhysicalDevice)
        );
        VULKAN_CHECK_RESULT(
            vkEnumeratePhysicalDevices(
                renderer.instance,
                &device_count,
                devices
                )
            )
        for (uint32_t i = 0; i < device_count; ++i)
        {
            if (
                vulkan_is_physical_device_suitable(
                    renderer,
                    devices[i],
                    renderer.surface
                    )
                )
            {
                renderer.physical_device = devices[i];
                break;
            }
        }
    }

    void vulkan_create_logical_device(
        lna::renderer_api& renderer,
        const lna::renderer_config& config
        )
    {
        LNA_ASSERT(renderer.physical_device);
        LNA_ASSERT(renderer.device == nullptr);
        LNA_ASSERT(renderer.graphics_queue == nullptr);

        vulkan_queue_family_indices indices = vulkan_find_queue_families(
            renderer,
            renderer.physical_device,
            renderer.surface
            );

        uint32_t unique_queue_families[] =
        {
            indices.graphics_family,
            indices.present_family
        };
        uint32_t unique_queue_family_count = (indices.graphics_family == indices.present_family) ? 1 : 2;

        VkDeviceQueueCreateInfo* queue_create_infos = (VkDeviceQueueCreateInfo*)lna::memory_pool_reserve(
            renderer.memory_pools[lna::renderer_api::FRAME_LIFETIME_MEMORY_POOL],
            unique_queue_family_count * sizeof(VkDeviceQueueCreateInfo)
            );

        float queue_priority = 1.0f;
        for (size_t i = 0; i < unique_queue_family_count; ++i)
        {
            queue_create_infos[i].sType             = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_infos[i].queueFamilyIndex  = unique_queue_families[i];
            queue_create_infos[i].queueCount        = 1;
            queue_create_infos[i].pQueuePriorities  = &queue_priority;
        }

        VkPhysicalDeviceFeatures device_features{};
        device_features.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo device_create_info{};
        device_create_info.sType                    = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.pQueueCreateInfos        = queue_create_infos;
        device_create_info.queueCreateInfoCount     = unique_queue_family_count;
        device_create_info.pEnabledFeatures         = &device_features;
        device_create_info.enabledExtensionCount    = static_cast<uint32_t>(sizeof(VULKAN_DEVICE_EXTENSIONS) / sizeof(VULKAN_DEVICE_EXTENSIONS[0]));
        device_create_info.ppEnabledExtensionNames  = VULKAN_DEVICE_EXTENSIONS;
        device_create_info.enabledLayerCount        = config.enable_validation_layers ? static_cast<uint32_t>(sizeof(VULKAN_VALIDATION_LAYERS) / sizeof(VULKAN_VALIDATION_LAYERS[0])) : 0;
        device_create_info.ppEnabledLayerNames      = config.enable_validation_layers ? VULKAN_VALIDATION_LAYERS : nullptr;

        VULKAN_CHECK_RESULT(
            vkCreateDevice(
                renderer.physical_device,
                &device_create_info,
                nullptr,
                &renderer.device
                )
            )
        vkGetDeviceQueue(
            renderer.device,
            indices.graphics_family,
            0,
            &renderer.graphics_queue
            );
        vkGetDeviceQueue(
            renderer.device,
            indices.present_family,
            0,
            &renderer.present_queue
            );
    }

    void vulkan_create_swap_chain(
        lna::renderer_api& renderer,
        uint32_t framebuffer_width,
        uint32_t framebuffer_height
        )
    {
        LNA_ASSERT(renderer.device);
        LNA_ASSERT(renderer.swap_chain == nullptr);

        vulkan_swap_chain_support_details   swap_chain_support  = vulkan_query_swap_chain_support(renderer, renderer.physical_device, renderer.surface);
        VkSurfaceFormatKHR                  surface_format      = vulkan_choose_swap_surface_format(swap_chain_support.formats, swap_chain_support.format_count);
        VkPresentModeKHR                    present_mode        = vulkan_choose_swap_present_mode(swap_chain_support.present_modes, swap_chain_support.present_mode_count);
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
        swap_chain_create_info.surface          = renderer.surface;
        swap_chain_create_info.minImageCount    = image_count;
        swap_chain_create_info.imageFormat      = surface_format.format;
        swap_chain_create_info.imageColorSpace  = surface_format.colorSpace;
        swap_chain_create_info.imageExtent      = extent;
        swap_chain_create_info.imageArrayLayers = 1;
        swap_chain_create_info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        vulkan_queue_family_indices indices = vulkan_find_queue_families(
            renderer,
            renderer.physical_device,
            renderer.surface
            );
        uint32_t queue_family_indices[]     =
        {
            indices.graphics_family,
            indices.present_family
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
                renderer.device,
                &swap_chain_create_info,
                nullptr,
                &renderer.swap_chain
                )
            )
        VULKAN_CHECK_RESULT(
            vkGetSwapchainImagesKHR(
                renderer.device,
                renderer.swap_chain,
                &image_count,
                nullptr
                )
            )

        renderer.swap_chain_image_count = image_count;
        renderer.swap_chain_images = (VkImage*)lna::memory_pool_reserve(
            renderer.memory_pools[lna::renderer_api::SWAP_CHAIN_LIFETIME_MEMORY_POOL],
            image_count * sizeof(VkImage)
            );
        VULKAN_CHECK_RESULT(
            vkGetSwapchainImagesKHR(
                renderer.device,
                renderer.swap_chain,
                &image_count,
                renderer.swap_chain_images
                )
            )
        renderer.swap_chain_image_format    = surface_format.format;
        renderer.swap_chain_extent          = extent;
    }

    void vulkan_create_image_views(
        lna::renderer_api& renderer
        )
    {
        renderer.swap_chain_image_views = (VkImageView*)lna::memory_pool_reserve(
            renderer.memory_pools[lna::renderer_api::SWAP_CHAIN_LIFETIME_MEMORY_POOL],
            renderer.swap_chain_image_count * sizeof(VkImageView)
            );

        for (size_t i = 0; i < renderer.swap_chain_image_count; ++i)
        {
            renderer.swap_chain_image_views[i] = lna::vulkan_helpers::create_image_view(
                renderer.device,
                renderer.swap_chain_images[i],
                renderer.swap_chain_image_format
                );
        }
    }

    void vulkan_create_render_pass(
        lna::renderer_api& renderer
        )
    {
        LNA_ASSERT(renderer.device);

        VkAttachmentDescription color_attachment{};
        color_attachment.format                     = renderer.swap_chain_image_format;
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
                renderer.device,
                &render_pass_create_info,
                nullptr,
                &renderer.render_pass
                )
            )
    }

    void vulkan_create_descriptor_set_layout(
        lna::renderer_api& renderer
        )
    {
        LNA_ASSERT(renderer.device);

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

        VkDescriptorSetLayoutBinding bindings[2];
        bindings[0] = ubo_layout_binding;
        bindings[1] = sampler_layout_binding;

        VkDescriptorSetLayoutCreateInfo layout_create_info{};
        layout_create_info.sType                    = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_create_info.bindingCount             = static_cast<uint32_t>(sizeof(bindings) / sizeof(bindings[0]));
        layout_create_info.pBindings                = bindings;

        VULKAN_CHECK_RESULT(
            vkCreateDescriptorSetLayout(
                renderer.device,
                &layout_create_info,
                nullptr,
                &renderer.mesh_system.descriptor_set_layout
                )
            )
    }

    void vulkan_create_graphics_pipeline(
        lna::renderer_api& renderer
        )
    {
        LNA_ASSERT(renderer.device);
        LNA_ASSERT(renderer.render_pass);

        lna::file vertex_shader_file;
        lna::file_init(vertex_shader_file);
        lna::file_debug_load(
            vertex_shader_file,
            "shaders/default_vert.spv",
            true,
            renderer.memory_pools[lna::renderer_api::FRAME_LIFETIME_MEMORY_POOL]
            );
        VkShaderModule vertex_shader_module = vulkan_create_shader_module(
            renderer,
            vertex_shader_file.content,
            vertex_shader_file.content_size
            );
        lna::file fragment_shader_file;
        lna::file_init(fragment_shader_file);
        lna::file_debug_load(
            fragment_shader_file,
            "shaders/default_frag.spv",
            true,
            renderer.memory_pools[lna::renderer_api::FRAME_LIFETIME_MEMORY_POOL]
            );
        VkShaderModule fragment_shader_module = vulkan_create_shader_module(
            renderer,
            fragment_shader_file.content,
            fragment_shader_file.content_size
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

        auto vertex_description = lna::vulkan_default_vertex_description();

        VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
        vertex_input_state_create_info.sType                            = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_state_create_info.vertexBindingDescriptionCount    = lna::vulkan_vertex_description::MAX_BINDING;
        vertex_input_state_create_info.pVertexBindingDescriptions       = vertex_description.bindings;
        vertex_input_state_create_info.vertexAttributeDescriptionCount  = lna::vulkan_vertex_description::MAX_ATTRIBUTES;
        vertex_input_state_create_info.pVertexAttributeDescriptions     = vertex_description.attributes;

        VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info{};
        input_assembly_state_create_info.sType                          = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly_state_create_info.topology                       = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly_state_create_info.primitiveRestartEnable         = VK_FALSE;

        VkViewport viewport{};
        viewport.x          = 0.0f;
        viewport.y          = 0.0f;
        viewport.width      = static_cast<float>(renderer.swap_chain_extent.width);
        viewport.height     = static_cast<float>(renderer.swap_chain_extent.height);
        viewport.minDepth   = 0.0f;
        viewport.maxDepth   = 1.0f;

        VkRect2D scissor{};
        scissor.offset      = { 0, 0 };
        scissor.extent      = renderer.swap_chain_extent;

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
        layout_create_info.pSetLayouts              = &renderer.mesh_system.descriptor_set_layout;
        layout_create_info.pushConstantRangeCount   = 0;
        layout_create_info.pPushConstantRanges      = nullptr;

        VULKAN_CHECK_RESULT(
            vkCreatePipelineLayout(
                renderer.device,
                &layout_create_info,
                nullptr,
                &renderer.pipeline_layout
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
        graphics_pipeline_create_info.layout                = renderer.pipeline_layout;
        graphics_pipeline_create_info.renderPass            = renderer.render_pass;
        graphics_pipeline_create_info.subpass               = 0;
        graphics_pipeline_create_info.basePipelineHandle    = VK_NULL_HANDLE;
        graphics_pipeline_create_info.basePipelineIndex     = -1;

        VULKAN_CHECK_RESULT(
            vkCreateGraphicsPipelines(
                renderer.device,
                VK_NULL_HANDLE,
                1,
                &graphics_pipeline_create_info,
                nullptr,
                &renderer.graphics_pipeline
                )
            )

        vkDestroyShaderModule(renderer.device, fragment_shader_module, nullptr);
        vkDestroyShaderModule(renderer.device, vertex_shader_module, nullptr);
    }

    void vulkan_create_framebuffers(
        lna::renderer_api& renderer
        )
    {
        LNA_ASSERT(renderer.device);

        renderer.swap_chain_framebuffers = (VkFramebuffer*)lna::memory_pool_reserve(
            renderer.memory_pools[lna::renderer_api::SWAP_CHAIN_LIFETIME_MEMORY_POOL],
            renderer.swap_chain_image_count * sizeof(VkFramebuffer)
            );
        for (size_t i = 0; i < renderer.swap_chain_image_count; ++i)
        {
            VkImageView attachments[] =
            {
                renderer.swap_chain_image_views[i]
            };

            VkFramebufferCreateInfo framebuffer_create_info{};
            framebuffer_create_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_create_info.renderPass      = renderer.render_pass;
            framebuffer_create_info.attachmentCount = 1;
            framebuffer_create_info.pAttachments    = attachments;
            framebuffer_create_info.width           = renderer.swap_chain_extent.width;
            framebuffer_create_info.height          = renderer.swap_chain_extent.height;
            framebuffer_create_info.layers          = 1;

            VULKAN_CHECK_RESULT(
                vkCreateFramebuffer(
                    renderer.device,
                    &framebuffer_create_info,
                    nullptr,
                    &renderer.swap_chain_framebuffers[i]
                    )
                )
        }
    }

    void vulkan_create_command_pool(
        lna::renderer_api& renderer
        )
    {
        LNA_ASSERT(renderer.device);

        vulkan_queue_family_indices indices = vulkan_find_queue_families(
            renderer,
            renderer.physical_device,
            renderer.surface
            );

        VkCommandPoolCreateInfo command_pool_create_info{};
        command_pool_create_info.sType              = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_create_info.queueFamilyIndex   = indices.graphics_family;
        command_pool_create_info.flags              = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT 

        VULKAN_CHECK_RESULT(
            vkCreateCommandPool(
                renderer.device,
                &command_pool_create_info,
                nullptr,
                &renderer.command_pool
                )
            )
    }

    void vulkan_create_command_buffers(
        lna::renderer_api& renderer
        )
    {
        LNA_ASSERT(renderer.device);

        renderer.command_buffers = (VkCommandBuffer*)lna::memory_pool_reserve(
            renderer.memory_pools[lna::renderer_api::SWAP_CHAIN_LIFETIME_MEMORY_POOL],
            renderer.swap_chain_image_count * sizeof(VkCommandBuffer)
            );

        VkCommandBufferAllocateInfo command_buffer_allocate_info{}; 
        command_buffer_allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_allocate_info.commandPool        = renderer.command_pool;
        command_buffer_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        command_buffer_allocate_info.commandBufferCount = renderer.swap_chain_image_count;

        VULKAN_CHECK_RESULT(
            vkAllocateCommandBuffers(
                renderer.device,
                &command_buffer_allocate_info,
                renderer.command_buffers
                )
            )
    }

    void vulkan_create_sync_objects(
        lna::renderer_api& renderer
        )
    {
        LNA_ASSERT(renderer.device);

        renderer.images_in_flight_fences = (VkFence*)lna::memory_pool_reserve(
            renderer.memory_pools[lna::renderer_api::PERSISTENT_LIFETIME_MEMORY_POOL],
            renderer.swap_chain_image_count * sizeof(VkFence)
            );
        for (uint32_t i = 0; i < renderer.swap_chain_image_count; ++i)
        {
            renderer.images_in_flight_fences[i] = nullptr;
        }

        VkSemaphoreCreateInfo semaphore_create_info{};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fence_create_info{};
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (uint32_t i = 0; i < lna::VULKAN_MAX_FRAMES_IN_FLIGHT; ++i)
        {
            VULKAN_CHECK_RESULT(
                vkCreateSemaphore(
                    renderer.device,
                    &semaphore_create_info,
                    nullptr,
                    &renderer.image_available_semaphores[i]
                    )
                )
            VULKAN_CHECK_RESULT(
                vkCreateSemaphore(
                    renderer.device,
                    &semaphore_create_info,
                    nullptr,
                    &renderer.render_finished_semaphores[i]
                    )
                )
            VULKAN_CHECK_RESULT(
                vkCreateFence(
                    renderer.device,
                    &fence_create_info,
                    nullptr,
                    &renderer.in_flight_fences[i]
                    )
                )
        }
    }

    // TODO: we will have to create another descriptor pool when we will be working on the debug primitive as descriptor set will be different (no texture sampler) 
    void vulkan_create_descriptor_pool(
        lna::renderer_api& renderer
        )
    {
        LNA_ASSERT(renderer.device);
        LNA_ASSERT(renderer.mesh_system.descriptor_pool == nullptr);
        LNA_ASSERT(renderer.mesh_system.max_mesh_count > 0); // mesh_system must have been previously configured

        VkDescriptorPoolSize pool_sizes[2] {};
        pool_sizes[0].type              = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        pool_sizes[0].descriptorCount   = renderer.swap_chain_image_count;
        pool_sizes[1].type              = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        pool_sizes[1].descriptorCount   = renderer.swap_chain_image_count;;

        VkDescriptorPoolCreateInfo pool_create_info{};
        pool_create_info.sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_create_info.poolSizeCount  = static_cast<uint32_t>(sizeof(pool_sizes) / sizeof(pool_sizes[0]));
        pool_create_info.pPoolSizes     = pool_sizes;
        pool_create_info.maxSets        = renderer.swap_chain_image_count * renderer.mesh_system.max_mesh_count;

        VULKAN_CHECK_RESULT(
            vkCreateDescriptorPool(
                renderer.device,
                &pool_create_info,
                nullptr,
                &renderer.mesh_system.descriptor_pool
                )
            )
    }

    void vulkan_cleanup_swap_chain(
        lna::renderer_api& renderer
        )
    {
        LNA_ASSERT(renderer.device);

        for (uint32_t i = 0; i < renderer.swap_chain_image_count; ++i)
        {
            vkDestroyFramebuffer(
                renderer.device,
                renderer.swap_chain_framebuffers[i],
                nullptr
                );
        }
        vkFreeCommandBuffers(
            renderer.device,
            renderer.command_pool,
            renderer.swap_chain_image_count,
            renderer.command_buffers
            );
        vkDestroyPipeline(
            renderer.device,
            renderer.graphics_pipeline,
            nullptr
            );
        vkDestroyPipelineLayout(
            renderer.device,
            renderer.pipeline_layout,
            nullptr
            );
        vkDestroyRenderPass(
            renderer.device,
            renderer.render_pass,
            nullptr
            );
        for (uint32_t i = 0; i < renderer.swap_chain_image_count; ++i)
        {
            vkDestroyImageView(
                renderer.device,
                renderer.swap_chain_image_views[i],
                nullptr
                );
        }
        vkDestroySwapchainKHR(
            renderer.device,
            renderer.swap_chain,
            nullptr
            );

        for (uint32_t i = 0; i < renderer.mesh_system.cur_mesh_count; ++i)
        {
            lna::vulkan_mesh_clean_uniform_buffer(
                renderer.mesh_system.meshes[i],
                renderer.device
                );
            lna::vulkan_mesh_clean_descriptor_sets(
                renderer.mesh_system.meshes[i]
                );
        }

        vkDestroyDescriptorPool(
            renderer.device,
            renderer.mesh_system.descriptor_pool,
            nullptr
            );

        lna::memory_pool_empty(
            renderer.memory_pools[lna::renderer_api::SWAP_CHAIN_LIFETIME_MEMORY_POOL]
            );
        renderer.swap_chain_images          = nullptr;
        renderer.swap_chain_image_views     = nullptr;
        renderer.swap_chain_framebuffers    = nullptr;
        renderer.command_buffers            = nullptr;
    }

    void vulkan_recreate_swap_chain(
        lna::renderer_api& renderer,
        uint32_t framebuffer_width,
        uint32_t framebuffer_height
        )
    {
        LNA_ASSERT(renderer.device);

        VULKAN_CHECK_RESULT(
            vkDeviceWaitIdle(
                renderer.device
                )
            )

        vulkan_cleanup_swap_chain(renderer);
        vulkan_create_swap_chain(renderer, framebuffer_width, framebuffer_height);
        vulkan_create_image_views(renderer);
        vulkan_create_render_pass(renderer);
        vulkan_create_graphics_pipeline(renderer);
        vulkan_create_framebuffers(renderer);
        vulkan_create_descriptor_pool(renderer);

        for (uint32_t i = 0; i < renderer.mesh_system.cur_mesh_count; ++i)
        {
            lna::vulkan_mesh_create_uniform_buffer_info uniform_buffer_info{};
            uniform_buffer_info.device                      = renderer.device;
            uniform_buffer_info.physical_device             = renderer.physical_device;
            uniform_buffer_info.swap_chain_image_count      = renderer.swap_chain_image_count;
            uniform_buffer_info.swap_chain_memory_pool_ptr  = &renderer.memory_pools[lna::renderer_api::SWAP_CHAIN_LIFETIME_MEMORY_POOL];
            lna::vulkan_mesh_create_uniform_buffer(
                renderer.mesh_system.meshes[i],
                uniform_buffer_info
                );
            lna::vulkan_mesh_create_descriptor_sets_info descriptor_sets_info{};
            descriptor_sets_info.device                     = renderer.device;
            descriptor_sets_info.physical_device            = renderer.physical_device;
            descriptor_sets_info.descriptor_pool            = renderer.mesh_system.descriptor_pool;
            descriptor_sets_info.descriptor_set_layout      = renderer.mesh_system.descriptor_set_layout;
            descriptor_sets_info.swap_chain_memory_pool_ptr = &renderer.memory_pools[lna::renderer_api::SWAP_CHAIN_LIFETIME_MEMORY_POOL];
            descriptor_sets_info.temp_memory_pool_ptr       = &renderer.memory_pools[lna::renderer_api::FRAME_LIFETIME_MEMORY_POOL];
            lna::vulkan_mesh_create_descriptor_sets(
                renderer.mesh_system.meshes[i],
                descriptor_sets_info
                );
        }

        vulkan_create_command_buffers(renderer);
    }
}

namespace lna
{
    void renderer_init(
        renderer_api& renderer
        )
    {
        renderer.instance                           = nullptr;
        renderer.debug_messenger                    = nullptr;
        renderer.physical_device                    = nullptr;
        renderer.device                             = nullptr;
        renderer.graphics_queue                     = nullptr;
        renderer.surface                            = nullptr;
        renderer.present_queue                      = nullptr;
        renderer.swap_chain                         = nullptr;
        renderer.render_pass                        = nullptr;
        renderer.graphics_pipeline                  = nullptr;
        renderer.texture_system.cur_texture_count   = 0;
        renderer.texture_system.max_texture_count   = 0;
        renderer.texture_system.textures            = nullptr;
        renderer.mesh_system.descriptor_pool        = nullptr;
        renderer.mesh_system.descriptor_set_layout  = nullptr;
        renderer.mesh_system.cur_mesh_count         = 0;
        renderer.mesh_system.max_mesh_count         = 0;
        renderer.mesh_system.mesh_positions         = nullptr;
        renderer.pipeline_layout                    = nullptr;
        renderer.command_pool                       = nullptr;
        renderer.curr_frame                         = 0;
        renderer.images_in_flight_fences            = nullptr;
        renderer.swap_chain_images                  = nullptr;
        renderer.swap_chain_image_views             = nullptr;
        renderer.swap_chain_framebuffers            = nullptr;
        renderer.command_buffers                    = nullptr;
        renderer.swap_chain_image_count             = 0;
    }

    void renderer_configure(
        renderer_api& renderer,
        const renderer_config& config
        )
    {
        LNA_ASSERT(config.window_ptr);

        renderer.curr_frame = 0;

        for (uint32_t i = 0; i < renderer_api::MEMORY_POOL_COUNT; ++i)
        {
            memory_pool_init(
                renderer.memory_pools[i]
                );
            memory_pool_allocate_megabytes(
                renderer.memory_pools[i],
                MEMORY_POOL_SIZES[i]
                );
        }

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

        renderer.texture_system.max_texture_count = config.max_texture_count;
        renderer.texture_system.textures = renderer.texture_system.max_texture_count > 0 ?
            (vulkan_texture*)memory_pool_reserve(renderer.memory_pools[renderer_api::PERSISTENT_LIFETIME_MEMORY_POOL], renderer.texture_system.max_texture_count * sizeof(vulkan_texture)) : nullptr;

        renderer.mesh_system.max_mesh_count = config.max_mesh_count;
        renderer.mesh_system.meshes = renderer.mesh_system.max_mesh_count > 0 ?
            (vulkan_mesh*)memory_pool_reserve(renderer.memory_pools[renderer_api::PERSISTENT_LIFETIME_MEMORY_POOL], renderer.mesh_system.max_mesh_count * sizeof(vulkan_mesh)) : nullptr;

        vulkan_create_descriptor_pool(renderer);

        // vulkan_texture_config_info texture_info{};
        // texture_info.command_pool       = renderer.command_pool;
        // texture_info.device             = renderer.device;
        // texture_info.physical_device    = renderer.physical_device;
        // texture_info.graphics_queue     = renderer.graphics_queue;
        // texture_info.filename           = "textures/texture.jpg";
        // vulkan_texture_configure(
        //     renderer.vk_texture,
        //     texture_info
        //     );

        // heap_array_set_max_element_count(
        //     renderer.vk_graphics_objects,
        //     renderer.memory_pools[renderer_api::PERSISTENT_LIFETIME_MEMORY_POOL],
        //     renderer.max_graphics_object_count
        //     );
        // for (uint32_t i = 0; i < renderer.vk_graphics_objects.element_count; ++i)
        // {
        //     vulkan_graphics_object& graphics_object = renderer.vk_graphics_objects.elements[i];
        //     vulkan_mesh&            vk_mesh         = graphics_object.vk_mesh;
        //     float offset = -static_cast<float>(i);
        //     graphics_object.position = { 0.5f + offset, 0.5f + offset, 0.0f };
        //     vulkan_mesh_init(
        //         vk_mesh
        //         );
        //     vulkan_mesh_create_vertex_and_index_info vertex_and_index_info{};
        //     vertex_and_index_info.device = renderer.device;
        //     vertex_and_index_info.physical_device   = renderer.physical_device;
        //     vertex_and_index_info.command_pool      = renderer.command_pool;
        //     vertex_and_index_info.graphics_queue    = renderer.graphics_queue;
        //     vertex_and_index_info.vertices          = VERTICES;
        //     vertex_and_index_info.vertex_count      = static_cast<uint32_t>(sizeof(VERTICES) / sizeof(VERTICES[0]));
        //     vertex_and_index_info.indices           = INDICES;
        //     vertex_and_index_info.index_count       = static_cast<uint32_t>(sizeof(INDICES) / sizeof(INDICES[0]));
        //     vulkan_mesh_create_vertex_and_index_buffer(
        //         vk_mesh,
        //         vertex_and_index_info
        //         );
        //     vulkan_mesh_create_uniform_buffer_info uniform_buffer_info{};
        //     uniform_buffer_info.device                      = renderer.device;
        //     uniform_buffer_info.physical_device             = renderer.physical_device;
        //     uniform_buffer_info.swap_chain_image_count      = renderer.swap_chain_images.element_count;
        //     uniform_buffer_info.swap_chain_memory_pool_ptr  = &renderer.memory_pools[renderer_api::SWAP_CHAIN_LIFETIME_MEMORY_POOL];
        //     vulkan_mesh_create_uniform_buffer(
        //         vk_mesh,
        //         uniform_buffer_info
        //         );
        //     vulkan_mesh_create_descriptor_sets_info descriptor_sets_info{};
        //     descriptor_sets_info.device                     = renderer.device;
        //     descriptor_sets_info.physical_device            = renderer.physical_device;
        //     descriptor_sets_info.descriptor_pool            = renderer.descriptor_pool;
        //     descriptor_sets_info.descriptor_set_layout      = renderer.descriptor_set_layout;
        //     descriptor_sets_info.swap_chain_image_count     = renderer.swap_chain_images.element_count;
        //     descriptor_sets_info.swap_chain_memory_pool_ptr = &renderer.memory_pools[renderer_api::SWAP_CHAIN_LIFETIME_MEMORY_POOL];
        //     descriptor_sets_info.temp_memory_pool_ptr       = &renderer.memory_pools[renderer_api::FRAME_LIFETIME_MEMORY_POOL];
        //     descriptor_sets_info.texture_ptr                = &renderer.vk_texture;
        //     vulkan_mesh_create_descriptor_sets(
        //         vk_mesh,
        //         descriptor_sets_info
        //         );
        // }

        vulkan_create_command_buffers(renderer);
        vulkan_create_sync_objects(renderer);
    }

    void renderer_draw_frame(
        renderer_api& renderer,
        bool framebuffer_resized,
        uint32_t framebuffer_width,
        uint32_t framebuffer_height
        )
    {
        VULKAN_CHECK_RESULT(
            vkWaitForFences(
                renderer.device,
                1,
                &renderer.in_flight_fences[renderer.curr_frame],
                VK_TRUE,
                UINT64_MAX
                )
            )

        uint32_t image_index;
        auto result = vkAcquireNextImageKHR(
            renderer.device,
            renderer.swap_chain,
            UINT64_MAX,
            renderer.image_available_semaphores[renderer.curr_frame],
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

        if (renderer.images_in_flight_fences[image_index] != VK_NULL_HANDLE)
        {
            VULKAN_CHECK_RESULT(
                vkWaitForFences(
                    renderer.device,
                    1,
                    &renderer.images_in_flight_fences[image_index],
                    VK_TRUE,
                    UINT64_MAX
                    )
                )
        }
        renderer.images_in_flight_fences[image_index] = renderer.in_flight_fences[renderer.curr_frame];

        VkSemaphore wait_semaphores[] =
        {
            renderer.image_available_semaphores[renderer.curr_frame],
        };

        VkPipelineStageFlags wait_stages[] =
        {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        };

        VkSemaphore signal_semaphores[] =
        {
            renderer.render_finished_semaphores[renderer.curr_frame],
        };

        // TODO: (TEMP CODE) to remove when we will have a "camera system" and a "graphics object system"
        const vec3  eye     = { 0.0f, 0.0f, 2.0f };
        const vec3  target  = { 0.0f, 0.0f, 0.0f };
        const vec3  up      = { 0.0f, -1.0f, 0.0f };
        const float fov     = 45.0f;
        const float aspect  = static_cast<float>(renderer.swap_chain_extent.width) / static_cast<float>(renderer.swap_chain_extent.height);
        const float near    = 1.0f;
        const float far     = 10.0f;
        mat4        model;
        mat4        view;
        mat4        projection;
        lna::mat4_loot_at(
            view,
            eye,
            target,
            up
            );
        lna::mat4_perspective(
            projection,
            fov,
            aspect,
            near,
            far
            );
        // END TEMP CODE


        VkCommandBufferBeginInfo    command_buffer_begin_info{};
        command_buffer_begin_info.sType             = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        command_buffer_begin_info.flags             = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        command_buffer_begin_info.pInheritanceInfo  = nullptr;

        VULKAN_CHECK_RESULT(
            vkResetCommandBuffer(
                renderer.command_buffers[image_index],
                VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT
                )
            )

        VULKAN_CHECK_RESULT(
            vkBeginCommandBuffer(
                renderer.command_buffers[image_index],
                &command_buffer_begin_info
                )
            )

        VkRenderPassBeginInfo render_pass_begin_info{};
        render_pass_begin_info.sType                = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.renderPass           = renderer.render_pass;
        render_pass_begin_info.framebuffer          = renderer.swap_chain_framebuffers[image_index];
        render_pass_begin_info.renderArea.offset    = { 0, 0 };
        render_pass_begin_info.renderArea.extent    = renderer.swap_chain_extent;
        VkClearValue clear_color = {{{ 0.0f, 0.0f, 0.0f, 1.0f }}};
        render_pass_begin_info.clearValueCount      = 1;
        render_pass_begin_info.pClearValues         = &clear_color;

        vkCmdBeginRenderPass(
            renderer.command_buffers[image_index],
            &render_pass_begin_info,
            VK_SUBPASS_CONTENTS_INLINE
            );
        vkCmdBindPipeline(
            renderer.command_buffers[image_index],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            renderer.graphics_pipeline
            );

        for (uint32_t i = 0; i < renderer.mesh_system.cur_mesh_count; ++i)
        {
            lna::mat4_translation(
                model,
                renderer.mesh_system.mesh_positions[i].x,
                renderer.mesh_system.mesh_positions[i].y,
                renderer.mesh_system.mesh_positions[i].z
                );
            vulkan_mesh_update_uniform_buffer_info update_uniform_buffer_info{};
            update_uniform_buffer_info.device                   = renderer.device;
            update_uniform_buffer_info.image_index              = image_index;
            update_uniform_buffer_info.model_matrix_ptr         = &model;
            update_uniform_buffer_info.view_matrix_ptr          = &view;
            update_uniform_buffer_info.projection_matrix_ptr    = &projection;
            vulkan_mesh_upate_uniform_buffer(
                renderer.mesh_system.meshes[i],
                update_uniform_buffer_info
                );
            vkCmdBindDescriptorSets(
                renderer.command_buffers[image_index],
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                renderer.pipeline_layout,
                0,
                1,
                &renderer.mesh_system.meshes[i].descriptor_sets[image_index],
                0,
                nullptr
                );
            VkBuffer        vertex_buffers[]    = { renderer.mesh_system.meshes[i].vertex_buffer };
            VkDeviceSize    offsets[]           = { 0 };
            vkCmdBindVertexBuffers(
                renderer.command_buffers[image_index],
                0,
                1,
                vertex_buffers,
                offsets
                );
            vkCmdBindIndexBuffer(
                renderer.command_buffers[image_index],
                renderer.mesh_system.meshes[i].index_buffer,
                0,
                VK_INDEX_TYPE_UINT16
                );
            vkCmdDrawIndexed(
                renderer.command_buffers[image_index],
                renderer.mesh_system.meshes[i].index_count,
                1,
                0,
                0,
                0
                );
        }

        vkCmdEndRenderPass(
            renderer.command_buffers[image_index]
            );
        VULKAN_CHECK_RESULT(
            vkEndCommandBuffer(
                renderer.command_buffers[image_index]
                )
            )

        VkSubmitInfo submit_info{};
        submit_info.sType                   = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.waitSemaphoreCount      = 1;
        submit_info.pWaitSemaphores         = wait_semaphores;
        submit_info.pWaitDstStageMask       = wait_stages;
        submit_info.commandBufferCount      = 1;
        submit_info.pCommandBuffers         = &renderer.command_buffers[image_index];
        submit_info.signalSemaphoreCount    = 1;
        submit_info.pSignalSemaphores       = signal_semaphores;

        VULKAN_CHECK_RESULT(
            vkResetFences(
                renderer.device,
                1,
                &renderer.in_flight_fences[renderer.curr_frame]
                )
            )

        VULKAN_CHECK_RESULT(
            vkQueueSubmit(
                renderer.graphics_queue,
                1,
                &submit_info,
                renderer.in_flight_fences[renderer.curr_frame]
                )
            )

        VkSwapchainKHR swap_chains[] =
        {
            renderer.swap_chain,
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
            renderer.present_queue,
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

        renderer.curr_frame = (renderer.curr_frame + 1) % VULKAN_MAX_FRAMES_IN_FLIGHT;

        lna::memory_pool_empty(
            renderer.memory_pools[renderer_api::FRAME_LIFETIME_MEMORY_POOL]
            );
    }

    void renderer_release(
        renderer_api& renderer
        )
    {
        LNA_ASSERT(renderer.device);
        VULKAN_CHECK_RESULT(
            vkDeviceWaitIdle(
                renderer.device
                )
            )
        vulkan_cleanup_swap_chain(renderer);

        for (uint32_t i = 0; i < renderer.texture_system.cur_texture_count; ++i)
        {
            vulkan_texture_release(
                renderer.texture_system.textures[i],
                renderer.device
                );
        }
        
        vkDestroyDescriptorSetLayout(
            renderer.device,
            renderer.mesh_system.descriptor_set_layout,
            nullptr
            );

        for (uint32_t i = 0; i < renderer.mesh_system.cur_mesh_count; ++i)
        {
            vulkan_mesh_release(
                renderer.mesh_system.meshes[i],
                renderer.device
                );
        }

        for (size_t i = 0; i < VULKAN_MAX_FRAMES_IN_FLIGHT; ++i)
        {
            vkDestroySemaphore(
                renderer.device,
                renderer.render_finished_semaphores[i],
                nullptr
                );
            vkDestroySemaphore(
                renderer.device,
                renderer.image_available_semaphores[i],
                nullptr
                );
            vkDestroyFence(
                renderer.device,
                renderer.in_flight_fences[i],
                nullptr
                );
        }
        vkDestroyCommandPool(
            renderer.device,
            renderer.command_pool,
            nullptr    
            );
        vkDestroyDevice(
            renderer.device,
            nullptr
            );
        if (renderer.debug_messenger)
        {
            vulkan_destroy_debug_utilis_messenger_EXT(
                renderer.instance,
                renderer.debug_messenger,
                nullptr
                );
        }
        vkDestroySurfaceKHR(
            renderer.instance,
            renderer.surface,
            nullptr
            );
        vkDestroyInstance(
            renderer.instance,
            nullptr
            );

        for (uint32_t i = 0; i < renderer_api::MEMORY_POOL_COUNT; ++i)
        {
            memory_pool_free(
                renderer.memory_pools[i]
            );
        }
    }
}
