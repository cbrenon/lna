#include <string.h>
#include "graphics/lna_renderer.h"
#include "backends/vulkan/lna_renderer_vulkan.h"
#include "backends/vulkan/lna_vulkan.h"
#include "system/lna_window.h"
#include "backends/sdl/lna_window_sdl.h"
#include "maths/lna_maths.h"
#include "core/lna_heap_allocator.h"
#include "core/lna_version.h"
#include "core/lna_memory.h"

//! ============================================================================
//!                             LOCAL CONST
//! ============================================================================

static const char* LNA_VULKAN_VALIDATION_LAYERS[] =
{
    "VK_LAYER_KHRONOS_validation",
};

static const char* LNA_VULKAN_DEVICE_EXTENSIONS[] =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};  

static const size_t LNA_VULKAN_RENDERER_DEFAULT_MEMORY_POOL_SIZES[LNA_VULKAN_RENDERER_MEMORY_POOL_COUNT] =
{
    256LL * 1024LL * 1024LL,
    256LL * 1024LL * 1024LL,
    256LL * 1024LL * 1024LL,
};

//! ============================================================================
//!                             LOCAL STRUCT
//! ============================================================================

typedef struct lna_vulkan_queue_family_indices_s
{
    uint32_t    graphics_family;
    uint32_t    present_family;
} lna_vulkan_queue_family_indices_t;

typedef struct lna_vulkan_swap_chain_support_details_s
{
    VkSurfaceCapabilitiesKHR    capabilities;
    VkSurfaceFormatKHR*         formats;
    uint32_t                    format_count;
    VkPresentModeKHR*           present_modes;
    uint32_t                    present_mode_count;
} lna_vulkan_swap_chain_support_details_t;

//! ============================================================================
//!                         VULKAN DEBUG CALLBACKS
//! ============================================================================

static VKAPI_ATTR VkBool32 VKAPI_CALL lna_vulkan_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT *call_back_data_ptr,
    void *user_data_ptr
    )
{
    lna_assert(call_back_data_ptr)

    (void)message_type;
    (void)user_data_ptr;

    if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        lna_log_error("vulkan validation layer: %s", call_back_data_ptr->pMessage);
        //lna_assert(message_severity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        lna_assert(0)
    }
    return VK_FALSE;
}

static VkResult lna_vulkan_create_debug_utils_messenger_EXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *create_info_ptr,
    const VkAllocationCallbacks *allocator_ptr,
    VkDebugUtilsMessengerEXT *debug_messenger_ptr
    )
{
    lna_assert(instance)

    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance,
        "vkCreateDebugUtilsMessengerEXT"
        );
    if (func != NULL)
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

