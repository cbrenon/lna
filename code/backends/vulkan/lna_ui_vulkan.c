#include <string.h>
#include "graphics/lna_ui.h"
#include "system/lna_window.h"
#include "backends/vulkan/lna_renderer_vulkan.h"
#include "backends/vulkan/lna_ui_vulkan.h"
#include "backends/vulkan/lna_vulkan.h"
#include "backends/vulkan/lna_texture_vulkan.h"
#include "core/lna_assert.h"
#include "core/lna_file.h"

static void lna_ui_buffer_init(
    lna_ui_buffer_t* buffer,
    const lna_ui_buffer_config_t* config,
    VkDescriptorPool descriptor_pool,
    VkDescriptorSetLayout descriptor_set_layout,
    VkDevice device,
    VkPhysicalDevice physical_device
    )
{
    lna_assert(buffer)
    lna_assert(buffer->vertices == NULL)
    lna_assert(buffer->indices == NULL)
    lna_assert(buffer->max_vertex_count == 0)
    lna_assert(buffer->max_index_count == 0)
    lna_assert(buffer->cur_vertex_count == 0)
    lna_assert(buffer->cur_index_count == 0)
    lna_assert(buffer->vertex_buffer == VK_NULL_HANDLE)
    lna_assert(buffer->vertex_buffer_memory == VK_NULL_HANDLE)
    lna_assert(buffer->index_buffer == VK_NULL_HANDLE)
    lna_assert(buffer->index_buffer_memory == VK_NULL_HANDLE)
    lna_assert(buffer->descriptor_set == VK_NULL_HANDLE)
    lna_assert(buffer->vertex_data_mapped == NULL)
    lna_assert(buffer->index_data_mapped == NULL)
    lna_assert(buffer->texture == NULL)
    lna_assert(config)
    lna_assert(config->memory_pool)
    lna_assert(config->max_vertex_count > 0)
    lna_assert(config->max_index_count > 0)
    lna_assert(descriptor_pool)
    lna_assert(descriptor_set_layout)
    lna_assert(device)

    buffer->max_vertex_count    = config->max_vertex_count;
    buffer->max_index_count     = config->max_index_count;
    buffer->vertices            = lna_memory_pool_reserve(config->memory_pool, sizeof(lna_ui_vertex_t) * buffer->max_vertex_count);
    buffer->indices             = lna_memory_pool_reserve(config->memory_pool, sizeof(uint32_t) * buffer->max_index_count);
    buffer->texture             = config->texture;

    const VkDescriptorSetAllocateInfo set_allocate_info =
    {
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool     = descriptor_pool,
        .pSetLayouts        = &descriptor_set_layout,
        .descriptorSetCount = 1,
    };
    lna_vulkan_check(
        vkAllocateDescriptorSets(
            device,
            &set_allocate_info,
            &buffer->descriptor_set
            )
        );

    const VkDescriptorImageInfo descriptor_image_info =
    {
        .sampler        = config->texture->image_sampler,
        .imageView      = config->texture->image_view,
        .imageLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
    const VkWriteDescriptorSet write_descriptor_sets[1] =
    {
        {
            .sType              = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet             = buffer->descriptor_set,
            .descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .dstBinding         = 0,
            .pImageInfo         = &descriptor_image_info,
            .descriptorCount    = 1,
        }
    };
    vkUpdateDescriptorSets(
        device,
        1,
        write_descriptor_sets,
        0,
        NULL
        );

    const VkDeviceSize vertex_buffer_size = buffer->max_vertex_count * sizeof(lna_ui_vertex_t);
    const VkBufferCreateInfo vertex_buffer_create_info =
    {
        .sType  = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .usage  = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .size   = vertex_buffer_size,
    };
    lna_vulkan_check(
        vkCreateBuffer(
            device,
            &vertex_buffer_create_info,
            NULL,
            &buffer->vertex_buffer
            )
        );

    VkMemoryRequirements vertex_buffer_memory_requirements;
    vkGetBufferMemoryRequirements(
        device,
        buffer->vertex_buffer,
        &vertex_buffer_memory_requirements
        );

    const VkMemoryAllocateInfo vertex_buffer_memory_allocate_info =
    {
        .sType              = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize     = vertex_buffer_memory_requirements.size,
        .memoryTypeIndex    = lna_vulkan_find_memory_type(physical_device, vertex_buffer_memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT),
    };
    lna_vulkan_check(
        vkAllocateMemory(
            device,
            &vertex_buffer_memory_allocate_info,
            NULL,
            &buffer->vertex_buffer_memory
            )
        );
    lna_vulkan_check(
        vkBindBufferMemory(
            device,
            buffer->vertex_buffer,
            buffer->vertex_buffer_memory,
            0
            )
        );
    lna_vulkan_check(
        vkMapMemory(
            device,
            buffer->vertex_buffer_memory,
            0,
            VK_WHOLE_SIZE,
            0,
            &buffer->vertex_data_mapped
            )
        );

    const VkDeviceSize index_buffer_size = buffer->max_index_count * sizeof (uint32_t);
    const VkBufferCreateInfo index_buffer_create_info =
    {
        .sType  = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .usage  = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        .size   = index_buffer_size,
    };
    lna_vulkan_check(
        vkCreateBuffer(
            device,
            &index_buffer_create_info,
            NULL,
            &buffer->index_buffer
            )
        );
    VkMemoryRequirements index_buffer_memory_requirements;
    vkGetBufferMemoryRequirements(
        device,
        buffer->index_buffer,
        &index_buffer_memory_requirements
        );
    const VkMemoryAllocateInfo index_buffer_memory_allocate_info =
    {
        .sType              = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize     = index_buffer_memory_requirements.size,
        .memoryTypeIndex    = lna_vulkan_find_memory_type(physical_device, index_buffer_memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT),
    };
    lna_vulkan_check(
        vkAllocateMemory(
            device,
            &index_buffer_memory_allocate_info,
            NULL,
            &buffer->index_buffer_memory
            )
        );
    lna_vulkan_check(
        vkBindBufferMemory(
            device,
            buffer->index_buffer,
            buffer->index_buffer_memory,
            0
            )
        );
    lna_vulkan_check(
        vkMapMemory(
            device,
            buffer->index_buffer_memory,
            0,
            VK_WHOLE_SIZE,
            0,
            &buffer->index_data_mapped
            )
        );

    {
        lna_ui_vertex_t* vertex_dst = (lna_ui_vertex_t*)buffer->vertex_data_mapped;
        memcpy(
            vertex_dst,
            buffer->vertices,
            buffer->max_vertex_count * sizeof(lna_ui_vertex_t)
            );
        const VkMappedMemoryRange mapped_memory_range =
        {
            .sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
            .memory = buffer->vertex_buffer_memory,
            .offset = 0,
            .size   = vertex_buffer_size,
        };
        lna_vulkan_check(
            vkFlushMappedMemoryRanges(
                device,
                1,
                &mapped_memory_range
            )   
        );
    }

    {
        uint32_t* index_dst = (uint32_t*)buffer->index_data_mapped;
        memcpy(
            index_dst,
            buffer->indices,
            buffer->max_index_count * sizeof(uint32_t)
            );
        const VkMappedMemoryRange mapped_memory_range =
        {
            .sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
            .memory = buffer->index_buffer_memory,
            .offset = 0,
            .size   = index_buffer_size,
        };
        lna_vulkan_check(
            vkFlushMappedMemoryRanges(
                device,
                1,
                &mapped_memory_range
            )   
        );
    }
}

static void lna_ui_buffer_release(lna_ui_buffer_t* buffer, VkDevice device)
{
    lna_assert(buffer)
    lna_assert(buffer->vertex_buffer)
    lna_assert(buffer->vertex_buffer_memory)
    lna_assert(buffer->index_buffer)
    lna_assert(buffer->index_buffer_memory)
    lna_assert(device)

    vkUnmapMemory(device, buffer->vertex_buffer_memory);
    vkDestroyBuffer(device, buffer->vertex_buffer, NULL);
    vkFreeMemory(device, buffer->vertex_buffer_memory, NULL);

    vkUnmapMemory(device, buffer->index_buffer_memory);
    vkDestroyBuffer(device, buffer->index_buffer, NULL);
    vkFreeMemory(device, buffer->index_buffer_memory, NULL);
}

void lna_ui_system_init(lna_ui_system_t* ui_system, const lna_ui_system_config_t* config)
{
    lna_assert(ui_system)
    lna_assert(ui_system->renderer == NULL)
    lna_assert(ui_system->buffers.cur_element_count == 0)
    lna_assert(ui_system->buffers.max_element_count == 0)
    lna_assert(ui_system->buffers.elements == NULL)
    lna_assert(ui_system->pipeline == VK_NULL_HANDLE)
    lna_assert(ui_system->pipeline_cache == VK_NULL_HANDLE)
    lna_assert(ui_system->pipeline_layout == VK_NULL_HANDLE)
    lna_assert(ui_system->descriptor_pool == VK_NULL_HANDLE)
    lna_assert(ui_system->descriptor_set_layout == VK_NULL_HANDLE)
    lna_assert(config)
    lna_assert(config->memory_pool)
    lna_assert(config->renderer)
    lna_assert(config->renderer->device)
    lna_assert(config->renderer->render_pass)
    lna_assert(config->max_buffer_count > 0)

    ui_system->renderer = config->renderer;
    
    ui_system->buffers.max_element_count    = config->max_buffer_count;
    ui_system->buffers.elements             = lna_memory_pool_reserve(
        config->memory_pool,
        sizeof(lna_ui_buffer_t) * config->max_buffer_count
        );

    //! DESCRIPTOR POOL

    const VkDescriptorPoolSize descriptor_pool_sizes[] =
    {
        {
            .type               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount    = config->max_buffer_count,
        },
    };
    const VkDescriptorPoolCreateInfo pool_create_info =
    {
        .sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount  = (uint32_t)(sizeof(descriptor_pool_sizes) / sizeof(descriptor_pool_sizes[0])),
        .pPoolSizes     = descriptor_pool_sizes,
        .maxSets        = config->max_buffer_count,
    };
    lna_vulkan_check(
        vkCreateDescriptorPool(
            config->renderer->device,
            &pool_create_info,
            NULL,
            &ui_system->descriptor_pool
            )
        );

    //! DESCRIPTOR SET LAYOUT

    const VkDescriptorSetLayoutBinding set_layout_bindings[1] =
    {
        {
            .descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT,
            .binding            = 0,
            .descriptorCount    = 1,
        },
    };
    const VkDescriptorSetLayoutCreateInfo set_layout_create_info =
    {
        .sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pBindings      = set_layout_bindings,
        .bindingCount   = 1,
    };
    lna_vulkan_check(
        vkCreateDescriptorSetLayout(
            config->renderer->device,
            &set_layout_create_info,
            NULL,
            &ui_system->descriptor_set_layout
            )
        );

    //! PIPELINE

    const VkPipelineCacheCreateInfo pipeline_cache_create_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
    };
    lna_vulkan_check(
        vkCreatePipelineCache(
            config->renderer->device,
            &pipeline_cache_create_info,
            NULL,
            &ui_system->pipeline_cache
            )
        );

    const VkPushConstantRange push_constance_range =
    {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .size       = sizeof(lna_ui_push_const_block_vulkan_t),
        .offset     = 0,
    };
    const VkPipelineLayoutCreateInfo layout_create_info =
    {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount         = 1,
        .pSetLayouts            = &ui_system->descriptor_set_layout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges    = &push_constance_range,
    };
    lna_vulkan_check(
        vkCreatePipelineLayout(
            config->renderer->device,
            &layout_create_info,
            NULL,
            &ui_system->pipeline_layout
            )
        );

    const VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info =
    {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .flags                  = 0,
        .primitiveRestartEnable = VK_FALSE,
    };
    const VkPipelineRasterizationStateCreateInfo rasterization_state_create_info =
    {
        .sType              = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode        = VK_POLYGON_MODE_FILL,
        .cullMode           = VK_CULL_MODE_NONE,
        .frontFace          = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .flags              = 0,
        .depthClampEnable   = VK_FALSE,
        .lineWidth          = 1.0f,
    };
    const VkPipelineColorBlendAttachmentState blend_attachment_state =
    {
        .blendEnable            = VK_TRUE,
        .colorWriteMask         = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .srcColorBlendFactor    = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor    = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp           = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor    = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .dstAlphaBlendFactor    = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp           = VK_BLEND_OP_ADD,
    };
    const VkPipelineColorBlendStateCreateInfo color_blend_state_create_info =
    {
        .sType              = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .attachmentCount    = 1,
        .pAttachments       = &blend_attachment_state,
    };
    const VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info =
    {
        .sType              = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable    = VK_FALSE,
        .depthWriteEnable   = VK_FALSE,
        .depthCompareOp     = VK_COMPARE_OP_LESS_OR_EQUAL,
        .back.compareOp     = VK_COMPARE_OP_ALWAYS,
    };
    const VkPipelineViewportStateCreateInfo viewport_state_create_info =
    {
        .sType          = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount  = 1,
        .scissorCount   = 1,
        .flags          = 0,
    };
    const VkPipelineMultisampleStateCreateInfo multisample_state_create_info =
    {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples   = VK_SAMPLE_COUNT_1_BIT,
        .flags                  = 0,
    };
    const VkDynamicState dynamic_states[] =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    const VkPipelineDynamicStateCreateInfo dynamic_state_create_info =
    {
        .sType              = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pDynamicStates     = dynamic_states,
        .dynamicStateCount  = 2,
        .flags              = 0,
    };
    // TODO: avoid direct file load: add binary code directly in code as a static const uint32_t* 
    lna_binary_file_content_uint32_t vertex_shader_file = { 0 };
    lna_binary_file_debug_load_uint32(
        &vertex_shader_file,
        &config->renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_FRAME],
        "shaders/ui_vert.spv"
        );
    // TODO: avoid direct file load: add binary code directly in code as a static const uint32_t* 
    lna_binary_file_content_uint32_t fragment_shader_file = { 0 };
    lna_binary_file_debug_load_uint32(
        &fragment_shader_file,
        &config->renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_FRAME],
        "shaders/ui_frag.spv"
        );
    
    VkShaderModule vertex_shader_module = lna_vulkan_create_shader_module(
        config->renderer->device,
        vertex_shader_file.content,
        vertex_shader_file.size
        );
    VkShaderModule fragment_shader_module = lna_vulkan_create_shader_module(
        config->renderer->device,
        fragment_shader_file.content,
        fragment_shader_file.size
        );
    const VkPipelineShaderStageCreateInfo shader_stage_create_infos[2] =
    {
        {
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertex_shader_module,
            .pName  = "main",
        },
        {
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragment_shader_module,
            .pName  = "main",
        },
    };
    const VkVertexInputBindingDescription vertex_input_binding_descriptions[1] =
    {
        {
            .binding    = 0,
            .stride     = sizeof(lna_ui_vertex_t),
            .inputRate  = VK_VERTEX_INPUT_RATE_VERTEX,
        }
    };
    const VkVertexInputAttributeDescription vertex_input_attribute_descriptions[3] =
    {
        {
            .binding    = 0,
            .location   = 0,
            .format     = VK_FORMAT_R32G32_SFLOAT,
            .offset     = offsetof(lna_ui_vertex_t, position),
        },
        {
            .binding    = 0,
            .location   = 1,
            .format     = VK_FORMAT_R32G32_SFLOAT,
            .offset     = offsetof(lna_ui_vertex_t, uv),
        },
        {
            .binding    = 0,
            .location   = 2,
            .format     = VK_FORMAT_R32G32B32A32_SFLOAT,
            .offset     = offsetof(lna_ui_vertex_t, color),
        },
    };
    const VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_create_info =
    {
        .sType                              = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount      = 1,
        .pVertexBindingDescriptions         = vertex_input_binding_descriptions,
        .vertexAttributeDescriptionCount    = 3,
        .pVertexAttributeDescriptions       = vertex_input_attribute_descriptions,
    };
    const VkGraphicsPipelineCreateInfo pipeline_create_info =
    {
        .sType                  = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .layout                 = ui_system->pipeline_layout,
        .renderPass             = config->renderer->render_pass,
        .flags                  = 0,
        .basePipelineIndex      = -1,
        .basePipelineHandle     = VK_NULL_HANDLE,
        .pInputAssemblyState    = &input_assembly_state_create_info,
        .pRasterizationState    = &rasterization_state_create_info,
        .pColorBlendState       = &color_blend_state_create_info,
        .pMultisampleState      = &multisample_state_create_info,
        .pViewportState         = &viewport_state_create_info,
        .pDepthStencilState     = &depth_stencil_state_create_info,
        .pDynamicState          = &dynamic_state_create_info,
        .stageCount             = 2,
        .pStages                = shader_stage_create_infos,
        .pVertexInputState      = &pipeline_vertex_input_state_create_info,
    };

    lna_vulkan_check(
        vkCreateGraphicsPipelines(
            config->renderer->device,
            ui_system->pipeline_cache,
            1,
            &pipeline_create_info,
            NULL,
            &ui_system->pipeline
            )
        );

    vkDestroyShaderModule(config->renderer->device, fragment_shader_module, NULL);
    vkDestroyShaderModule(config->renderer->device, vertex_shader_module, NULL);
}

