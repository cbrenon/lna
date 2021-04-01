#include "backends/vulkan/vulkan_helpers.hpp"
#include "core/assert.hpp"
#include "core/memory_pool.hpp"
#include "core/file.hpp"

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

void lna::vulkan_helpers::create_image(
    VkDevice device,
    VkPhysicalDevice physical_device,
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
    LNA_ASSERT(device);
    LNA_ASSERT(physical_device);

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
            device,
            &image_create_info,
            nullptr,
            &image
            )
        )

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(
        device,
        image,
        &memory_requirements
        );

    VkMemoryAllocateInfo allocate_info{};
    allocate_info.sType             = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize    = memory_requirements.size;
    allocate_info.memoryTypeIndex   = lna::vulkan_helpers::find_memory_type(
        physical_device,
        memory_requirements.memoryTypeBits,
        properties
        );

    VULKAN_CHECK_RESULT(
        vkAllocateMemory(
            device,
            &allocate_info,
            nullptr,
            &image_memory
            )
        )

    VULKAN_CHECK_RESULT(
        vkBindImageMemory(
            device,
            image,
            image_memory,
            0
            )
        )
}

VkImageView lna::vulkan_helpers::create_image_view(
    VkDevice device,
    VkImage image,
    VkFormat format
    )
{
    LNA_ASSERT(device);
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
            device,
            &view_create_info,
            nullptr,
            &image_view
            )
        )

    return image_view;
}

VkSampler create_texture_sampler(
    VkDevice device,
    VkPhysicalDevice physical_device
    )
{
    LNA_ASSERT(device);
    LNA_ASSERT(physical_device);

    VkPhysicalDeviceProperties gpu_properties{};
    vkGetPhysicalDeviceProperties(
        physical_device,
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

    VkSampler sampler = nullptr;

    VULKAN_CHECK_RESULT(
        vkCreateSampler(
            device,
            &sampler_create_info,
            nullptr,
            &sampler
            )
        )

    return sampler;
}

VkCommandBuffer lna::vulkan_helpers::begin_single_time_commands(
    VkDevice device,
    VkCommandPool command_pool
    )
{
    LNA_ASSERT(device);
    LNA_ASSERT(command_pool);

    VkCommandBufferAllocateInfo allocate_info{};
    allocate_info.sType                 = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.level                 = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandPool           = command_pool;
    allocate_info.commandBufferCount    = 1;

    VkCommandBuffer command_buffer = nullptr;
    VULKAN_CHECK_RESULT(
        vkAllocateCommandBuffers(
            device,
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

void lna::vulkan_helpers::end_single_time_commands(
    VkDevice device,
    VkCommandPool command_pool,
    VkCommandBuffer command_buffer,
    VkQueue graphics_queue
    )
{
    LNA_ASSERT(device);
    LNA_ASSERT(command_pool);
    LNA_ASSERT(command_buffer);
    LNA_ASSERT(graphics_queue);

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

void lna::vulkan_helpers::transition_image_layout(
    VkDevice device,
    VkCommandPool command_pool,
    VkQueue graphics_queue,
    VkImage image,
    VkImageLayout old_layout,
    VkImageLayout new_layout
    )
{
    VkCommandBuffer command_buffer = lna::vulkan_helpers::begin_single_time_commands(
        device,
        command_pool
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

    lna::vulkan_helpers::end_single_time_commands(
        device,
        command_pool,
        command_buffer,
        graphics_queue
        );
}

void lna::vulkan_helpers::copy_buffer_to_image(
    VkDevice device,
    VkCommandPool command_pool,
    VkBuffer buffer,
    VkQueue graphics_queue,
    VkImage image,
    uint32_t width,
    uint32_t height
    )
{
    VkCommandBuffer command_buffer = lna::vulkan_helpers::begin_single_time_commands(
        device,
        command_pool
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
    
    lna::vulkan_helpers::end_single_time_commands(
        device,
        command_pool,
        command_buffer,
        graphics_queue
        );
}

void lna::vulkan_helpers::copy_buffer(
    VkDevice device,
    VkCommandPool command_pool,
    VkQueue graphics_queue,
    VkBuffer src,
    VkBuffer dst,
    VkDeviceSize size
    )
{
    LNA_ASSERT(device);
    LNA_ASSERT(command_pool);
    LNA_ASSERT(graphics_queue);
    
    VkCommandBuffer command_buffer = lna::vulkan_helpers::begin_single_time_commands(
        device,
        command_pool
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
    lna::vulkan_helpers::end_single_time_commands(
        device,
        command_pool,
        command_buffer,
        graphics_queue
        );
}

VkShaderModule lna::vulkan_helpers::load_shader(
    VkDevice device,
    const char* filename,
    lna::memory_pool& pool
    )
{
    LNA_ASSERT(device);
    LNA_ASSERT(filename);

    lna::file shader_file;
    shader_file.content = nullptr;
    shader_file.content_size = 0;
    lna::file_debug_load(
        shader_file,
        filename,
        true,
        pool
        );

    LNA_ASSERT(shader_file.content);
    LNA_ASSERT(shader_file.content_size > 0);
    
    VkShaderModuleCreateInfo shader_module_create_info{};
    shader_module_create_info.sType     = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_create_info.codeSize  = shader_file.content_size;
    shader_module_create_info.pCode     = reinterpret_cast<const uint32_t*>(shader_file.content);

    VkShaderModule shader_module = nullptr;
    VULKAN_CHECK_RESULT(
        vkCreateShaderModule(
            device,
            &shader_module_create_info,
            nullptr,
            &shader_module
            )
        )

    return shader_module;
}