static void lna_vulkan_destroy_debug_utilis_messenger_EXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debug_messenger,
    const VkAllocationCallbacks *allocator_ptr
    )
{
    lna_assert(instance)

    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
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

//! ============================================================================
//!                         VULKAN LOCAL HELPERS
//! ============================================================================

static bool lna_vulkan_queue_family_indices_is_complete(const lna_vulkan_queue_family_indices_t* indices)
{
    lna_assert(indices)

    return indices->graphics_family != (uint32_t)-1
        && indices->present_family != (uint32_t)-1;
}

static bool lna_vulkan_check_validation_layer_support(lna_memory_pool_t* memory_pool)
{
    uint32_t layer_count;
    lna_vulkan_check(
        vkEnumerateInstanceLayerProperties(
            &layer_count,
            NULL
            )
        );
    VkLayerProperties* available_layers = lna_memory_pool_reserve(
        memory_pool,
        sizeof(VkLayerProperties) * layer_count
        );

    lna_vulkan_check(
        vkEnumerateInstanceLayerProperties(
            &layer_count,
            available_layers
            )
        );

    size_t validation_layer_count = sizeof(LNA_VULKAN_VALIDATION_LAYERS) / sizeof(LNA_VULKAN_VALIDATION_LAYERS[0]);
    for (size_t i = 0; i < validation_layer_count; ++i)
    {
        bool layer_found = false;
        for (uint32_t j = 0; j < layer_count; ++j)
        {
            if (strcmp(LNA_VULKAN_VALIDATION_LAYERS[i], available_layers[j].layerName) == 0)
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

static lna_vulkan_queue_family_indices_t lna_vulkan_find_queue_families(
    lna_memory_pool_t* memory_pool,
    VkPhysicalDevice physical_device,
    VkSurfaceKHR surface
    )
{
    lna_assert(memory_pool)
    lna_assert(physical_device)
    lna_assert(surface)

    lna_vulkan_queue_family_indices_t indices =
    {
        .graphics_family    = (uint32_t)-1,
        .present_family     = (uint32_t)-1
    };
    uint32_t queue_family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(
        physical_device,
        &queue_family_count,
        NULL
        );

    VkQueueFamilyProperties* queue_families = lna_memory_pool_reserve(
        memory_pool,
        sizeof(VkQueueFamilyProperties) * queue_family_count
        );

    vkGetPhysicalDeviceQueueFamilyProperties(
        physical_device,
        &queue_family_count,
        queue_families);

    for (uint32_t i = 0; i < queue_family_count; ++i)
    {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphics_family = i;
        }

        VkBool32 present_support = false;
        lna_vulkan_check(
            vkGetPhysicalDeviceSurfaceSupportKHR(
                physical_device,
                i,
                surface,
                &present_support
                )
            );
        if (present_support)
        {
            indices.present_family = i;
        }

        if (lna_vulkan_queue_family_indices_is_complete(&indices))
        {
            break;
        }
    }
    return indices;
}

static bool lna_vulkan_check_device_extension_support(
    lna_memory_pool_t* memory_pool,
    VkPhysicalDevice physical_device
    )
{
    lna_assert(memory_pool)
    lna_assert(physical_device)

    uint32_t extension_count;
    lna_vulkan_check(
        vkEnumerateDeviceExtensionProperties(
            physical_device,
            NULL,
            &extension_count,
            NULL
            )
        );

    VkExtensionProperties *available_extensions = lna_memory_pool_reserve(
        memory_pool,
        sizeof(VkExtensionProperties) *  extension_count
        );

    lna_vulkan_check(
        vkEnumerateDeviceExtensionProperties(
            physical_device,
            NULL,
            &extension_count,
            available_extensions
            )
        );

    size_t required_extension_count = sizeof(LNA_VULKAN_DEVICE_EXTENSIONS) / sizeof(LNA_VULKAN_DEVICE_EXTENSIONS[0]);
    for (size_t i = 0; i < required_extension_count; ++i)
    {
        bool extension_found = false;
        for (uint32_t j = 0; j < extension_count; ++j)
        {

            if (strcmp(LNA_VULKAN_DEVICE_EXTENSIONS[i], available_extensions[j].extensionName) == 0)
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

static lna_vulkan_swap_chain_support_details_t lna_vulkan_query_swap_chain_support(
    lna_memory_pool_t* memory_pool,
    VkPhysicalDevice physical_device,
    VkSurfaceKHR surface
    )
{
    lna_assert(memory_pool)
    lna_assert(physical_device)
    lna_assert(surface)

    lna_vulkan_swap_chain_support_details_t details;
    lna_vulkan_check(
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            physical_device,
            surface,
            &details.capabilities
            )
        );

    lna_vulkan_check(
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            physical_device,
            surface,
            &details.format_count,
            NULL
            )
        );
    if (details.format_count > 0)
    {
        details.formats = lna_memory_pool_reserve(
            memory_pool,
            sizeof(VkSurfaceFormatKHR) * details.format_count
            );

        lna_vulkan_check(
            vkGetPhysicalDeviceSurfaceFormatsKHR(
                physical_device,
                surface,
                &details.format_count,
                details.formats
                )
            );
    }

    lna_vulkan_check(
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            physical_device,
            surface,
            &details.present_mode_count,
            NULL
            )
        );
    if (details.present_mode_count > 0)
    {
        details.present_modes = lna_memory_pool_reserve(
            memory_pool,
            sizeof(VkPresentModeKHR) * details.present_mode_count
            );

        lna_vulkan_check(
            vkGetPhysicalDeviceSurfacePresentModesKHR(
                physical_device,
                surface,
                &details.present_mode_count,
                details.present_modes
                )
            );
    }
    return details;
}

static VkSurfaceFormatKHR lna_vulkan_choose_swap_surface_format(
    const VkSurfaceFormatKHR* available_formats,
    uint32_t available_format_count
    )
{
    lna_assert(available_formats)

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

static VkPresentModeKHR lna_vulkan_choose_swap_present_mode(
    const VkPresentModeKHR* available_present_modes,
    uint32_t available_present_mode_count
    )
{
    lna_assert(available_present_modes)

    for (uint32_t i = 0; i < available_present_mode_count; ++i)
    {
        if (available_present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return available_present_modes[i];
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D lna_vulkan_choose_swap_extent(
    const VkSurfaceCapabilitiesKHR* capabilities,
    uint32_t framebuffer_width,
    uint32_t framebuffer_height
    )
{
    lna_assert(capabilities)

    if (capabilities->currentExtent.width != UINT32_MAX)
    {
        return capabilities->currentExtent;
    }
    VkExtent2D actual_extent =
    {
        lna_clamp_uint32(
            framebuffer_width,
            capabilities->minImageExtent.width,
            capabilities->maxImageExtent.width
            ),
        lna_clamp_uint32(
            framebuffer_height,
            capabilities->minImageExtent.height,
            capabilities->maxImageExtent.height
            ),
    };
    return actual_extent;
}

static bool lna_vulkan_is_physical_device_suitable(
    lna_memory_pool_t* memory_pool,
    VkPhysicalDevice physical_device,
    VkSurfaceKHR surface
    )
{
    lna_assert(memory_pool)
    lna_assert(physical_device)
    lna_assert(surface)

    lna_vulkan_queue_family_indices_t indices = lna_vulkan_find_queue_families(
        memory_pool,
        physical_device,
        surface
        );

    bool supported_extensions = lna_vulkan_check_device_extension_support(
        memory_pool,
        physical_device
        );
    bool swap_chain_adequate = false;
    if (supported_extensions)
    {
        lna_vulkan_swap_chain_support_details_t swap_chain_support = lna_vulkan_query_swap_chain_support(
            memory_pool,
            physical_device,
            surface
            );

        swap_chain_adequate =
                swap_chain_support.format_count != 0
            &&  swap_chain_support.present_mode_count != 0;
    }
    VkPhysicalDeviceFeatures supported_features;
    vkGetPhysicalDeviceFeatures(
        physical_device,
        &supported_features
        );
    //! NOTE: we can pick a physical device without supported_features.samplerAnisotropy
    //! in this case we must indicate somewhere that the choosen physical device does not managed sampler anisotropy and when
    //! we will create a sampler create info we must check it and set to:
    //! sampler_create_info.anisotropyEnable    = VK_FALSE;
    //! sampler_create_info.maxAnisotropy       = 1.0f;
    //! see the end of https://vulkan-tutorial.com/Texture_mapping/Image_view_and_sampler for more information
    return
            lna_vulkan_queue_family_indices_is_complete(&indices)
        &&  supported_extensions
        &&  swap_chain_adequate
        &&  supported_features.samplerAnisotropy;
}

//! ============================================================================
//!                     VULKAN RENDERER LOCAL FUNCTIONS
//! ============================================================================

static bool lna_vulkan_renderer_create_instance(
    lna_renderer_t* renderer,
    const lna_window_t* window,
    bool enable_validation_layers
    )
{
    lna_assert(renderer)
    lna_assert(renderer->instance == NULL)
    lna_assert(window)

    if (
        enable_validation_layers
        && !lna_vulkan_check_validation_layer_support(&renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_FRAME])
        )
    {
        lna_log_error("cannot find valid Vulkan support GPU");
        return false;
    }

    VkApplicationInfo application_info =
    {
        .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName   = "LNA FRAMEWORK",
        .applicationVersion = VK_MAKE_VERSION(LNA_ENGINE_MAJOR_VERSION_NUMBER, LNA_ENGINE_MINOR_VERSION_NUMBER, LNA_ENGINE_PATCH_VERSION_NUMBER),
        .pEngineName        = "LNA FRAMEWORK",
        .engineVersion      = VK_MAKE_VERSION(LNA_ENGINE_MAJOR_VERSION_NUMBER, LNA_ENGINE_MINOR_VERSION_NUMBER, LNA_ENGINE_PATCH_VERSION_NUMBER),
    };
    

    uint32_t extension_count;
    lna_window_vulkan_extension_count(window, &extension_count);
    const char** extension_names = lna_memory_pool_reserve(
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_FRAME],
        sizeof(const char*) * (enable_validation_layers ? extension_count + 1 : extension_count)
        );
    lna_window_vulkan_extension_names(
        window,
        &extension_count,
        extension_names
        );
    if (enable_validation_layers)
    {
        extension_names[extension_count] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }

    const VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info =
    {
        .sType              = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity    = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType        = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback    = lna_vulkan_debug_callback,
    };

    const VkInstanceCreateInfo instance_create_info =
    {
        .sType                      = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo           = &application_info,
        .enabledExtensionCount      = enable_validation_layers ? extension_count + 1 : extension_count,
        .ppEnabledExtensionNames    = extension_names,
        .enabledLayerCount          = enable_validation_layers ? (uint32_t)(sizeof(LNA_VULKAN_VALIDATION_LAYERS) / sizeof(LNA_VULKAN_VALIDATION_LAYERS[0])) : 0,
        .ppEnabledLayerNames        = enable_validation_layers ? LNA_VULKAN_VALIDATION_LAYERS : NULL,
        .pNext                      = enable_validation_layers ? &debug_messenger_create_info : NULL,
    };

    lna_vulkan_check(
        vkCreateInstance(
            &instance_create_info,
            NULL,
            &renderer->instance)
            );

    return true;
}

static void lna_vulkan_renderer_setup_debug_messenger(
    lna_renderer_t* renderer
    )
{
    lna_assert(renderer)
    lna_assert(renderer->instance)
    lna_assert(renderer->debug_messenger == NULL)

    const VkDebugUtilsMessengerCreateInfoEXT debug_messenger_create_info =
    {
        .sType              = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity    = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType        = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback    = lna_vulkan_debug_callback,
    };

    lna_vulkan_check(
        lna_vulkan_create_debug_utils_messenger_EXT(
            renderer->instance,
            &debug_messenger_create_info,
            NULL,
            &renderer->debug_messenger
            )
        );
}

static void lna_vulkan_renderer_create_surface(
    lna_renderer_t* renderer,
    const lna_window_t* window
    )
{
    lna_assert(renderer)
    lna_assert(renderer->surface == NULL)
    lna_assert(renderer->instance)
    lna_assert(window)

    lna_window_vulkan_create_surface(
        window,
        renderer->instance,
        &renderer->surface
        );
}

static void lna_vulkan_renderer_pick_physical_device(
    lna_renderer_t* renderer
    )
{
    lna_assert(renderer)
    lna_assert(renderer->instance)
    lna_assert(renderer->surface)
    lna_assert(renderer->physical_device == NULL)

    uint32_t device_count;
    lna_vulkan_check(
        vkEnumeratePhysicalDevices(
            renderer->instance,
            &device_count,
            NULL
            )
        );
    lna_assert(device_count > 0)

    VkPhysicalDevice* devices = lna_memory_pool_reserve(
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_FRAME],
        sizeof(VkPhysicalDevice) * device_count
        );

    lna_vulkan_check(
        vkEnumeratePhysicalDevices(
            renderer->instance,
            &device_count,
            devices
            )
        );

    for (uint32_t i = 0; i < device_count; ++i)
    {
        if (
            lna_vulkan_is_physical_device_suitable(
                &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_FRAME],
                devices[i],
                renderer->surface
                )
            )
        {
            renderer->physical_device = devices[i];
            break;
        }
    }
}

static void lna_vulkan_renderer_create_logical_device(
    lna_renderer_t* renderer,
    bool enable_validation_layers
    )
{
    lna_assert(renderer)
    lna_assert(renderer->physical_device)
    lna_assert(renderer->surface)
    lna_assert(renderer->device == NULL)
    lna_assert(renderer->graphics_queue == NULL)
    lna_assert(renderer->present_queue == NULL)
    lna_assert(renderer->graphics_family == (uint32_t)-1)

    lna_vulkan_queue_family_indices_t indices = lna_vulkan_find_queue_families(
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_FRAME],
        renderer->physical_device,
        renderer->surface
        );

    uint32_t unique_queue_families[] =
    {
        indices.graphics_family,
        indices.present_family
    };
    uint32_t unique_queue_family_count = (indices.graphics_family == indices.present_family) ? 1 : 2;

    VkDeviceQueueCreateInfo *queue_create_infos = lna_memory_pool_reserve(
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_FRAME],
        sizeof(VkDeviceQueueCreateInfo) * unique_queue_family_count
        );

    float queue_priority = 1.0f;
    for (size_t i = 0; i < unique_queue_family_count; ++i)
    {
        queue_create_infos[i].sType             = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[i].queueFamilyIndex  = unique_queue_families[i];
        queue_create_infos[i].queueCount        = 1;
        queue_create_infos[i].pQueuePriorities  = &queue_priority;
    }

    const VkPhysicalDeviceFeatures device_features =
    {
        .samplerAnisotropy = VK_TRUE,
        .fillModeNonSolid = VK_TRUE,
        .wideLines = VK_TRUE,
    };

    const VkDeviceCreateInfo device_create_info =
    {
        .sType                      = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos          = queue_create_infos,
        .queueCreateInfoCount       = unique_queue_family_count,
        .pEnabledFeatures           = &device_features,
        .enabledExtensionCount      = (uint32_t)(sizeof(LNA_VULKAN_DEVICE_EXTENSIONS) / sizeof(LNA_VULKAN_DEVICE_EXTENSIONS[0])),
        .ppEnabledExtensionNames    = LNA_VULKAN_DEVICE_EXTENSIONS,
        .enabledLayerCount          = enable_validation_layers ? (uint32_t)(sizeof(LNA_VULKAN_VALIDATION_LAYERS) / sizeof(LNA_VULKAN_VALIDATION_LAYERS[0])) : 0,
        .ppEnabledLayerNames        = enable_validation_layers ? LNA_VULKAN_VALIDATION_LAYERS : NULL,
    };

    lna_vulkan_check(
        vkCreateDevice(
            renderer->physical_device,
            &device_create_info,
            NULL,
            &renderer->device
            )
        );

    renderer->graphics_family = indices.graphics_family;
    vkGetDeviceQueue(
        renderer->device,
        indices.graphics_family,
        0,
        &renderer->graphics_queue
        );
    vkGetDeviceQueue(
        renderer->device,
        indices.present_family,
        0,
        &renderer->present_queue
        );
}

static void lna_vulkan_renderer_create_swap_chain(
    lna_renderer_t* renderer,
    uint32_t framebuffer_width,
    uint32_t framebuffer_height
    )
{
    lna_assert(renderer)
    lna_assert(renderer->device)
    lna_assert(renderer->swap_chain == NULL)

    lna_vulkan_swap_chain_support_details_t swap_chain_support = lna_vulkan_query_swap_chain_support(
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_FRAME],
        renderer->physical_device,
        renderer->surface
        );

    VkSurfaceFormatKHR  surface_format  = lna_vulkan_choose_swap_surface_format(swap_chain_support.formats, swap_chain_support.format_count);
    VkPresentModeKHR    present_mode    = lna_vulkan_choose_swap_present_mode(swap_chain_support.present_modes, swap_chain_support.present_mode_count);
    VkExtent2D          extent          = lna_vulkan_choose_swap_extent(&swap_chain_support.capabilities, framebuffer_width, framebuffer_height);

    //! simply sticking to this minimum means that we may sometimes have to wait on the driver to complete internal operations before we can acquire another image to render to.
    //! Therefore it is recommended to request at least one more image than the minimum:
    uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
    if (
        swap_chain_support.capabilities.maxImageCount > 0 && image_count > swap_chain_support.capabilities.maxImageCount)
    {
        //! do not exceed the maximum number of image:
        image_count = swap_chain_support.capabilities.maxImageCount;
    }

    lna_vulkan_queue_family_indices_t indices = lna_vulkan_find_queue_families(
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_FRAME],
        renderer->physical_device,
        renderer->surface
        );
    uint32_t queue_family_indices[] =
    {
        indices.graphics_family,
        indices.present_family
    };

    const VkSwapchainCreateInfoKHR swap_chain_create_info =
    {
        .sType                  = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface                = renderer->surface,
        .minImageCount          = image_count,
        .imageFormat            = surface_format.format,
        .imageColorSpace        = surface_format.colorSpace,
        .imageExtent            = extent,
        .imageArrayLayers       = 1,
        .imageUsage             = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode       = (indices.graphics_family != indices.present_family) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount  = (indices.graphics_family != indices.present_family) ? 2 : 0,
        .pQueueFamilyIndices    = (indices.graphics_family != indices.present_family) ? queue_family_indices : NULL,
        .preTransform           = swap_chain_support.capabilities.currentTransform,
        .compositeAlpha         = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode            = present_mode,
        .clipped                = VK_TRUE,
        .oldSwapchain           = VK_NULL_HANDLE,
    };
    lna_vulkan_check(
        vkCreateSwapchainKHR(
            renderer->device,
            &swap_chain_create_info,
            NULL,
            &renderer->swap_chain
            )
        );
    lna_vulkan_check(
        vkGetSwapchainImagesKHR(
            renderer->device,
            renderer->swap_chain,
            &image_count,
            NULL
            )
        );
    lna_assert(image_count > 0)

    renderer->swap_chain_images.count       = image_count;
    renderer->swap_chain_images.elements    = lna_memory_pool_reserve(
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_SWAP_CHAIN],
        sizeof(VkImage) * image_count
        );

    lna_vulkan_check(
        vkGetSwapchainImagesKHR(
            renderer->device,
            renderer->swap_chain,
            &renderer->swap_chain_images.count,
            renderer->swap_chain_images.elements
            )
        );
    renderer->swap_chain_image_format   = surface_format.format;
    renderer->swap_chain_extent         = extent;

    renderer->images_in_flight_fences.count     = renderer->swap_chain_images.count;
    renderer->images_in_flight_fences.elements  = lna_memory_pool_reserve(
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_SWAP_CHAIN],
        sizeof(VkFence) * renderer->swap_chain_images.count
        );
}

