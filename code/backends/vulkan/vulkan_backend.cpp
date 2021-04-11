#include <cstring>
#include <algorithm>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wlanguage-extension-token"
#pragma warning(push, 0)
#include <SDL.h>
#pragma warning(pop)
#pragma clang diagnostic pop

#include "backends/vulkan/vulkan_backend.hpp"
#include "backends/vulkan/vulkan_helpers.hpp"
#include "backends/sdl/sdl_backend.hpp"
#include "backends/renderer_backend.hpp"
#include "core/assert.hpp"
#include "maths/mat4.hpp"

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
        lna::renderer_backend& renderer
        )
    {
        uint32_t layer_count;
        VULKAN_CHECK_RESULT(
            vkEnumerateInstanceLayerProperties(
                &layer_count,
                nullptr
                )
            )
        VkLayerProperties* available_layers = renderer.memory_pools[lna::renderer_backend::FRAME_LIFETIME_MEMORY_POOL].alloc<VkLayerProperties>(layer_count);

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
        lna::renderer_backend& renderer,
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
        VkQueueFamilyProperties* queue_families = renderer.memory_pools[lna::renderer_backend::FRAME_LIFETIME_MEMORY_POOL].alloc<VkQueueFamilyProperties>(queue_family_count);
        
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
        lna::renderer_backend& renderer,
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

        VkExtensionProperties* available_extensions = renderer.memory_pools[lna::renderer_backend::FRAME_LIFETIME_MEMORY_POOL].alloc<VkExtensionProperties>(extension_count);

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
        lna::renderer_backend& renderer,
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
            details.formats = renderer.memory_pools[lna::renderer_backend::FRAME_LIFETIME_MEMORY_POOL].alloc<VkSurfaceFormatKHR>(details.format_count);

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
            details.present_modes = renderer.memory_pools[lna::renderer_backend::FRAME_LIFETIME_MEMORY_POOL].alloc<VkPresentModeKHR>(details.present_mode_count);

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
        lna::renderer_backend& renderer,
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

        //! NOTE: we can pick a physical device without supported_features.samplerAnisotropy
        //! in this case we must indicate somewhere that the choosen physical device does not managed sampler anisotropy and when
        //! we will create a sampler create info we must check it and set to:
        //! sampler_create_info.anisotropyEnable    = VK_FALSE;
        //! sampler_create_info.maxAnisotropy       = 1.0f;
        //! see the end of https://vulkan-tutorial.com/Texture_mapping/Image_view_and_sampler for more information
        return
                vulkan_queue_family_indices_is_complete(indices)
            &&  supported_extensions
            &&  swap_chain_adequate
            &&  supported_features.samplerAnisotropy;
    }

    //! VULKAN INIT PROCESS FUNCTION -------------------------------------------

    bool vulkan_create_instance(
        lna::renderer_backend& renderer,
        const lna::renderer_backend_config& config
        )
    {
        LNA_ASSERT(renderer.instance == nullptr);
        LNA_ASSERT(config.window_ptr);

        if (
            config.enable_validation_layers
            && !vulkan_check_validation_layer_support(renderer)
            )
        {
            lna::log::error("cannot find valid Vulkan support GPU");
            return false;
        }

        VkApplicationInfo application_info {};
        application_info.sType                          = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        application_info.pApplicationName               = config.application_name ? config.application_name : "LNA FRAMEWORK";
        application_info.applicationVersion             = VK_MAKE_VERSION(config.application_major_ver, config.application_minor_ver, config.application_patch_ver);
        application_info.pEngineName                    = config.engine_name ? config.engine_name : "LNA FRAMEWORK";
        application_info.engineVersion                  = VK_MAKE_VERSION(config.engine_major_ver, config.engine_minor_ver, config.engine_patch_ver);

        const lna::window_extension_infos& extensions   = lna::window_backend_extensions(*config.window_ptr);

        VkInstanceCreateInfo instance_create_info {};
        instance_create_info.sType                      = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.pApplicationInfo           = &application_info;
        instance_create_info.enabledExtensionCount      = extensions.size();
        instance_create_info.ppEnabledExtensionNames    = extensions.ptr();

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

        return true;
    }

    void vulkan_setup_debug_messenger(
        lna::renderer_backend& renderer
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
        lna::renderer_backend& renderer,
        const lna::renderer_backend_config& config
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
        lna::renderer_backend& renderer
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

        VkPhysicalDevice* devices = renderer.memory_pools[lna::renderer_backend::FRAME_LIFETIME_MEMORY_POOL].alloc<VkPhysicalDevice>(device_count);

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
        lna::renderer_backend& renderer,
        const lna::renderer_backend_config& config
        )
    {
        LNA_ASSERT(renderer.physical_device);
        LNA_ASSERT(renderer.device == nullptr);
        LNA_ASSERT(renderer.graphics_queue == nullptr);
        LNA_ASSERT(renderer.graphics_family == (uint32_t)-1);

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

        VkDeviceQueueCreateInfo* queue_create_infos = renderer.memory_pools[lna::renderer_backend::FRAME_LIFETIME_MEMORY_POOL].alloc<VkDeviceQueueCreateInfo>(unique_queue_family_count);

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

        renderer.graphics_family = indices.graphics_family;

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
        lna::renderer_backend& renderer,
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

        //! simply sticking to this minimum means that we may sometimes have to wait on the driver to complete internal operations before we can acquire another image to render to.
        //! Therefore it is recommended to request at least one more image than the minimum:
        uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
        if (
            swap_chain_support.capabilities.maxImageCount > 0
            && image_count > swap_chain_support.capabilities.maxImageCount
            )
        {
            //! do not exceed the maximum number of image:
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

        renderer.swap_chain_images.init(
            image_count,
            renderer.memory_pools[lna::renderer_backend::SWAP_CHAIN_LIFETIME_MEMORY_POOL]
            );

        VULKAN_CHECK_RESULT(
            vkGetSwapchainImagesKHR(
                renderer.device,
                renderer.swap_chain,
                &image_count,
                renderer.swap_chain_images.ptr()
                )
            )
        renderer.swap_chain_image_format    = surface_format.format;
        renderer.swap_chain_extent          = extent;
    }

    void vulkan_create_image_views(
        lna::renderer_backend& renderer
        )
    {
        renderer.swap_chain_image_views.init(
            renderer.swap_chain_images.size(),
            renderer.memory_pools[lna::renderer_backend::SWAP_CHAIN_LIFETIME_MEMORY_POOL]
            );

        for (size_t i = 0; i < renderer.swap_chain_images.size(); ++i)
        {
            renderer.swap_chain_image_views[i] = lna::vulkan_helpers::create_image_view(
                renderer.device,
                renderer.swap_chain_images[i],
                renderer.swap_chain_image_format,
                VK_IMAGE_ASPECT_COLOR_BIT
                );
        }
    }

    void vulkan_create_render_pass(
        lna::renderer_backend& renderer
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

        VkAttachmentDescription depth_attachment{};
        depth_attachment.format                     = lna::vulkan_helpers::find_depth_format(renderer.physical_device);
        depth_attachment.samples                    = VK_SAMPLE_COUNT_1_BIT;
        depth_attachment.loadOp                     = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_attachment.storeOp                    = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.stencilLoadOp              = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depth_attachment.stencilStoreOp             = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.initialLayout              = VK_IMAGE_LAYOUT_UNDEFINED;
        depth_attachment.finalLayout                = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depth_attachment_reference{};
        depth_attachment_reference.attachment       = 1;
        depth_attachment_reference.layout           = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass_description{};
        subpass_description.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_description.colorAttachmentCount    = 1;
        subpass_description.pColorAttachments       = &color_attachment_reference;
        subpass_description.pDepthStencilAttachment = &depth_attachment_reference;

        VkSubpassDependency subpass_dependancy{};
        subpass_dependancy.srcSubpass               = VK_SUBPASS_EXTERNAL;
        subpass_dependancy.srcStageMask             = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        subpass_dependancy.srcAccessMask            = 0;
        subpass_dependancy.dstSubpass               = 0;
        subpass_dependancy.dstStageMask             = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        subpass_dependancy.dstAccessMask            = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        VkAttachmentDescription attachments[] =
        {
            color_attachment,
            depth_attachment
        };

        VkRenderPassCreateInfo render_pass_create_info{};
        render_pass_create_info.sType               = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_create_info.attachmentCount     = static_cast<uint32_t>(sizeof(attachments) / sizeof(attachments[0]));
        render_pass_create_info.pAttachments        = attachments;
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

    void vulkan_create_framebuffers(
        lna::renderer_backend& renderer
        )
    {
        LNA_ASSERT(renderer.device);

        renderer.swap_chain_framebuffers.init(
            renderer.swap_chain_images.size(),
            renderer.memory_pools[lna::renderer_backend::SWAP_CHAIN_LIFETIME_MEMORY_POOL]
            );
        
        for (size_t i = 0; i < renderer.swap_chain_images.size(); ++i)
        {
            VkImageView attachments[] =
            {
                renderer.swap_chain_image_views[i],
                renderer.depth_image_view
            };

            VkFramebufferCreateInfo framebuffer_create_info{};
            framebuffer_create_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_create_info.renderPass      = renderer.render_pass;
            framebuffer_create_info.attachmentCount = static_cast<uint32_t>(sizeof(attachments) / sizeof(attachments[0]));
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
        lna::renderer_backend& renderer
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
        command_pool_create_info.flags              = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        VULKAN_CHECK_RESULT(
            vkCreateCommandPool(
                renderer.device,
                &command_pool_create_info,
                nullptr,
                &renderer.command_pool
                )
            )
    }

    void vulkan_create_depth_resources(
        lna::renderer_backend& renderer
        )
    {
        VkFormat depth_format = lna::vulkan_helpers::find_depth_format(renderer.physical_device);
        LNA_ASSERT(depth_format != VK_FORMAT_UNDEFINED);

        lna::vulkan_helpers::create_image(
            renderer.device,
            renderer.physical_device,
            renderer.swap_chain_extent.width,
            renderer.swap_chain_extent.height,
            depth_format,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            renderer.depth_image,
            renderer.depth_image_memory
            );
        renderer.depth_image_view = lna::vulkan_helpers::create_image_view(
            renderer.device,
            renderer.depth_image,
            depth_format,
            VK_IMAGE_ASPECT_DEPTH_BIT
            );
    }

    void vulkan_create_command_buffers(
        lna::renderer_backend& renderer
        )
    {
        LNA_ASSERT(renderer.device);

        renderer.command_buffers.init(
            renderer.swap_chain_images.size(),
            renderer.memory_pools[lna::renderer_backend::SWAP_CHAIN_LIFETIME_MEMORY_POOL]  
            );

        VkCommandBufferAllocateInfo command_buffer_allocate_info{}; 
        command_buffer_allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        command_buffer_allocate_info.commandPool        = renderer.command_pool;
        command_buffer_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        command_buffer_allocate_info.commandBufferCount = renderer.command_buffers.size();

        VULKAN_CHECK_RESULT(
            vkAllocateCommandBuffers(
                renderer.device,
                &command_buffer_allocate_info,
                renderer.command_buffers.ptr()
                )
            )
    }

    void vulkan_create_sync_objects(
        lna::renderer_backend& renderer
        )
    {
        LNA_ASSERT(renderer.device);

        renderer.images_in_flight_fences.init(
            renderer.swap_chain_images.size(),
            renderer.memory_pools[lna::renderer_backend::PERSISTENT_LIFETIME_MEMORY_POOL]
            );

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

    void vulkan_cleanup_swap_chain(
        lna::renderer_backend& renderer
        )
    {
        LNA_ASSERT(renderer.device);

        vkDestroyImageView(renderer.device, renderer.depth_image_view, nullptr);
        vkDestroyImage(renderer.device, renderer.depth_image, nullptr);
        vkFreeMemory(renderer.device, renderer.depth_image_memory, nullptr);

        for (auto& swap_chain_framebuffer : renderer.swap_chain_framebuffers)
        {
            vkDestroyFramebuffer(
                renderer.device,
                swap_chain_framebuffer,
                nullptr
                );
        }

        vkFreeCommandBuffers(
            renderer.device,
            renderer.command_pool,
            renderer.command_buffers.size(),
            renderer.command_buffers.ptr()
            );

        for (uint32_t i = 0; i < lna::renderer_backend::MAX_SWAP_CHAIN_CALLBACKS; ++i)
        {
            if (
                renderer.swap_chain_cleanup_callbacks[i]
                && renderer.callback_owners[i]
                )
            {
                renderer.swap_chain_cleanup_callbacks[i](
                    renderer.callback_owners[i]
                    );
            }
        }

        vkDestroyRenderPass(
            renderer.device,
            renderer.render_pass,
            nullptr
            );

        for (auto& swap_chain_image_view : renderer.swap_chain_image_views)
        {
            vkDestroyImageView(
                renderer.device,
                swap_chain_image_view,
                nullptr
                );
        }

        vkDestroySwapchainKHR(
            renderer.device,
            renderer.swap_chain,
            nullptr
            );

        renderer.memory_pools[lna::renderer_backend::SWAP_CHAIN_LIFETIME_MEMORY_POOL].empty();

        renderer.swap_chain_images.release();
        renderer.swap_chain_image_views.release();
        renderer.swap_chain_framebuffers.release();
        renderer.command_buffers.release();
    }

    void vulkan_recreate_swap_chain(
        lna::renderer_backend& renderer,
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
        vulkan_create_depth_resources(renderer);
        vulkan_create_framebuffers(renderer);
        vulkan_create_command_buffers(renderer);

        for (uint32_t i = 0; i < lna::renderer_backend::MAX_SWAP_CHAIN_CALLBACKS; ++i)
        {
            if (
                renderer.swap_chain_recreate_callbacks[i]
                && renderer.callback_owners[i]
                )
            {
                renderer.swap_chain_recreate_callbacks[i](
                    renderer.callback_owners[i]
                    );
            }
        }
    }
}

namespace lna
{
    void vulkan_renderer_backend_register_swap_chain_callbacks(
        renderer_backend&               backend,
        vulkan_on_swap_chain_cleanup    on_clean_up,
        vulkan_on_swap_chain_recreate   on_recreate,
        vulkan_on_draw                  on_draw,
        void*                           owner
        )
    {
        for (uint32_t i = 0; i < renderer_backend::MAX_SWAP_CHAIN_CALLBACKS; ++i)
        {
            if (
                backend.swap_chain_cleanup_callbacks[i] == nullptr
                && backend.swap_chain_recreate_callbacks[i] == nullptr
                && backend.callback_owners[i] == nullptr
                && backend.draw_callbacks[i] == nullptr
                )
            {
                backend.swap_chain_cleanup_callbacks[i]     = on_clean_up;
                backend.swap_chain_recreate_callbacks[i]    = on_recreate;
                backend.draw_callbacks[i]                   = on_draw;
                backend.callback_owners[i]                  = owner;
                return;
            }
        }
        LNA_ASSERT(0);
    }

    bool renderer_backend_configure(
        renderer_backend& renderer,
        const renderer_backend_config& config
        )
    {
        LNA_ASSERT(renderer.instance == nullptr);
        LNA_ASSERT(renderer.debug_messenger == nullptr);
        LNA_ASSERT(renderer.physical_device == nullptr);
        LNA_ASSERT(renderer.device == nullptr);
        LNA_ASSERT(renderer.graphics_family == 0);
        LNA_ASSERT(renderer.graphics_queue == nullptr);
        LNA_ASSERT(renderer.surface == nullptr);
        LNA_ASSERT(renderer.present_queue == nullptr);
        LNA_ASSERT(renderer.swap_chain == nullptr);
        LNA_ASSERT(renderer.render_pass == nullptr);
        LNA_ASSERT(renderer.command_pool == nullptr);
        LNA_ASSERT(renderer.curr_frame == 0);
        LNA_ASSERT(renderer.images_in_flight_fences.ptr() == nullptr);
        LNA_ASSERT(renderer.swap_chain_images.ptr() == nullptr);
        LNA_ASSERT(renderer.swap_chain_image_views.ptr() == nullptr);
        LNA_ASSERT(renderer.swap_chain_framebuffers.ptr() == nullptr);
        LNA_ASSERT(renderer.command_buffers.ptr() == nullptr);
        LNA_ASSERT(config.window_ptr);
        LNA_ASSERT(config.allocator_ptr);

        for (uint32_t i = 0; i < renderer_backend::MAX_SWAP_CHAIN_CALLBACKS; ++i)
        {
            renderer.swap_chain_cleanup_callbacks[i]    = nullptr;
            renderer.swap_chain_recreate_callbacks[i]   = nullptr;
            renderer.draw_callbacks[i]                  = nullptr;
            renderer.callback_owners[i]                 = nullptr;
        }

        renderer.curr_frame = 0;
        renderer.graphics_family = (uint32_t)-1;
        if (!vulkan_create_instance(renderer, config))
        {
            return false;
        }
        for (uint32_t i = 0; i < renderer_backend::MEMORY_POOL_COUNT; ++i)
        {
            renderer.memory_pools[i].init(LNA_MEGABYTES(MEMORY_POOL_SIZES[i]), *config.allocator_ptr);
        }
        if (config.enable_validation_layers)
        {
            vulkan_setup_debug_messenger(renderer);
        }
        vulkan_create_surface(renderer, config);
        vulkan_pick_physical_device(renderer);
        vulkan_create_logical_device(renderer, config);
        vulkan_create_swap_chain(renderer, lna::window_backend_width(*config.window_ptr), lna::window_backend_height(*config.window_ptr));
        vulkan_create_image_views(renderer);
        vulkan_create_render_pass(renderer);
        vulkan_create_command_pool(renderer);
        vulkan_create_depth_resources(renderer);
        vulkan_create_framebuffers(renderer);
        vulkan_create_command_buffers(renderer);
        vulkan_create_sync_objects(renderer);

        return true;
    }

    void renderer_backend_draw_frame(
        renderer_backend& renderer,
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

        for (size_t i = 0; i < renderer.swap_chain_images.size(); ++i)
        {
            VkCommandBufferBeginInfo    command_buffer_begin_info{};
            command_buffer_begin_info.sType             = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            command_buffer_begin_info.flags             = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            command_buffer_begin_info.pInheritanceInfo  = nullptr;

            VULKAN_CHECK_RESULT(
                vkResetCommandBuffer(
                    renderer.command_buffers[i],
                    VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT
                    )
                )

            VULKAN_CHECK_RESULT(
                vkBeginCommandBuffer(
                    renderer.command_buffers[i],
                    &command_buffer_begin_info
                    )
                )

            VkRenderPassBeginInfo render_pass_begin_info{};
            render_pass_begin_info.sType                = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            render_pass_begin_info.renderPass           = renderer.render_pass;
            render_pass_begin_info.framebuffer          = renderer.swap_chain_framebuffers[i];
            render_pass_begin_info.renderArea.offset    = { 0, 0 };
            render_pass_begin_info.renderArea.extent    = renderer.swap_chain_extent;

            VkClearValue clear_values[2]{};
            clear_values[0].color           = {{ 0.0f, 0.0f, 0.0f, 1.0f }};
            clear_values[1].depthStencil    = { 1.0f, 0 };

            render_pass_begin_info.clearValueCount      = 2;
            render_pass_begin_info.pClearValues         = clear_values;

            vkCmdBeginRenderPass(
                renderer.command_buffers[i],
                &render_pass_begin_info,
                VK_SUBPASS_CONTENTS_INLINE
                );

            VkViewport viewport;
            viewport.width      = renderer.swap_chain_extent.width;
		    viewport.height     = renderer.swap_chain_extent.height;
		    viewport.minDepth   = 0.0f;
		    viewport.maxDepth   = 1.0f;
            vkCmdSetViewport(
                renderer.command_buffers[i],
                0,
                1,
                &viewport
                );

            VkRect2D scissor_rect;
            scissor_rect.offset.x = 0;
            scissor_rect.offset.y = 0;
            scissor_rect.extent.width   = renderer.swap_chain_extent.width;
            scissor_rect.extent.height  = renderer.swap_chain_extent.height;
            vkCmdSetScissor(
                renderer.command_buffers[i],
                0,
                1,
                &scissor_rect
                );

            for (uint32_t j = 0; j < renderer_backend::MAX_SWAP_CHAIN_CALLBACKS; ++j)
            {
                if (
                    renderer.draw_callbacks[j]
                    && renderer.callback_owners[j]
                    )
                {
                    renderer.draw_callbacks[j](renderer.callback_owners[j], i);
                }
            }

            vkCmdEndRenderPass(
                renderer.command_buffers[i]
                );
            VULKAN_CHECK_RESULT(
                vkEndCommandBuffer(
                    renderer.command_buffers[i]
                    )
                )
        }

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

        renderer.memory_pools[renderer_backend::FRAME_LIFETIME_MEMORY_POOL].empty();
    }

    void renderer_backend_release(
        renderer_backend& renderer
        )
    {
        LNA_ASSERT(renderer.device);
        VULKAN_CHECK_RESULT(
            vkDeviceWaitIdle(
                renderer.device
                )
            )

        vulkan_cleanup_swap_chain(renderer);

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
    }

    uint32_t renderer_memory_pool_count()
    {
        return renderer_backend::MEMORY_POOL_COUNT;
    }
}
