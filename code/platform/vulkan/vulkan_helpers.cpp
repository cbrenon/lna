#include "core/assert.hpp"
#include "platform/vulkan/vulkan_helpers.hpp"
#include "graphics/vertex.hpp"

const char* lna::vulkan_helpers::error_string(VkResult error_code)
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

uint32_t lna::vulkan_helpers::find_memory_type(
    VkPhysicalDevice physical_device,
    uint32_t type_filter,
    VkMemoryPropertyFlags properties
    )
{
    LNA_ASSERT(physical_device);

    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(
        physical_device,
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

void lna::vulkan_helpers::create_buffer(
    VkDevice device,
    VkPhysicalDevice physical_device,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer& buffer,
    VkDeviceMemory& buffer_memory
    )
{
    LNA_ASSERT(device);

    VkBufferCreateInfo buffer_create_info{};
    buffer_create_info.sType        = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.size         = size;
    buffer_create_info.usage        = usage;
    buffer_create_info.sharingMode  = VK_SHARING_MODE_EXCLUSIVE;

    VULKAN_CHECK_RESULT(
        vkCreateBuffer(
            device,
            &buffer_create_info,
            nullptr,
            &buffer
            )
        )

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(
        device,
        buffer,
        &memory_requirements
        );

    VkMemoryAllocateInfo memory_allocate_info{};
    memory_allocate_info.sType              = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize     = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex    = lna::vulkan_helpers::find_memory_type(
        physical_device,
        memory_requirements.memoryTypeBits,
        properties
        );
        
    VULKAN_CHECK_RESULT(
        vkAllocateMemory(
            device,
            &memory_allocate_info,
            nullptr,
            &buffer_memory
            )
        )
        
    VULKAN_CHECK_RESULT(
        vkBindBufferMemory(
            device,
            buffer,
            buffer_memory,
            0
            )
        )
}