static void lna_vulkan_renderer_create_image_views(
    lna_renderer_t* renderer
    )

{
    lna_assert(renderer)
    lna_assert(renderer->swap_chain_image_views.count == 0)
    lna_assert(renderer->swap_chain_image_views.elements == NULL)
    lna_assert(renderer->swap_chain_images.count > 0)

    renderer->swap_chain_image_views.count      = renderer->swap_chain_images.count;
    renderer->swap_chain_image_views.elements   = lna_memory_pool_reserve(
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_SWAP_CHAIN],
        sizeof(VkImageView) * renderer->swap_chain_images.count
        );

    for (size_t i = 0; i < renderer->swap_chain_image_views.count; ++i)
    {
        renderer->swap_chain_image_views.elements[i] = lna_vulkan_create_image_view(
            renderer->device,
            renderer->swap_chain_images.elements[i],
            renderer->swap_chain_image_format,
            VK_IMAGE_ASPECT_COLOR_BIT
            );
    }
}

static void lna_vulkan_renderer_create_render_pass(
    lna_renderer_t* renderer
    )
{
    lna_assert(renderer)
    lna_assert(renderer->device)

    const VkAttachmentDescription color_attachment =
    {
        .format         = renderer->swap_chain_image_format,
        .samples        = VK_SAMPLE_COUNT_1_BIT,
        .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    const VkAttachmentReference color_attachment_reference =
    {
        .attachment = 0,
        .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    const VkAttachmentDescription depth_attachment =
    {
        .format         = lna_vulkan_find_depth_format(renderer->physical_device),
        .samples        = VK_SAMPLE_COUNT_1_BIT,
        .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    const VkAttachmentReference depth_attachment_reference =
    {
        .attachment = 1,
        .layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    const VkSubpassDescription subpass_description =
    {
        .pipelineBindPoint          = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount       = 1,
        .pColorAttachments          = &color_attachment_reference,
        .pDepthStencilAttachment    = &depth_attachment_reference,
    };

    const VkSubpassDependency subpass_dependancy =
    {
        .srcSubpass     = VK_SUBPASS_EXTERNAL,
        .srcStageMask   = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .srcAccessMask  = 0,
        .dstSubpass     = 0,
        .dstStageMask   = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .dstAccessMask  = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    };

    const VkAttachmentDescription attachments[] =
    {
        color_attachment,
        depth_attachment
    };

    const VkRenderPassCreateInfo render_pass_create_info =
    {
        .sType              = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount    = (uint32_t)(sizeof(attachments) / sizeof(attachments[0])),
        .pAttachments       = attachments,
        .subpassCount       = 1,
        .pSubpasses         = &subpass_description,
        .dependencyCount    = 1,
        .pDependencies      = &subpass_dependancy,
    };

    lna_vulkan_check(
        vkCreateRenderPass(
            renderer->device,
            &render_pass_create_info,
            NULL,
            &renderer->render_pass
            )
        );
}

static void lna_vulkan_renderer_create_command_pool(
    lna_renderer_t* renderer
    )
{
    lna_assert(renderer)
    lna_assert(renderer->device)

    lna_vulkan_queue_family_indices_t indices = lna_vulkan_find_queue_families(
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_FRAME],
        renderer->physical_device,
        renderer->surface
        );

    const VkCommandPoolCreateInfo command_pool_create_info =
    {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex   = indices.graphics_family,
        .flags              = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    };

    lna_vulkan_check(
        vkCreateCommandPool(
            renderer->device,
            &command_pool_create_info,
            NULL,
            &renderer->command_pool
            )
        );
}

static void lna_vulkan_renderer_create_depth_resources(
    lna_renderer_t* renderer
    )
{
    lna_assert(renderer)

    VkFormat depth_format = lna_vulkan_find_depth_format(renderer->physical_device);
    lna_assert(depth_format != VK_FORMAT_UNDEFINED)

    lna_vulkan_create_image(
        renderer->device,
        renderer->physical_device,
        renderer->swap_chain_extent.width,
        renderer->swap_chain_extent.height,
        depth_format,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &renderer->depth_image,
        &renderer->depth_image_memory
        );

    renderer->depth_image_view = lna_vulkan_create_image_view(
        renderer->device,
        renderer->depth_image,
        depth_format,
        VK_IMAGE_ASPECT_DEPTH_BIT
        );
}

static void lna_vulkan_renderer_create_framebuffers(
    lna_renderer_t* renderer
    )
{
    lna_assert(renderer)
    lna_assert(renderer->device)
    lna_assert(renderer->swap_chain_framebuffers.count == 0)
    lna_assert(renderer->swap_chain_framebuffers.elements == NULL)
    lna_assert(renderer->swap_chain_image_views.count > 0)

    renderer->swap_chain_framebuffers.count     = renderer->swap_chain_image_views.count;
    renderer->swap_chain_framebuffers.elements  = lna_memory_pool_reserve(
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_SWAP_CHAIN],
        sizeof(VkFramebuffer) * renderer->swap_chain_image_views.count
        );

    for (size_t i = 0; i < renderer->swap_chain_framebuffers.count; ++i)
    {
        const VkImageView attachments[] =
        {
            renderer->swap_chain_image_views.elements[i],
            renderer->depth_image_view
        };

        const VkFramebufferCreateInfo framebuffer_create_info =
        {
            .sType              = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass         = renderer->render_pass,
            .attachmentCount    = (uint32_t)(sizeof(attachments) / sizeof(attachments[0])),
            .pAttachments       = attachments,
            .width              = renderer->swap_chain_extent.width,
            .height             = renderer->swap_chain_extent.height,
            .layers             = 1,
        };

        lna_vulkan_check(
            vkCreateFramebuffer(
                renderer->device,
                &framebuffer_create_info,
                NULL,
                &renderer->swap_chain_framebuffers.elements[i]
                )
            );
    }
}

static void lna_vulkan_renderer_create_command_buffers(
    lna_renderer_t* renderer
    )
{
    lna_assert(renderer)
    lna_assert(renderer->device)
    lna_assert(renderer->command_buffers.count == 0)
    lna_assert(renderer->command_buffers.elements == NULL)
    lna_assert(renderer->swap_chain_images.count > 0)
    
    renderer->command_buffers.count     = renderer->swap_chain_images.count;
    renderer->command_buffers.elements  = lna_memory_pool_reserve(
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_SWAP_CHAIN],
        sizeof(VkCommandBuffer) * renderer->swap_chain_images.count
        );

    const VkCommandBufferAllocateInfo command_buffer_allocate_info =
    {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool        = renderer->command_pool,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = renderer->command_buffers.count,
    };

    lna_vulkan_check(
        vkAllocateCommandBuffers(
            renderer->device,
            &command_buffer_allocate_info,
            renderer->command_buffers.elements
            )
        );
}

static void lna_vulkan_renderer_create_sync_objects(
    lna_renderer_t* renderer
    )
{
    lna_assert(renderer)
    lna_assert(renderer->device)

    const VkSemaphoreCreateInfo semaphore_create_info =
    {
        .sType  = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    
    const VkFenceCreateInfo fence_create_info =
    {
        .sType  = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags  = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    for (uint32_t i = 0; i < LNA_VULKAN_MAX_FRAMES_IN_FLIGHT; ++i)
    {
        lna_vulkan_check(
            vkCreateSemaphore(
                renderer->device,
                &semaphore_create_info,
                NULL,
                &renderer->image_available_semaphores[i]
                )
            );
        lna_vulkan_check(
            vkCreateSemaphore(
                renderer->device,
                &semaphore_create_info,
                NULL,
                &renderer->render_finished_semaphores[i]
                )
            );
        lna_vulkan_check(
            vkCreateFence(
                renderer->device,
                &fence_create_info,
                NULL,
                &renderer->in_flight_fences[i]
                )
            );
    }
}

static void lna_vulkan_renderer_cleanup_swap_chain(
    lna_renderer_t* renderer
    )
{
    lna_assert(renderer)
    lna_assert(renderer->device)

    vkDestroyImageView(
        renderer->device,
        renderer->depth_image_view,
        NULL
        );
    vkDestroyImage(
        renderer->device,
        renderer->depth_image,
        NULL
        );
    vkFreeMemory(
        renderer->device,
        renderer->depth_image_memory,
        NULL
        );

    for (size_t i = 0; i < renderer->swap_chain_framebuffers.count; ++i)
    {
        vkDestroyFramebuffer(
            renderer->device,
            renderer->swap_chain_framebuffers.elements[i],
            NULL
            );
    }

    vkFreeCommandBuffers(
        renderer->device,
        renderer->command_pool,
        renderer->command_buffers.count,
        renderer->command_buffers.elements
        );

    for (uint32_t i = 0; i < renderer->listeners.cur_element_count; ++i)
    {
        lna_renderer_listener_t* listener = &renderer->listeners.elements[i];
        listener->on_cleanup(listener->handle);
    }

    vkDestroyRenderPass(
        renderer->device,
        renderer->render_pass,
        NULL
        );

    for (size_t i = 0; i < renderer->swap_chain_image_views.count; ++i)
    {
        vkDestroyImageView(
            renderer->device,
            renderer->swap_chain_image_views.elements[i],
            NULL
            );
    }

    vkDestroySwapchainKHR(
        renderer->device,
        renderer->swap_chain,
        NULL
        );

    lna_memory_pool_empty(
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_SWAP_CHAIN]
        );

    renderer->swap_chain_images.count           = 0;
    renderer->swap_chain_images.elements        = NULL;
    renderer->swap_chain_image_views.count      = 0;
    renderer->swap_chain_image_views.elements   = NULL;
    renderer->swap_chain_framebuffers.count     = 0;
    renderer->swap_chain_framebuffers.elements  = NULL;
    renderer->command_buffers.count             = 0;
    renderer->command_buffers.elements          = NULL;
    renderer->images_in_flight_fences.count     = 0;
    renderer->images_in_flight_fences.elements  = NULL;
}

static void lna_vulkan_renderer_recreate_swap_chain(
    lna_renderer_t* renderer,
    uint32_t framebuffer_width,
    uint32_t framebuffer_height
    )
{
    lna_assert(renderer)
    lna_assert(renderer->device)

    lna_vulkan_check(
        vkDeviceWaitIdle(
            renderer->device
            )
        );

    lna_vulkan_renderer_cleanup_swap_chain(renderer);
    lna_vulkan_renderer_create_swap_chain(renderer, framebuffer_width, framebuffer_height);
    lna_vulkan_renderer_create_image_views(renderer);
    lna_vulkan_renderer_create_render_pass(renderer);
    lna_vulkan_renderer_create_depth_resources(renderer);
    lna_vulkan_renderer_create_framebuffers(renderer);
    lna_vulkan_renderer_create_command_buffers(renderer);

    for (uint32_t i = 0; i < renderer->listeners.cur_element_count; ++i)
    {
        lna_renderer_listener_t* listener = &renderer->listeners.elements[i];
        listener->on_recreate(listener->handle);
    }
}

//! ============================================================================
//!                         RENDERER PUBLIC FUNCTIONS
//! ============================================================================

bool lna_renderer_init(lna_renderer_t* renderer, const lna_renderer_config_t* config)
{
    lna_assert(renderer)
    lna_assert(renderer->instance == NULL)
    lna_assert(renderer->debug_messenger == NULL)
    lna_assert(renderer->physical_device == NULL)
    lna_assert(renderer->device == NULL)
    lna_assert(renderer->graphics_family == 0)
    lna_assert(renderer->graphics_queue == NULL)
    lna_assert(renderer->surface == NULL)
    lna_assert(renderer->present_queue == NULL)
    lna_assert(renderer->swap_chain == NULL)
    lna_assert(renderer->render_pass == NULL)
    lna_assert(renderer->command_pool == NULL)
    lna_assert(renderer->curr_frame == 0)
    lna_assert(renderer->images_in_flight_fences.count == 0)
    lna_assert(renderer->images_in_flight_fences.elements == NULL)
    lna_assert(renderer->swap_chain_images.count == 0)
    lna_assert(renderer->swap_chain_images.elements == NULL)
    lna_assert(renderer->swap_chain_image_views.count == 0)
    lna_assert(renderer->swap_chain_image_views.elements == NULL)
    lna_assert(renderer->swap_chain_framebuffers.count == 0)
    lna_assert(renderer->swap_chain_framebuffers.elements == NULL)
    lna_assert(renderer->command_buffers.count == 0)
    lna_assert(renderer->command_buffers.elements == NULL)
    lna_assert(renderer->listeners.cur_element_count == 0)
    lna_assert(renderer->listeners.max_element_count == 0)
    lna_assert(renderer->listeners.elements == NULL)

    lna_assert(config)
    lna_assert(config->window)

    lna_memory_pool_init_with_heap(
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_FRAME],
        config->allocator,
        config->frame_mem_pool_size == 0 ? LNA_VULKAN_RENDERER_DEFAULT_MEMORY_POOL_SIZES[LNA_VULKAN_RENDERER_MEMORY_POOL_FRAME] : config->frame_mem_pool_size
        );
    lna_memory_pool_init_with_heap(
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_PERSISTENT],
        config->allocator,
        config->persistent_mem_pool_size == 0 ? LNA_VULKAN_RENDERER_DEFAULT_MEMORY_POOL_SIZES[LNA_VULKAN_RENDERER_MEMORY_POOL_FRAME] : config->persistent_mem_pool_size
        );
    lna_memory_pool_init_with_heap(
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_SWAP_CHAIN],
        config->allocator,
        config->swap_chain_mem_pool_size == 0 ? LNA_VULKAN_RENDERER_DEFAULT_MEMORY_POOL_SIZES[LNA_VULKAN_RENDERER_MEMORY_POOL_FRAME] : config->swap_chain_mem_pool_size
        );

    if (config->max_listener_count > 0)
    {
        renderer->listeners.max_element_count   = config->max_listener_count;
        renderer->listeners.elements            = lna_memory_pool_reserve(
            &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_PERSISTENT],
            sizeof(lna_renderer_listener_t) * renderer->listeners.max_element_count
            );
    }

    renderer->curr_frame = 0;
    renderer->graphics_family = (uint32_t)-1;
    if (!lna_vulkan_renderer_create_instance(renderer, config->window, config->enable_api_diagnostic))
    {
        return false;
    }
    if (config->enable_api_diagnostic)
    {
        lna_vulkan_renderer_setup_debug_messenger(renderer);
    }
    lna_vulkan_renderer_create_surface(renderer, config->window);
    lna_vulkan_renderer_pick_physical_device(renderer);
    lna_vulkan_renderer_create_logical_device(renderer, config->enable_api_diagnostic);
    lna_vulkan_renderer_create_swap_chain(renderer, lna_window_width(config->window), lna_window_height(config->window));
    lna_vulkan_renderer_create_image_views(renderer);
    lna_vulkan_renderer_create_render_pass(renderer);
    lna_vulkan_renderer_create_command_pool(renderer);
    lna_vulkan_renderer_create_depth_resources(renderer);
    lna_vulkan_renderer_create_framebuffers(renderer);
    lna_vulkan_renderer_create_command_buffers(renderer);
    lna_vulkan_renderer_create_sync_objects(renderer);

    return true;
}

