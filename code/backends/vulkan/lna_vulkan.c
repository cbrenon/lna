#include "backends/vulkan/lna_vulkan.h"

const char* lna_vulkan_error_string(VkResult error_code)
{
    //switch (error_code)
	//{
#define STR(r) if (error_code == VK_ ##r) return #r;
		STR(NOT_READY)
		STR(TIMEOUT)
		STR(EVENT_SET)
		STR(EVENT_RESET)
		STR(INCOMPLETE)
		STR(ERROR_OUT_OF_HOST_MEMORY)
		STR(ERROR_OUT_OF_DEVICE_MEMORY)
		STR(ERROR_INITIALIZATION_FAILED)
		STR(ERROR_DEVICE_LOST)
		STR(ERROR_MEMORY_MAP_FAILED)
		STR(ERROR_LAYER_NOT_PRESENT)
		STR(ERROR_EXTENSION_NOT_PRESENT)
		STR(ERROR_FEATURE_NOT_PRESENT)
		STR(ERROR_INCOMPATIBLE_DRIVER)
		STR(ERROR_TOO_MANY_OBJECTS)
		STR(ERROR_FORMAT_NOT_SUPPORTED)
		STR(ERROR_SURFACE_LOST_KHR)
		STR(ERROR_NATIVE_WINDOW_IN_USE_KHR)
		STR(SUBOPTIMAL_KHR)
		STR(ERROR_OUT_OF_DATE_KHR)
		STR(ERROR_INCOMPATIBLE_DISPLAY_KHR)
		STR(ERROR_VALIDATION_FAILED_EXT)
		STR(ERROR_INVALID_SHADER_NV)
#undef STR
		//default:
		    return "UNKNOWN_ERROR";
	//}
}

uint32_t lna_vulkan_find_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties)
{
    lna_assert(physical_device)

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
    lna_assert(0)
}

void lna_vulkan_create_buffer(VkDevice device, VkPhysicalDevice physical_device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* buffer_memory)
{
    lna_assert(device)

    const VkBufferCreateInfo buffer_create_info =
    {
        .sType          = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size           = size,
        .usage          = usage,
        .sharingMode    = VK_SHARING_MODE_EXCLUSIVE,
    };

    VULKAN_CHECK_RESULT(
        vkCreateBuffer(
            device,
            &buffer_create_info,
            NULL,
            buffer
            )
        )

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(
        device,
        *buffer,
        &memory_requirements
        );

    const VkMemoryAllocateInfo memory_allocate_info =
    {
        .sType              = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize     = memory_requirements.size,
        .memoryTypeIndex    = lna_vulkan_find_memory_type(
            physical_device,
            memory_requirements.memoryTypeBits,
            properties
            ),
    };
        
    VULKAN_CHECK_RESULT(
        vkAllocateMemory(
            device,
            &memory_allocate_info,
            NULL,
            buffer_memory
            )
        )
        
    VULKAN_CHECK_RESULT(
        vkBindBufferMemory(
            device,
            *buffer,
            *buffer_memory,
            0
            )
        )
}

void lna_vulkan_create_image(VkDevice device, VkPhysicalDevice physical_device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VkDeviceMemory* image_memory)
{
    lna_assert(device)
    lna_assert(physical_device)

    const VkImageCreateInfo image_create_info =
    {
        .sType          = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType      = VK_IMAGE_TYPE_2D,
        .extent.width   = width,
        .extent.height  = height,
        .extent.depth   = 1,
        .mipLevels      = 1,
        .arrayLayers    = 1,
        .format         = format,
        .tiling         = tiling,
        .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage          = usage,
        .sharingMode    = VK_SHARING_MODE_EXCLUSIVE,
        .samples        = VK_SAMPLE_COUNT_1_BIT,
        .flags          = 0,
    };

    VULKAN_CHECK_RESULT(
        vkCreateImage(
            device,
            &image_create_info,
            NULL,
            image
            )
        )

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(
        device,
        *image,
        &memory_requirements
        );

    const VkMemoryAllocateInfo allocate_info =
    {
        .sType              = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize     = memory_requirements.size,
        .memoryTypeIndex    = lna_vulkan_find_memory_type(
            physical_device,
            memory_requirements.memoryTypeBits,
            properties
            ),
    };

    VULKAN_CHECK_RESULT(
        vkAllocateMemory(
            device,
            &allocate_info,
            NULL,
            image_memory
            )
        )

    VULKAN_CHECK_RESULT(
        vkBindImageMemory(
            device,
            *image,
            *image_memory,
            0
            )
        )
}