lna_ui_buffer_t* lna_ui_system_new_buffer(lna_ui_system_t* ui_system, const lna_ui_buffer_config_t* config)
{
    lna_assert(ui_system)
    lna_assert(ui_system->buffers.elements)
    lna_assert(ui_system->buffers.cur_element_count < ui_system->buffers.max_element_count)
    lna_assert(ui_system->renderer)
    
    lna_ui_buffer_t* buffer = &ui_system->buffers.elements[ui_system->buffers.cur_element_count++];
    lna_ui_buffer_init(
        buffer,
        config,
        ui_system->descriptor_pool,
        ui_system->descriptor_set_layout,
        ui_system->renderer->device,
        ui_system->renderer->physical_device
        );
    return buffer;
}

void lna_ui_system_draw(lna_ui_system_t* ui_system)
{
    lna_assert(ui_system)
    lna_assert(ui_system->renderer)
    lna_assert(ui_system->renderer->command_buffers.elements)
    lna_assert(ui_system->renderer->command_buffers.count > ui_system->renderer->image_index)

    const VkViewport viewport =
    {
        .width      = (float)ui_system->renderer->swap_chain_extent.width,
	    .height     = (float)ui_system->renderer->swap_chain_extent.height,
	    .minDepth   = 0.0f,
	    .maxDepth   = 1.0f,
    };
    const VkRect2D scissor_rect =
    {
        .offset.x       = 0,
        .offset.y       = 0,
        .extent.width   = ui_system->renderer->swap_chain_extent.width,
        .extent.height  = ui_system->renderer->swap_chain_extent.height,
    };

    VkCommandBuffer command_buffer = ui_system->renderer->command_buffers.elements[ui_system->renderer->image_index];

    for (uint32_t i = 0; i < ui_system->buffers.cur_element_count; ++i)
    {
        lna_ui_buffer_t* buffer = &ui_system->buffers.elements[i];

        //! 1. UPDATE MAPPED DATA

        {
            lna_ui_vertex_t* vertex_dst = (lna_ui_vertex_t*)buffer->vertex_data_mapped;
            memcpy(
                vertex_dst,
                buffer->vertices,
                buffer->max_vertex_count * sizeof(lna_ui_vertex_t)
                );
            const VkDeviceSize vertex_buffer_size = buffer->max_vertex_count * sizeof(lna_ui_vertex_t);
            const VkMappedMemoryRange mapped_memory_range =
            {
                .sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                .memory = buffer->vertex_buffer_memory,
                .offset = 0,
                .size   = vertex_buffer_size,
            };
            lna_vulkan_check(
                vkFlushMappedMemoryRanges(
                    ui_system->renderer->device,
                    1,
                    &mapped_memory_range
                    )   
                );
        }

        {
            uint32_t* index_dst = (uint32_t*)buffer->index_data_mapped;
            memcpy(
                index_dst,
                buffer->indices,
                buffer->max_index_count * sizeof(uint32_t)
                );
            const VkDeviceSize index_buffer_size = buffer->max_index_count * sizeof (uint32_t);
            const VkMappedMemoryRange mapped_memory_range =
            {
                .sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                .memory = buffer->index_buffer_memory,
                .offset = 0,
                .size   = index_buffer_size,
            };
            lna_vulkan_check(
                vkFlushMappedMemoryRanges(
                    ui_system->renderer->device,
                    1,
                    &mapped_memory_range
                    )   
                );
        }

        //! 2. UPDATE CURRENT COMMAND BUFFER

        vkCmdBindDescriptorSets(
            command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            ui_system->pipeline_layout,
            0,
            1,
            &buffer->descriptor_set,
            0,
            NULL
            );
        vkCmdBindPipeline(
            command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            ui_system->pipeline
            );
        vkCmdSetViewport(
            command_buffer,
            0,
            1,
            &viewport
            );
        
        buffer->push_const_block.scale.x        = 1.0f;
        buffer->push_const_block.scale.y        = 1.0f;
        buffer->push_const_block.translate.x    = 0.0f;
        buffer->push_const_block.translate.y    = 0.0f;
        vkCmdPushConstants(
            command_buffer,
            ui_system->pipeline_layout,
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(lna_ui_push_const_block_vulkan_t),
            &buffer->push_const_block
            );

        if (buffer->cur_index_count > 0)
        {
            const VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(
                command_buffer,
                0,
                1,
                &buffer->vertex_buffer,
                offsets
                );
            vkCmdBindIndexBuffer(
                command_buffer,
                buffer->index_buffer,
                0,
                VK_INDEX_TYPE_UINT32
                );

            vkCmdSetScissor(
                command_buffer,
                0,
                1,
                &scissor_rect
                );
            vkCmdDrawIndexed(
                command_buffer,
                buffer->cur_index_count,
                1,
                0,
                0,
                0
                );
        }
    }
}