void lna_renderer_begin_draw_frame(lna_renderer_t* renderer, uint32_t window_width, uint32_t window_height)
{
    lna_assert(renderer)
    lna_assert(renderer->device)

    lna_vulkan_check(
        vkWaitForFences(
            renderer->device,
            1,
            &renderer->in_flight_fences[renderer->curr_frame],
            VK_TRUE,
            UINT64_MAX
            )
        );

    VkResult result = vkAcquireNextImageKHR(
        renderer->device,
        renderer->swap_chain,
        UINT64_MAX,
        renderer->image_available_semaphores[renderer->curr_frame],
        VK_NULL_HANDLE,
        &renderer->image_index
        );
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        lna_vulkan_renderer_recreate_swap_chain(
            renderer,
            window_width,
            window_height
            );
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        lna_assert(0)
    }

    lna_assert(renderer->images_in_flight_fences.count > renderer->image_index)
    if (renderer->images_in_flight_fences.elements[renderer->image_index] != VK_NULL_HANDLE)
    {
        lna_vulkan_check(
            vkWaitForFences(
                renderer->device,
                1,
                &renderer->images_in_flight_fences.elements[renderer->image_index],
                VK_TRUE,
                UINT64_MAX
                )
            );
    }
    renderer->images_in_flight_fences.elements[renderer->image_index] = renderer->in_flight_fences[renderer->curr_frame];

    lna_vulkan_check(
        vkResetFences(
            renderer->device,
            1,
            &renderer->in_flight_fences[renderer->curr_frame]
            )
        );

    const VkClearValue clear_values[] =
    {
        {
            .color = { 0.05f, 0.05f, 0.05f, 1.0f },
        },
        {
            .depthStencil = { 1.0f, 0 }
        },
    };
    const uint32_t clear_value_count = (uint32_t)(sizeof(clear_values) / sizeof(clear_values[0]));

    const VkCommandBufferBeginInfo command_buffer_begin_info =
    {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags              = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
        .pInheritanceInfo   = NULL,
    };

    const VkViewport viewport =
    {
        .width      = (float)renderer->swap_chain_extent.width,
	    .height     = (float)renderer->swap_chain_extent.height,
	    .minDepth   = 0.0f,
	    .maxDepth   = 1.0f,
    };

    const VkRect2D scissor_rect =
    {
        .offset.x       = 0,
        .offset.y       = 0,
        .extent.width   = renderer->swap_chain_extent.width,
        .extent.height  = renderer->swap_chain_extent.height,
    };

    lna_assert(renderer->command_buffers.count > renderer->image_index)
    VkCommandBuffer command_buffer = renderer->command_buffers.elements[renderer->image_index];

    lna_vulkan_check(
        vkResetCommandBuffer(
            command_buffer,
            VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT
            )
        );

    lna_vulkan_check(
        vkBeginCommandBuffer(
            command_buffer,
            &command_buffer_begin_info
            )
        );

    VkRenderPassBeginInfo render_pass_begin_info =
    {
        .sType              = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass         = renderer->render_pass,
        .framebuffer        = renderer->swap_chain_framebuffers.elements[renderer->image_index],
        .renderArea.offset  = { 0, 0 },
        .renderArea.extent  = renderer->swap_chain_extent,
        .clearValueCount    = clear_value_count,
        .pClearValues       = clear_values,
    };

    vkCmdBeginRenderPass(
        command_buffer,
        &render_pass_begin_info,
        VK_SUBPASS_CONTENTS_INLINE
        );
    vkCmdSetViewport(
        command_buffer,
        0,
        1,
        &viewport
        );
    vkCmdSetScissor(
        command_buffer,
        0,
        1,
        &scissor_rect
        );
}

