#ifndef _LNA_PLATFORM_VULKAN_VULKAN_HELPERS_HPP_
#define _LNA_PLATFORM_VULKAN_VULKAN_HELPERS_HPP_

#include <vulkan.h>

#define VULKAN_CHECK_RESULT(func)                                                       \
    {                                                                                   \
        VkResult _result = (func);                                                      \
        if (_result != VK_SUCCESS)                                                      \
        {                                                                               \
            lna::log::error("vulkan %s", lna::vulkan_helpers::error_string(_result));   \
            LNA_ASSERT(0);                                                              \
        }                                                                               \
    }

namespace lna
{
    namespace vulkan_helpers
    {
        const char* error_string(
            VkResult error_code
            );

        uint32_t find_memory_type(
            VkPhysicalDevice physical_device,
            uint32_t type_filter,
            VkMemoryPropertyFlags properties
            );

        void create_buffer(
            VkDevice device,
            VkPhysicalDevice physical_device,
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer& buffer,
            VkDeviceMemory& buffer_memory
            );
    }
}

#endif // _LNA_PLATFORM_VULKAN_VULKAN_HELPERS_HPP_