void lna_ui_system_release(lna_ui_system_t* ui_system)
{
    lna_assert(ui_system)
    lna_assert(ui_system->buffers.elements)
    lna_assert(ui_system->pipeline_cache)
    lna_assert(ui_system->pipeline)
    lna_assert(ui_system->pipeline_layout)
    lna_assert(ui_system->descriptor_pool)
    lna_assert(ui_system->descriptor_set_layout)
    lna_assert(ui_system->renderer)
    lna_assert(ui_system->renderer->device)

    for (uint32_t index = 0; index < ui_system->buffers.cur_element_count; ++index)
    {
        lna_ui_buffer_t* buffer = &ui_system->buffers.elements[index];
        lna_ui_buffer_release(buffer, ui_system->renderer->device);
    }

    vkDestroyPipelineCache(ui_system->renderer->device, ui_system->pipeline_cache, NULL);
    vkDestroyPipeline(ui_system->renderer->device, ui_system->pipeline, NULL);
    vkDestroyPipelineLayout(ui_system->renderer->device, ui_system->pipeline_layout, NULL);
    vkDestroyDescriptorPool(ui_system->renderer->device, ui_system->descriptor_pool, NULL);
    vkDestroyDescriptorSetLayout(ui_system->renderer->device, ui_system->descriptor_set_layout, NULL);
}