void lna_renderer_end_draw_frame(lna_renderer_t* renderer, bool window_resized, uint32_t window_width, uint32_t window_height)
{
    const VkSemaphore wait_semaphores[] =
    {
        renderer->image_available_semaphores[renderer->curr_frame],
    };

    const VkPipelineStageFlags wait_stages[] =
    {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    };

    const VkSemaphore signal_semaphores[] =
    {
        renderer->render_finished_semaphores[renderer->curr_frame],
    };

    lna_assert(renderer->command_buffers.count > renderer->image_index)
    VkCommandBuffer command_buffer = renderer->command_buffers.elements[renderer->image_index];

    vkCmdEndRenderPass(
        command_buffer
        );
    lna_vulkan_check(
        vkEndCommandBuffer(
            command_buffer
            )
        );

    const VkSubmitInfo submit_info =
    {
        .sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount     = 1,
        .pWaitSemaphores        = wait_semaphores,
        .pWaitDstStageMask      = wait_stages,
        .commandBufferCount     = 1,
        .pCommandBuffers        = &command_buffer,
        .signalSemaphoreCount   = 1,
        .pSignalSemaphores      = signal_semaphores,
    };

    lna_vulkan_check(
        vkQueueSubmit(
            renderer->graphics_queue,
            1,
            &submit_info,
            renderer->in_flight_fences[renderer->curr_frame]
            )
        );

    const VkSwapchainKHR swap_chains[] =
    {
        renderer->swap_chain,
    };

    const VkPresentInfoKHR present_info =
    {
        .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = signal_semaphores,
        .swapchainCount     = 1,
        .pSwapchains        = swap_chains,
        .pImageIndices      = &renderer->image_index,
        .pResults           = NULL,
    };

    VkResult result = vkQueuePresentKHR(
        renderer->present_queue,
        &present_info
        );
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window_resized)
    {
        lna_vulkan_renderer_recreate_swap_chain(
            renderer,
            window_width,
            window_height
            );
        return;
    }
    else if (result != VK_SUCCESS)
    {
        lna_assert(0)
    }

    renderer->curr_frame = (renderer->curr_frame + 1) % LNA_VULKAN_MAX_FRAMES_IN_FLIGHT;

    lna_memory_pool_empty(&renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_FRAME]);
}