VkImageView lna_vulkan_create_image_view(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags)
{
    lna_assert(device)
    lna_assert(image)
    
    const VkImageViewCreateInfo view_create_info =
    {
        .sType                              = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image                              = image,
        .viewType                           = VK_IMAGE_VIEW_TYPE_2D,
        .format                             = format,
        .subresourceRange.aspectMask        = aspect_flags,
        .subresourceRange.baseMipLevel      = 0,
        .subresourceRange.levelCount        = 1,
        .subresourceRange.baseArrayLayer    = 0,
        .subresourceRange.layerCount        = 1,
    };

    VkImageView image_view;
    VULKAN_CHECK_RESULT(
        vkCreateImageView(
            device,
            &view_create_info,
            NULL,
            &image_view
            )
        )

    return image_view;
}

VkCommandBuffer lna_vulkan_begin_single_time_commands(VkDevice device, VkCommandPool command_pool)
{
    lna_assert(device)
    lna_assert(command_pool)

    const VkCommandBufferAllocateInfo allocate_info =
    {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool        = command_pool,
        .commandBufferCount = 1,
    };
    VkCommandBuffer command_buffer;
    VULKAN_CHECK_RESULT(
        vkAllocateCommandBuffers(
            device,
            &allocate_info,
            &command_buffer
            )
        )
    const VkCommandBufferBeginInfo begin_info =
    {
        .sType  = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags  = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    VULKAN_CHECK_RESULT(
        vkBeginCommandBuffer(
            command_buffer,
            &begin_info
            )
        )

    return command_buffer;
}

void lna_vulkan_end_single_time_commands(VkDevice device, VkCommandPool command_pool, VkCommandBuffer command_buffer, VkQueue graphics_queue)
{
    lna_assert(device)
    lna_assert(command_pool)
    lna_assert(command_buffer)
    lna_assert(graphics_queue)

    VULKAN_CHECK_RESULT(
        vkEndCommandBuffer(
            command_buffer
            )
        )

    const VkSubmitInfo submit_info =
    {
        .sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers    = &command_buffer,
    };

    VULKAN_CHECK_RESULT(
        vkQueueSubmit(
            graphics_queue,
            1,
            &submit_info,
            VK_NULL_HANDLE
            )
        )
    VULKAN_CHECK_RESULT(
        vkQueueWaitIdle(
            graphics_queue
            )
        )
    vkFreeCommandBuffers(
        device,
        command_pool,
        1,
        &command_buffer
        );
}

void lna_vulkan_transition_image_layout(VkDevice device, VkCommandPool command_pool, VkQueue graphics_queue, VkImage image, VkImageLayout old_layout, VkImageLayout new_layout)
{
    VkCommandBuffer command_buffer = lna_vulkan_begin_single_time_commands(
        device,
        command_pool
        );

    VkImageMemoryBarrier barrier =
    {
        .sType                              = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout                          = old_layout,
        .newLayout                          = new_layout,
        .srcQueueFamilyIndex                = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex                = VK_QUEUE_FAMILY_IGNORED,
        .image                              = image,
        .subresourceRange.aspectMask        = VK_IMAGE_ASPECT_COLOR_BIT,
        .subresourceRange.baseMipLevel      = 0,
        .subresourceRange.levelCount        = 1,
        .subresourceRange.baseArrayLayer    = 0,
        .subresourceRange.layerCount        = 1,
        .srcAccessMask                      = 0,
        .dstAccessMask                      = 0,
    };
        
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
        lna_assert(0)
    }

    vkCmdPipelineBarrier(
        command_buffer,
        src_stage,
        dst_stage,
        0,
        0,
        NULL,
        0,
        NULL,
        1,
        &barrier
        );

    lna_vulkan_end_single_time_commands(
        device,
        command_pool,
        command_buffer,
        graphics_queue
        );
}