static const uint32_t LNA_UI_VERTEX_COUNT_PER_RECT   = 4;
static const uint32_t LNA_UI_INDEX_COUNT_PER_RECT    = 6;

void lna_ui_buffer_push_rect(lna_ui_buffer_t* buffer, const lna_ui_buffer_rect_config_t* config)
{
    lna_assert(buffer)
    lna_assert(buffer->vertices)
    lna_assert(buffer->indices)
    lna_assert(buffer->cur_vertex_count + LNA_UI_VERTEX_COUNT_PER_RECT <= buffer->max_vertex_count)
    lna_assert(buffer->cur_index_count + LNA_UI_INDEX_COUNT_PER_RECT <= buffer->max_index_count)
    lna_assert(config)
    lna_assert(config->position)
    lna_assert(config->size)
    lna_assert(config->color)
    lna_assert(config->window_size)

    lna_vec2_t position =
    {
        2.0f * config->position->x / config->window_size->width - 1.0f,
        2.0f * config->position->y / config->window_size->height - 1.0f,
    };
    lna_vec2_t size =
    {
        2.0f * config->size->x / config->window_size->width,
        2.0f * config->size->y / config->window_size->height,
    };

    buffer->vertices[buffer->cur_vertex_count].position     = position;
    buffer->vertices[buffer->cur_vertex_count].uv           = (lna_vec2_t){ 0.0f, 0.0f };
    buffer->vertices[buffer->cur_vertex_count].color        = *config->color;

    buffer->vertices[buffer->cur_vertex_count + 1].position = (lna_vec2_t){ position.x, position.y + size.height };
    buffer->vertices[buffer->cur_vertex_count + 1].uv       = (lna_vec2_t){ 0.0f, 0.0f };
    buffer->vertices[buffer->cur_vertex_count + 1].color    = *config->color;

    buffer->vertices[buffer->cur_vertex_count + 2].position = (lna_vec2_t){ position.x + size.width, position.y + size.height };
    buffer->vertices[buffer->cur_vertex_count + 2].uv       = (lna_vec2_t){ 0.0f, 0.0f };
    buffer->vertices[buffer->cur_vertex_count + 2].color    = *config->color;

    buffer->vertices[buffer->cur_vertex_count + 3].position = (lna_vec2_t){ position.x + size.width, position.y };
    buffer->vertices[buffer->cur_vertex_count + 3].uv       = (lna_vec2_t){ 0.0f, 0.0f };
    buffer->vertices[buffer->cur_vertex_count + 3].color    = *config->color;

    buffer->indices[buffer->cur_index_count]                = buffer->cur_vertex_count;
    buffer->indices[buffer->cur_index_count + 1]            = buffer->cur_vertex_count + 1;
    buffer->indices[buffer->cur_index_count + 2]            = buffer->cur_vertex_count + 2;
    buffer->indices[buffer->cur_index_count + 3]            = buffer->cur_vertex_count + 2;
    buffer->indices[buffer->cur_index_count + 4]            = buffer->cur_vertex_count + 3;
    buffer->indices[buffer->cur_index_count + 5]            = buffer->cur_vertex_count;

    buffer->cur_vertex_count += LNA_UI_VERTEX_COUNT_PER_RECT;
    buffer->cur_index_count += LNA_UI_INDEX_COUNT_PER_RECT;
}