void lna_renderer_wait_idle(lna_renderer_t* renderer)
{
    lna_assert(renderer)
    lna_assert(renderer->device)

    lna_vulkan_check(
        vkDeviceWaitIdle(
            renderer->device
            )
        );
}

void lna_renderer_release(lna_renderer_t* renderer)
{
    lna_assert(renderer)
    lna_assert(renderer->device)

    lna_vulkan_renderer_cleanup_swap_chain(renderer);

    for (size_t i = 0; i < LNA_VULKAN_MAX_FRAMES_IN_FLIGHT; ++i)
    {
        vkDestroySemaphore(
            renderer->device,
            renderer->render_finished_semaphores[i],
            NULL
            );
        vkDestroySemaphore(
            renderer->device,
            renderer->image_available_semaphores[i],
            NULL
            );
        vkDestroyFence(
            renderer->device,
            renderer->in_flight_fences[i],
            NULL
            );
    }

    vkDestroyCommandPool(
        renderer->device,
        renderer->command_pool,
        NULL 
        );
    vkDestroyDevice(
        renderer->device,
        NULL
        );
    if (renderer->debug_messenger)
    {
        lna_vulkan_destroy_debug_utilis_messenger_EXT(
            renderer->instance,
            renderer->debug_messenger,
            NULL
            );
    }
    vkDestroySurfaceKHR(
        renderer->instance,
        renderer->surface,
        NULL
        );
    vkDestroyInstance(
        renderer->instance,
        NULL
        );
}

void lna_renderer_register_listener(
    lna_renderer_t* renderer,
    lna_vulkan_on_swap_chain_cleanup_t on_cleanup,
    lna_vulkan_on_swap_chain_recreate_t on_recreate,
    void* handle
    )
{
    lna_assert(renderer)
    lna_assert(renderer->listeners.elements)
    lna_assert(renderer->listeners.cur_element_count < renderer->listeners.max_element_count)
    lna_assert(on_cleanup)
    lna_assert(on_recreate)
    lna_assert(handle)

    lna_renderer_listener_t* listener = &renderer->listeners.elements[renderer->listeners.cur_element_count++];
    listener->on_cleanup    = on_cleanup;
    listener->on_recreate   = on_recreate;
    listener->handle        = handle;
}