void lna_vulkan_copy_buffer_to_image(VkDevice device, VkCommandPool command_pool, VkBuffer buffer, VkQueue graphics_queue, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer command_buffer = lna_vulkan_begin_single_time_commands(
        device,
        command_pool
        );

    const VkBufferImageCopy region =
    {
        .bufferOffset                       = 0,
        .bufferRowLength                    = 0,
        .bufferImageHeight                  = 0,
        .imageSubresource.aspectMask        = VK_IMAGE_ASPECT_COLOR_BIT,
        .imageSubresource.mipLevel          = 0,
        .imageSubresource.baseArrayLayer    = 0,
        .imageSubresource.layerCount        = 1,
        .imageOffset                        = (VkOffset3D){ 0, 0, 0 },
        .imageExtent                        = (VkExtent3D){ width, height, 1 },
    };

    vkCmdCopyBufferToImage(
        command_buffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
        );
    
    lna_vulkan_end_single_time_commands(
        device,
        command_pool,
        command_buffer,
        graphics_queue
        );
}

void lna_vulkan_copy_buffer(VkDevice device, VkCommandPool command_pool, VkQueue graphics_queue, VkBuffer src, VkBuffer dst, VkDeviceSize size)
{
    lna_assert(device)
    lna_assert(command_pool)
    lna_assert(graphics_queue)
    
    VkCommandBuffer command_buffer = lna_vulkan_begin_single_time_commands(
        device,
        command_pool
        );
    lna_assert(command_buffer)

    const VkBufferCopy copy_region =
    {
        .size = size,
    };

    vkCmdCopyBuffer(
        command_buffer,
        src,
        dst,
        1,
        &copy_region
        );
    lna_vulkan_end_single_time_commands(
        device,
        command_pool,
        command_buffer,
        graphics_queue
        );
}

VkShaderModule lna_vulkan_create_shader_module(VkDevice device, uint32_t* code, size_t code_size)
{
    lna_assert(device)
    lna_assert(code)
    lna_assert(code_size > 0)

    const VkShaderModuleCreateInfo shader_module_create_info =
    {
        .sType      = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize   = code_size,
        .pCode      = code,
    };

    VkShaderModule shader_module = NULL;
    VULKAN_CHECK_RESULT(
        vkCreateShaderModule(
            device,
            &shader_module_create_info,
            NULL,
            &shader_module
            )
        )

    return shader_module;
}

VkFormat lna_vulkan_find_supported_format(VkPhysicalDevice physical_device, VkFormat* candidate_formats, uint32_t candidate_format_count, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    lna_assert(physical_device)

    for (uint32_t i = 0; i < candidate_format_count; ++i)
    {
        VkFormatProperties format_properties;
        vkGetPhysicalDeviceFormatProperties(
            physical_device,
            candidate_formats[i],
            &format_properties
            );
        if (
            tiling == VK_IMAGE_TILING_LINEAR
            && (format_properties.linearTilingFeatures & features) == features
            )
        {
            return candidate_formats[i];
        }
        else if (
            tiling == VK_IMAGE_TILING_OPTIMAL
            && (format_properties.optimalTilingFeatures & features) == features
        )
        {
            return candidate_formats[i];
        }
    }
    lna_log_error("cannot find supported format");
    lna_assert(0)
}

bool lna_vulkan_has_stencil_component(VkFormat format)
{
    return
            format == VK_FORMAT_D32_SFLOAT_S8_UINT
        ||  format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkFormat lna_vulkan_find_depth_format(VkPhysicalDevice physical_device)
{
    VkFormat formats[] =
    {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT,
    };
    return lna_vulkan_find_supported_format(
        physical_device,
        formats,
        (uint32_t)(sizeof(formats) / sizeof(formats[0])),
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
}