void lna_ui_buffer_push_text(lna_ui_buffer_t* buffer, const lna_ui_buffer_text_config_t* config)
{
    lna_assert(buffer)
    lna_assert(config)
    lna_assert(config->text)
    lna_assert(config->position)
    lna_assert(config->color)
    lna_assert(config->window_size)

    size_t      text_length         = strlen(config->text);
    lna_vec2_t  texture_offset_pos  = { 0.0f, 0.0f };

    lna_assert(buffer->cur_vertex_count + ((uint32_t)text_length * LNA_UI_VERTEX_COUNT_PER_RECT) < buffer->max_vertex_count)
    lna_assert(buffer->cur_vertex_count + ((uint32_t)text_length * LNA_UI_INDEX_COUNT_PER_RECT) < buffer->max_vertex_count)

    lna_vec2_t position =
    {
        2.0f * config->position->x / config->window_size->width - 1.0f,
        2.0f * config->position->y / config->window_size->height - 1.0f,
    };
    lna_vec2_t size =
    {
        2.0f * config->size / config->window_size->width,
        2.0f * config->size / config->window_size->height,
    };
    float leading = config->leading / (config->window_size->height * 0.5f);
    float spacing = config->spacing / (config->window_size->width * 0.5f);

    for (size_t i = 0; i < text_length; ++i)
    {
        if (config->text[i] == '\n')
        {
            position.x = config->position->x / config->window_size->width - 1.0f;
            position.y += leading;
        }
        else
        {
            texture_offset_pos = (lna_vec2_t)
            {
                (float)((uint32_t)config->text[i] % config->texture_col_char_count) * config->uv_char_size->width,
                (float)((uint32_t)config->text[i] / config->texture_row_char_count) * config->uv_char_size->height
            };

            buffer->vertices[buffer->cur_vertex_count].position     = position;
            buffer->vertices[buffer->cur_vertex_count].uv           = texture_offset_pos;
            buffer->vertices[buffer->cur_vertex_count].color        = *config->color;

            buffer->vertices[buffer->cur_vertex_count + 1].position = (lna_vec2_t){ position.x, position.y + size.height };
            buffer->vertices[buffer->cur_vertex_count + 1].uv       = (lna_vec2_t){ texture_offset_pos.x, texture_offset_pos.y + config->uv_char_size->height };
            buffer->vertices[buffer->cur_vertex_count + 1].color    = *config->color;

            buffer->vertices[buffer->cur_vertex_count + 2].position = (lna_vec2_t){ position.x + size.width, position.y + size.height };
            buffer->vertices[buffer->cur_vertex_count + 2].uv       = (lna_vec2_t){ texture_offset_pos.x + config->uv_char_size->width, texture_offset_pos.y + config->uv_char_size->height };
            buffer->vertices[buffer->cur_vertex_count + 2].color    = *config->color;

            buffer->vertices[buffer->cur_vertex_count + 3].position = (lna_vec2_t){ position.x + size.width, position.y };
            buffer->vertices[buffer->cur_vertex_count + 3].uv       = (lna_vec2_t){ texture_offset_pos.x + config->uv_char_size->width, texture_offset_pos.y };
            buffer->vertices[buffer->cur_vertex_count + 3].color    = *config->color;

            buffer->indices[buffer->cur_index_count]                = buffer->cur_vertex_count;
            buffer->indices[buffer->cur_index_count + 1]            = buffer->cur_vertex_count + 1;
            buffer->indices[buffer->cur_index_count + 2]            = buffer->cur_vertex_count + 2;
            buffer->indices[buffer->cur_index_count + 3]            = buffer->cur_vertex_count + 2;
            buffer->indices[buffer->cur_index_count + 4]            = buffer->cur_vertex_count + 3;
            buffer->indices[buffer->cur_index_count + 5]            = buffer->cur_vertex_count;

            buffer->cur_vertex_count    += LNA_UI_VERTEX_COUNT_PER_RECT;
            buffer->cur_index_count     += LNA_UI_INDEX_COUNT_PER_RECT;

            position.x += size.width + spacing;
        }
    }
}

void lna_ui_buffer_empty(lna_ui_buffer_t* buffer)
{
    lna_assert(buffer)
    buffer->cur_vertex_count = 0;
    buffer->cur_index_count = 0;
}
