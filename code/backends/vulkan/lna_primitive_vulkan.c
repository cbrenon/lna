#include <string.h>
#include <math.h>
#include "graphics/lna_primitive.h"
#include "backends/vulkan/lna_primitive_vulkan.h"
#include "backends/vulkan/lna_vulkan.h"
#include "core/lna_assert.h"
#include "core/lna_memory_pool.h"
#include "core/lna_file.h"
#include "maths/lna_mat4.h"
#include "maths/lna_maths.h"

typedef struct lna_primitive_uniform_s
{
    lna_mat4_t  model;
    lna_mat4_t  view;
    lna_mat4_t  projection;
} lna_primitive_uniform_t;

static void lna_primitive_system_create_graphics_pipeline(
    lna_primitive_system_t* primitive_system,
    lna_renderer_t* renderer
    )
{
    lna_assert(primitive_system)
    lna_assert(renderer)

    // TODO: avoid direct file load --------------------------------------------
    lna_binary_file_content_uint32_t vertex_shader_file = { 0 };
    lna_binary_file_debug_load_uint32(
        &vertex_shader_file,
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_FRAME],
        "shaders/debug_primitive_vert.spv"
        );
    lna_binary_file_content_uint32_t fragment_shader_file = { 0 };
    lna_binary_file_debug_load_uint32(
        &fragment_shader_file,
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_FRAME],
        "shaders/debug_primitive_frag.spv"
        );
    // -------------------------------------------------------------------------
    VkShaderModule vertex_shader_module = lna_vulkan_create_shader_module(
        renderer->device,
        lna_array_ptr(&vertex_shader_file),
        lna_array_size(&vertex_shader_file)
        );
    VkShaderModule fragment_shader_module = lna_vulkan_create_shader_module(
        renderer->device,
        lna_array_ptr(&fragment_shader_file),
        lna_array_size(&fragment_shader_file)
        );
    const VkPipelineShaderStageCreateInfo shader_stage_create_infos[] =
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
    const VkVertexInputAttributeDescription vertex_input_attribute_descriptions[] =
    {
        {
            .binding = 0,
            .location = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(lna_primitive_vertex_t, position),
        },
        {
            .binding = 0,
            .location = 1,
            .format = VK_FORMAT_R32G32B32A32_SFLOAT,
            .offset = offsetof(lna_primitive_vertex_t, color),
        },
    };
    const VkVertexInputBindingDescription vertex_input_binding_description[] =
    {
        {
            .binding = 0,
            .stride = sizeof(lna_primitive_vertex_t),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        },
    };
    const VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = (uint32_t)(sizeof(vertex_input_binding_description) / sizeof(vertex_input_binding_description[0])),
        .pVertexBindingDescriptions = vertex_input_binding_description,
        .vertexAttributeDescriptionCount = (uint32_t)(sizeof(vertex_input_attribute_descriptions) / sizeof(vertex_input_attribute_descriptions[0])),
        .pVertexAttributeDescriptions = vertex_input_attribute_descriptions,
    };
    const VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = primitive_system->fill_shapes ? VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST : VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };
    const VkViewport viewport =
    {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)(renderer->swap_chain_extent.width),
        .height = (float)(renderer->swap_chain_extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    const VkRect2D scissor =
    {
        .offset = {0, 0},
        .extent = renderer->swap_chain_extent,
    };
    const VkPipelineViewportStateCreateInfo viewport_state_create_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor,
    };
    const VkPipelineRasterizationStateCreateInfo rasterization_state_create_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = primitive_system->fill_shapes ? VK_POLYGON_MODE_FILL : VK_POLYGON_MODE_LINE,
        .lineWidth = 2.0f,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
    };
    const VkPipelineMultisampleStateCreateInfo multisample_state_create_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .minSampleShading = 1.0f,
        .pSampleMask = NULL,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
    };
    const VkPipelineColorBlendAttachmentState color_blend_attachment_state =
    {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
    };
    const VkPipelineColorBlendStateCreateInfo color_blender_state_create_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment_state,
        .blendConstants[0] = 0.0f,
        .blendConstants[1] = 0.0f,
        .blendConstants[2] = 0.0f,
        .blendConstants[3] = 0.0f,
    };
    const VkDynamicState dynamic_states[] =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_LINE_WIDTH
    };
    const VkPipelineDynamicStateCreateInfo dynamic_state_create_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates = dynamic_states,
    };
    const VkPipelineLayoutCreateInfo pipeline_layout_create_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &primitive_system->descriptor_set_layout,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = NULL,
    };
    VULKAN_CHECK_RESULT(
        vkCreatePipelineLayout(
            renderer->device,
            &pipeline_layout_create_info,
            NULL,
            &primitive_system->pipeline_layout
            )
        )
    const VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info =
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 1.0f,
        .stencilTestEnable = VK_FALSE,
        .front = { 0 },
        .back = { 0 },

    };
    const VkGraphicsPipelineCreateInfo graphics_pipeline_create_info =
    {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shader_stage_create_infos,
        .pVertexInputState = &vertex_input_state_create_info,
        .pInputAssemblyState = &input_assembly_state_create_info,
        .pViewportState = &viewport_state_create_info,
        .pRasterizationState = &rasterization_state_create_info,
        .pMultisampleState = &multisample_state_create_info,
        .pDepthStencilState = &depth_stencil_state_create_info,
        .pColorBlendState = &color_blender_state_create_info,
        .pDynamicState = &dynamic_state_create_info,
        .layout = primitive_system->pipeline_layout,
        .renderPass = renderer->render_pass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1,
    };
    VULKAN_CHECK_RESULT(
        vkCreateGraphicsPipelines(
            renderer->device,
            VK_NULL_HANDLE,
            1,
            &graphics_pipeline_create_info,
            NULL,
            &primitive_system->pipeline
            )
        )

    vkDestroyShaderModule(renderer->device, fragment_shader_module, NULL);
    vkDestroyShaderModule(renderer->device, vertex_shader_module, NULL);
}

static void lna_primitive_system_create_descriptor_pool(
    lna_primitive_system_t* primitive_system,
    lna_renderer_t* renderer
    )
{
    lna_assert(primitive_system)
    lna_assert(renderer)

    const VkDescriptorPoolSize pool_sizes[] =
    {
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = lna_array_size(&renderer->swap_chain_images) * lna_vector_max_capacity(&primitive_system->primitives),
        },
    };

    const VkDescriptorPoolCreateInfo pool_create_info =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = (uint32_t)(sizeof(pool_sizes) / sizeof(pool_sizes[0])),
        .pPoolSizes = pool_sizes,
        .maxSets = lna_array_size(&renderer->swap_chain_images) * lna_vector_max_capacity(&primitive_system->primitives),
    };

    VULKAN_CHECK_RESULT(
        vkCreateDescriptorPool(
            renderer->device,
            &pool_create_info,
            NULL,
            &primitive_system->descriptor_pool
            )
        )

    lna_log_message("-------------------------------");
    lna_log_message("primitive descriptor pool info:");
    lna_log_message("-------------------------------");
    lna_log_message("\tdescriptor pool address: %p", (void*)primitive_system->descriptor_pool);
    lna_log_message("\tmax sets               : %d", pool_create_info.maxSets);
    lna_log_message("\tpool size count        : %d", pool_create_info.poolSizeCount);
    for (uint32_t i = 0; i < pool_create_info.poolSizeCount; ++i)
    {
        lna_log_message("\tpool %d descriptor count: %d", i, pool_sizes[i].descriptorCount);
    }
}

static void lna_primitive_create_uniform_buffer(
    lna_primitive_t* primitive,
    lna_renderer_t* renderer
    )
{
    lna_assert(primitive)
    lna_assert(lna_array_size(&primitive->mvp_uniform_buffers) == 0)
    lna_assert(lna_array_size(&primitive->mvp_uniform_buffers_memory) == 0)

    lna_array_init(
        &primitive->mvp_uniform_buffers,
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_SWAP_CHAIN],
        VkBuffer,
        lna_array_size(&renderer->swap_chain_images)
        );

    lna_array_init(
        &primitive->mvp_uniform_buffers_memory,
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_SWAP_CHAIN],
        VkDeviceMemory,
        lna_array_size(&renderer->swap_chain_images)
        );

    VkDeviceSize uniform_buffer_size = sizeof(lna_primitive_uniform_t);
    for (size_t i = 0; i < lna_array_size(&primitive->mvp_uniform_buffers); ++i)
    {
        lna_vulkan_create_buffer(
            renderer->device,
            renderer->physical_device,
            uniform_buffer_size,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            lna_array_at_ptr(&primitive->mvp_uniform_buffers, i),
            lna_array_at_ptr(&primitive->mvp_uniform_buffers_memory, i)
            );
    }
}

static void lna_primitive_create_descriptor_sets(
    lna_primitive_t* primitive,
    lna_primitive_system_t* primitive_system
    )
{
    lna_assert(primitive)
    lna_assert(primitive_system)
    lna_assert(lna_array_size(&primitive->descriptor_sets) == 0)

    lna_renderer_t* renderer = primitive_system->renderer;
    lna_assert(renderer)

    lna_vulkan_descriptor_set_layout_array_t layouts = { 0 };
    lna_array_init(
        &layouts,
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_FRAME],
        VkDescriptorSetLayout,
        lna_array_size(&renderer->swap_chain_images)
        );
    for (uint32_t i = 0; i < lna_array_size(&layouts); ++i)
    {
        lna_array_at_ref(&layouts, i) = primitive_system->descriptor_set_layout;
    }

    const VkDescriptorSetAllocateInfo allocate_info =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = primitive_system->descriptor_pool,
        .descriptorSetCount = lna_array_size(&layouts),
        .pSetLayouts = lna_array_ptr(&layouts),
    };

    lna_array_init(
        &primitive->descriptor_sets,
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_SWAP_CHAIN],
        VkDescriptorSet,
        lna_array_size(&renderer->swap_chain_images)
        );

    VULKAN_CHECK_RESULT(
        vkAllocateDescriptorSets(
            renderer->device,
            &allocate_info,
            lna_array_ptr(&primitive->descriptor_sets)
            )
        )

    for (size_t i = 0; i < lna_array_size(&primitive->descriptor_sets); ++i)
    {
        const VkDescriptorBufferInfo buffer_info =
        {
            .buffer = lna_array_at_ref(&primitive->mvp_uniform_buffers, i),
            .offset = 0,
            .range = sizeof(lna_primitive_uniform_t),
        };
        const VkWriteDescriptorSet write_descriptors[] =
        {
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = lna_array_at_ref(&primitive->descriptor_sets, i),
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .pBufferInfo = &buffer_info,
                .pImageInfo = NULL,
                .pTexelBufferView = NULL,
            },
        };

        vkUpdateDescriptorSets(
            renderer->device,
            (uint32_t)(sizeof(write_descriptors) / sizeof(write_descriptors[0])),
            write_descriptors,
            0,
            NULL
            );
    }
}

static void lna_primitive_system_on_swap_chain_cleanup(void* owner)
{
    lna_assert(owner)
    lna_primitive_system_t* primitive_system = (lna_primitive_system_t*)owner;
    lna_renderer_t* renderer = primitive_system->renderer;
    lna_assert(renderer)

    vkDestroyPipeline(
        renderer->device,
        primitive_system->pipeline,
        NULL
        );
    vkDestroyPipelineLayout(
        renderer->device,
        primitive_system->pipeline_layout,
        NULL
        );

    for (uint32_t i = 0; i < lna_vector_size(&primitive_system->primitives); ++i)
    {
        lna_primitive_t* primitive = lna_vector_at_ptr(&primitive_system->primitives, i);
        for (size_t j = 0; j < lna_array_size(&primitive->mvp_uniform_buffers); ++j)
        {
            vkDestroyBuffer(
                renderer->device,
                lna_array_at_ref(&primitive->mvp_uniform_buffers, j),
                NULL
                );
        }
        for (size_t j = 0; j < lna_array_size(&primitive->mvp_uniform_buffers); ++j)
        {
            vkFreeMemory(
                renderer->device,
                lna_array_at_ref(&primitive->mvp_uniform_buffers_memory, j),
                NULL
                );
        }
        lna_array_release(&primitive->mvp_uniform_buffers);
        lna_array_release(&primitive->mvp_uniform_buffers_memory);
        //! NOTE: no need to explicity clean up vulkan descriptor sets objects
        //! because it is done when the vulkan descriptor pool is destroyed.
        //! we just have to reset the array and wait for filling it again.
        //! the memory pool where we reserved memory for descriptor_sets has already
        //! been reset by the vulkan renderer backend.
        lna_array_release(&primitive->descriptor_sets);
    }
    vkDestroyDescriptorPool(
        renderer->device,
        primitive_system->descriptor_pool,
        NULL
        );
}

static void lna_primitive_system_on_swap_chain_recreate(void *owner)
{
    lna_assert(owner)
    lna_primitive_system_t* primitive_system = (lna_primitive_system_t*)owner;
    lna_renderer_t* renderer = primitive_system->renderer;
    lna_assert(renderer)

    lna_primitive_system_create_graphics_pipeline(
        primitive_system,
        renderer
        );
    lna_primitive_system_create_descriptor_pool(
        primitive_system,
        renderer
        );

    for (uint32_t i = 0; i < lna_vector_size(&primitive_system->primitives); ++i)
    {
        lna_primitive_create_uniform_buffer(
            lna_vector_at_ptr(&primitive_system->primitives, i),
            renderer
            );
        lna_primitive_create_descriptor_sets(
            lna_vector_at_ptr(&primitive_system->primitives, i),
            primitive_system
            );
    }
}

void lna_primitive_system_init(lna_primitive_system_t* primitive_system, const lna_primitive_system_config_t* config)
{
    lna_assert(primitive_system)
    lna_assert(primitive_system->renderer == NULL)
    lna_assert(lna_vector_max_capacity(&primitive_system->primitives) == 0)
    lna_assert(primitive_system->descriptor_pool == VK_NULL_HANDLE)
    lna_assert(primitive_system->descriptor_set_layout == VK_NULL_HANDLE)
    lna_assert(primitive_system->pipeline == VK_NULL_HANDLE)
    lna_assert(primitive_system->pipeline_layout == VK_NULL_HANDLE)
    lna_assert(config)
    lna_assert(config->renderer)
    lna_assert(config->renderer->device)
    lna_assert(config->renderer->render_pass)

    primitive_system->renderer = config->renderer;
    primitive_system->fill_shapes = config->fill_shapes;

    lna_renderer_listener_t* listener = NULL;
    lna_vector_new_element(
        &config->renderer->listeners,
        listener
        );
    listener->handle        = (void*)primitive_system;
    listener->on_cleanup    = lna_primitive_system_on_swap_chain_cleanup;
    listener->on_recreate   = lna_primitive_system_on_swap_chain_recreate;

    lna_vector_init(
        &primitive_system->primitives,
        config->memory_pool,
        lna_primitive_t,
        config->max_primitive_count
        );

    //! DESCRIPTOR SET LAYOUT

    const VkDescriptorSetLayoutBinding bindings[] =
    {
        {
            .binding = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = NULL,
        },
    };
    const VkDescriptorSetLayoutCreateInfo layout_create_info =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = (uint32_t)(sizeof(bindings) / sizeof(bindings[0])),
        .pBindings = bindings,
    };
    VULKAN_CHECK_RESULT(
        vkCreateDescriptorSetLayout(
            config->renderer->device,
            &layout_create_info,
            NULL,
            &primitive_system->descriptor_set_layout
            )
        )

    //! GRAPHICS PIPELINE

    lna_primitive_system_create_graphics_pipeline(
        primitive_system,
        config->renderer
        );

    //! DESCRIPTOR POOL

    lna_primitive_system_create_descriptor_pool(
        primitive_system,
        config->renderer
        );
}

void lna_primitive_system_draw(lna_primitive_system_t* primitive_system)
{
    lna_assert(primitive_system)

    lna_renderer_t* renderer = primitive_system->renderer;
    lna_assert(renderer)

    VkCommandBuffer command_buffer = lna_array_at_ref(&primitive_system->renderer->command_buffers, primitive_system->renderer->image_index);

    vkCmdSetLineWidth(
        command_buffer,
        2.0f
        );

    vkCmdBindPipeline(
        command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        primitive_system->pipeline
        );
    for (uint32_t i = 0; i < lna_vector_size(&primitive_system->primitives); ++i)
    {
        lna_primitive_t* primitive = lna_vector_at_ptr(&primitive_system->primitives, i);

        lna_assert(primitive->model_matrix)
        lna_assert(primitive->view_matrix)
        lna_assert(primitive->projection_matrix)

        const lna_primitive_uniform_t ubo =
        {
            .model = *primitive->model_matrix,
            .view = *primitive->view_matrix,
            .projection = *primitive->projection_matrix,
        };
        void *data;
        VULKAN_CHECK_RESULT(
            vkMapMemory(
                renderer->device,
                lna_array_at_ref(&primitive->mvp_uniform_buffers_memory, renderer->image_index),
                0,
                sizeof(ubo),
                0,
                &data
                )
            )
        memcpy(
            data,
            &ubo,
            sizeof(ubo));
        vkUnmapMemory(
            renderer->device,
            lna_array_at_ref(&primitive->mvp_uniform_buffers_memory, renderer->image_index)
            );

        vkCmdBindDescriptorSets(
            command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            primitive_system->pipeline_layout,
            0,
            1,
            lna_array_at_ptr(&primitive->descriptor_sets, renderer->image_index),
            0,
            NULL
            );
        const VkBuffer vertex_buffers[] =
        {
            primitive->vertex_buffer
        };
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(
            command_buffer,
            0,
            1,
            vertex_buffers,
            offsets
            );
        vkCmdBindIndexBuffer(
            command_buffer,
            primitive->index_buffer,
            0,
            VK_INDEX_TYPE_UINT32
            );
        vkCmdDrawIndexed(
            command_buffer,
            primitive->index_count,
            1,
            0,
            0,
            0
            );
    }
}

void lna_primitive_system_release(lna_primitive_system_t* primitive_system)
{
    lna_assert(primitive_system)
    lna_assert(primitive_system->renderer)
    lna_assert(primitive_system->renderer->device)

    vkDestroyDescriptorSetLayout(
        primitive_system->renderer->device,
        primitive_system->descriptor_set_layout,
        NULL
        );
    for (uint32_t i = 0; i < lna_vector_size(&primitive_system->primitives); ++i)
    {
        lna_primitive_t* primitive = lna_vector_at_ptr(&primitive_system->primitives, i);

        vkDestroyBuffer(
            primitive_system->renderer->device,
            primitive->index_buffer,
            NULL
            );
        vkFreeMemory(
            primitive_system->renderer->device,
            primitive->index_buffer_memory,
            NULL
            );
        vkDestroyBuffer(
            primitive_system->renderer->device,
            primitive->vertex_buffer,
            NULL
            );
        vkFreeMemory(
            primitive_system->renderer->device,
            primitive->vertex_buffer_memory,
            NULL
            );
    }
}

lna_primitive_t* lna_primitive_system_new_raw(lna_primitive_system_t* primitive_system, const lna_primitive_raw_config_t* config)
{
    lna_assert(primitive_system)
    lna_assert(config)

    lna_primitive_t* primitive = NULL;
    lna_vector_new_element(&primitive_system->primitives, primitive);

    lna_assert(primitive)
    lna_assert(primitive->vertex_buffer == VK_NULL_HANDLE)
    lna_assert(primitive->vertex_buffer_memory == VK_NULL_HANDLE)
    lna_assert(primitive->index_buffer == VK_NULL_HANDLE)
    lna_assert(primitive->index_buffer_memory == VK_NULL_HANDLE)
    lna_assert(lna_array_is_empty(&primitive->mvp_uniform_buffers))
    lna_assert(lna_array_is_empty(&primitive->mvp_uniform_buffers_memory))
    lna_assert(lna_array_is_empty(&primitive->descriptor_sets))
    lna_assert(primitive->model_matrix == NULL)
    lna_assert(primitive->view_matrix == NULL)
    lna_assert(primitive->projection_matrix == NULL)
    lna_assert(config)
    lna_assert(config->vertices)
    lna_assert(config->indices)
    lna_assert(config->vertex_count > 0)
    lna_assert(config->index_count > 0)
    lna_assert(config->model_matrix)
    lna_assert(config->view_matrix)
    lna_assert(config->projection_matrix)
    lna_assert(primitive_system)

    lna_renderer_t* renderer = primitive_system->renderer;
    lna_assert(renderer)

    primitive->model_matrix        = config->model_matrix;
    primitive->view_matrix         = config->view_matrix;
    primitive->projection_matrix   = config->projection_matrix;

    //! VERTEX BUFFER PART

    {
        size_t vertex_buffer_size = sizeof(config->vertices[0]) * config->vertex_count;
        
        VkBuffer staging_buffer;
        VkDeviceMemory staging_buffer_memory;
        lna_vulkan_create_buffer(
            renderer->device,
            renderer->physical_device,
            vertex_buffer_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &staging_buffer,
            &staging_buffer_memory
            );
        void *vertices_data;
        VULKAN_CHECK_RESULT(
            vkMapMemory(
                renderer->device,
                staging_buffer_memory,
                0,
                vertex_buffer_size,
                0,
                &vertices_data
                )
            )
        memcpy(
            vertices_data,
            config->vertices,
            vertex_buffer_size
            );
        vkUnmapMemory(
            renderer->device,
            staging_buffer_memory
            );
        lna_vulkan_create_buffer(
            renderer->device,
            renderer->physical_device,
            vertex_buffer_size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            &primitive->vertex_buffer,
            &primitive->vertex_buffer_memory
            );
        lna_vulkan_copy_buffer(
            renderer->device,
            renderer->command_pool,
            renderer->graphics_queue,
            staging_buffer,
            primitive->vertex_buffer,
            vertex_buffer_size
            );
        vkDestroyBuffer(
            renderer->device,
            staging_buffer,
            NULL
            );
        vkFreeMemory(
            renderer->device,
            staging_buffer_memory,
            NULL
            );
    }

    //! INDEX BUFFER PART

    {
        const size_t index_buffer_size = sizeof(config->indices[0]) * config->index_count;

        primitive->index_count = config->index_count;

        VkBuffer staging_buffer;
        VkDeviceMemory staging_buffer_memory;
        lna_vulkan_create_buffer(
            renderer->device,
            renderer->physical_device,
            index_buffer_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &staging_buffer,
            &staging_buffer_memory
            );
        void *indices_data;
        VULKAN_CHECK_RESULT(
            vkMapMemory(
                renderer->device,
                staging_buffer_memory,
                0,
                index_buffer_size,
                0,
                &indices_data
                )
            )
        memcpy(
            indices_data,
            config->indices,
            index_buffer_size
            );
        vkUnmapMemory(
            renderer->device,
            staging_buffer_memory
            );
        lna_vulkan_create_buffer(
            renderer->device,
            renderer->physical_device,
            index_buffer_size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            &primitive->index_buffer,
            &primitive->index_buffer_memory
            );
        lna_vulkan_copy_buffer(
            renderer->device,
            renderer->command_pool,
            renderer->graphics_queue,
            staging_buffer,
            primitive->index_buffer,
            index_buffer_size
            );
        vkDestroyBuffer(
            renderer->device,
            staging_buffer,
            NULL
            );
        vkFreeMemory(
            renderer->device,
            staging_buffer_memory,
            NULL
            );
    }

    //! UNIFORM BUFFER

    lna_primitive_create_uniform_buffer(
        primitive,
        renderer
        );

    //! DESCRIPTOR SETS

    lna_primitive_create_descriptor_sets(
        primitive,
        primitive_system
        );

    return primitive;
}

lna_primitive_t* lna_primitive_system_new_line(lna_primitive_system_t* primitive_system, const lna_primitive_line_config_t* config)
{
    lna_assert(primitive_system)
    lna_assert(!primitive_system->fill_shapes) //! cannot draw arrow in fill shape mode
    lna_assert(config)
    lna_assert(config->pos_a)
    lna_assert(config->pos_b)
    lna_assert(config->col_a)
    lna_assert(config->col_b)
    
    const lna_primitive_vertex_t vertices[] =
    {
        {
            .position = *config->pos_a,
            .color = *config->col_a,
        },
        {
            .position = *config->pos_b,
            .color = *config->col_b,
        },
    };
    const uint32_t indices[] = { 0, 1 };
    return lna_primitive_system_new_raw(
        primitive_system,
        &(lna_primitive_raw_config_t)
        {
            .vertices = vertices,
            .indices = indices,
            .vertex_count = 2,
            .index_count = 2,
            .model_matrix = config->model_matrix,
            .view_matrix = config->view_matrix,
            .projection_matrix = config->projection_matrix,
        }
        );
}

lna_primitive_t* lna_primitive_system_new_rect_xy(lna_primitive_system_t* primitive_system, const lna_primitive_rect_config_t* config)
{
    lna_assert(config)
    lna_assert(config->position)
    lna_assert(config->size)
    lna_assert(config->color)

    const lna_primitive_vertex_t vertices[] =
    {
        {
            .position = { config->position->x, config->position->y, config->position->z },
            .color = *config->color,
        },
        {
            .position = { config->position->x, config->position->y - config->size->height, config->position->z },
            .color = *config->color,
        },
        {
            .position = { config->position->x + config->size->width, config->position->y - config->size->height, config->position->z },
            .color = *config->color,
        },
        {
            .position = { config->position->x + config->size->width, config->position->y, config->position->z },
            .color = *config->color,
        },
    };

    return lna_primitive_system_new_raw(
        primitive_system,
        &(lna_primitive_raw_config_t)
        {
            .vertices = vertices,
            .indices = primitive_system->fill_shapes ? (uint32_t[]){ 0, 1, 2, 2, 3, 0 } : (uint32_t[]){ 0, 1, 1, 2, 2, 3, 3, 0 },
            .vertex_count = 4,
            .index_count = primitive_system->fill_shapes ? 6 : 8,
            .model_matrix = config->model_matrix,
            .view_matrix = config->view_matrix,
            .projection_matrix = config->projection_matrix,
        }
        );
}

#define LNA_PRIMITIVE_CIRCLE_VERTEX_COUNT       32
#define LNA_PRIMITIVE_CIRCLE_INDEX_COUNT        (LNA_PRIMITIVE_CIRCLE_VERTEX_COUNT * 2)
#define LNA_PRIMITIVE_FILL_CIRCLE_VERTEX_COUNT  (LNA_PRIMITIVE_CIRCLE_VERTEX_COUNT + 1)
#define LNA_PRIMITIVE_FILL_CIRCLE_INDEX_COUNT   (LNA_PRIMITIVE_FILL_CIRCLE_VERTEX_COUNT * 3)

lna_primitive_t* lna_primitive_system_new_circle_xy(lna_primitive_system_t* primitive_system, const lna_primitive_circle_config_t* config)
{
    lna_assert(config)
    lna_assert(config->center_position) 
    lna_assert(config->color)

    if (!primitive_system->fill_shapes)
    {
        lna_primitive_vertex_t vertices[LNA_PRIMITIVE_CIRCLE_VERTEX_COUNT];
        for (uint32_t i = 0; i < LNA_PRIMITIVE_CIRCLE_VERTEX_COUNT; ++i)
        {
            float theta = 2.0f * LNA_PI * (float)i / (LNA_PRIMITIVE_CIRCLE_VERTEX_COUNT - 1);
            vertices[i] = (lna_primitive_vertex_t)
            {
                .position =
                {
                    config->center_position->x + config->radius * cosf(theta),
                    config->center_position->y + config->radius * sinf(theta),
                    0.0f
                },
                .color = *config->color,
            };
        }

        uint32_t indices[LNA_PRIMITIVE_CIRCLE_INDEX_COUNT];
        uint32_t index = 0;
        for (uint32_t i = 0; i < LNA_PRIMITIVE_CIRCLE_INDEX_COUNT; i += 2)
        {
            indices[i] = index;
            indices[i+1] = index + 1;
            ++index;
        }
        indices[LNA_PRIMITIVE_CIRCLE_INDEX_COUNT-1] = 0;

        return lna_primitive_system_new_raw(
            primitive_system,
            &(lna_primitive_raw_config_t)
            {
                .vertices = vertices,
                .indices = indices,
                .vertex_count = LNA_PRIMITIVE_CIRCLE_VERTEX_COUNT,
                .index_count = LNA_PRIMITIVE_CIRCLE_INDEX_COUNT,
                .model_matrix = config->model_matrix,
                .view_matrix = config->view_matrix,
                .projection_matrix = config->projection_matrix,
            }
            );
    }
    else
    {
        lna_primitive_vertex_t vertices[LNA_PRIMITIVE_FILL_CIRCLE_VERTEX_COUNT];
        for (uint32_t i = 0; i < LNA_PRIMITIVE_CIRCLE_VERTEX_COUNT; ++i)
        {
            float theta = 2.0f * LNA_PI * (float)i / (LNA_PRIMITIVE_CIRCLE_VERTEX_COUNT - 1);
            vertices[i] = (lna_primitive_vertex_t)
            {
                .position =
                {
                    config->center_position->x + config->radius * cosf(theta),
                    config->center_position->y + config->radius * sinf(theta),
                    0.0f
                },
                .color = *config->color,
            };
        }
        vertices[LNA_PRIMITIVE_CIRCLE_VERTEX_COUNT] = (lna_primitive_vertex_t)
        {
            .position = { config->center_position->x, config->center_position->y , 0.0f },
            .color = *config->color,
        };

        uint32_t indices[LNA_PRIMITIVE_FILL_CIRCLE_INDEX_COUNT];
        uint32_t index = 0;
        for (uint32_t i = 0; i < LNA_PRIMITIVE_FILL_CIRCLE_INDEX_COUNT; i += 3)
        {
            indices[i] = index;
            indices[i+1] = index + 1;
            indices[i+2] = LNA_PRIMITIVE_CIRCLE_VERTEX_COUNT;
            ++index;
        }

        return lna_primitive_system_new_raw(
            primitive_system,
            &(lna_primitive_raw_config_t)
            {
                .vertices = vertices,
                .indices = indices,
                .vertex_count = LNA_PRIMITIVE_FILL_CIRCLE_VERTEX_COUNT,
                .index_count = LNA_PRIMITIVE_FILL_CIRCLE_INDEX_COUNT,
                .model_matrix = config->model_matrix,
                .view_matrix = config->view_matrix,
                .projection_matrix = config->projection_matrix,
            }
            );
    }
}

lna_primitive_t* lna_primitive_system_new_arrow_xy(lna_primitive_system_t* primitive_system, const lna_primitive_arrow_config_t* config)
{
    lna_assert(primitive_system)
    lna_assert(!primitive_system->fill_shapes) //! cannot draw arrow in fill shape mode
    lna_assert(config)
    lna_assert(config->head_position)
    lna_assert(config->tail_position)
    lna_assert(config->color)

    //? ------------------------------------------------------------------------
    //?
    //? SCHEMA OF THE DIFFERENT POINT
    //? 
    //?             e
    //?             |\
    //? tail _______| \ head
    //?            g| /
    //?             |/
    //?             f
    //?
    //? ------------------------------------------------------------------------

    float head_size = config->head_size;
    float head_x    = config->head_position->x;
    float head_y    = config->head_position->y;
    float tail_x    = config->tail_position->x;
    float tail_y    = config->tail_position->y;
    float dx        = head_x - tail_x;
    float dy        = head_y - tail_y;
    float d_length  = sqrtf(dx * dx + dy * dy);
    float ndx       = dx / d_length;
    float ndy       = dy / d_length;
    float gx        = head_x - head_size * ndx;
    float gy        = head_y - head_size * ndy;
    float ex        = head_x - ndx * head_size - ndy * head_size;
    float ey        = head_y - ndy * head_size + ndx * head_size;
    float fx        = head_x - ndx * head_size + ndy * head_size;
    float fy        = head_y - ndy * head_size - ndx * head_size;

    const lna_primitive_vertex_t vertices[] =
    {
        {
            .position = //! tail
            {
                tail_x,
                tail_y,
                0.0f
            },
            .color = *config->color,
        },
        {
            .position = //! head
            {
                head_x,
                head_y,
                0.0f
            },
            .color = *config->color,
        },
        {
            .position = //! g
            {
                gx,
                gy,
                0.0f
            },
            .color = *config->color,
        },
        {
            .position = //! e
            {
                ex,
                ey,
                0.0f
            },
            .color = *config->color,
        },
        {
            .position = //! f
            {
                fx,
                fy,
                0.0f
            },
            .color = *config->color,
        },
    };
    const uint32_t indices[] = { 0, 2, 3, 4, 1, 3, 1, 4 };
    return lna_primitive_system_new_raw(
        primitive_system,
        &(lna_primitive_raw_config_t)
        {
            .vertices = vertices,
            .indices = indices,
            .vertex_count = 5,
            .index_count = 8,
            .model_matrix = config->model_matrix,
            .view_matrix = config->view_matrix,
            .projection_matrix = config->projection_matrix,
        }
        );
}

lna_primitive_t* lna_primitive_system_new_cross_xy(lna_primitive_system_t* primitive_system, const lna_primitive_cross_config_t* config)
{
    lna_assert(primitive_system)
    lna_assert(!primitive_system->fill_shapes) //! cannot draw arrow in fill shape mode
    lna_assert(config)
    lna_assert(config->center_position)
    lna_assert(config->size)
    lna_assert(config->color)

    const lna_primitive_vertex_t vertices[] =
    {
        {
            .position =
            {
                config->center_position->x,
                config->center_position->y + config->size->height * 0.5f,
                0.0f
            },
            .color = *config->color,
        },
        {
            .position =
            {
                config->center_position->x,
                config->center_position->y - config->size->height * 0.5f,
                0.0f
            },
            .color = *config->color,
        },
        {
            .position =
            {
                config->center_position->x + config->size->width * 0.5f,
                config->center_position->y,
                0.0f
            },
            .color = *config->color,
        },
        {
            .position =
            {
                config->center_position->x - config->size->width * 0.5f,
                config->center_position->y,
                0.0f
            },
            .color = *config->color,
        },
    };
    const uint32_t indices[] = { 0, 1, 2, 3 };
    return lna_primitive_system_new_raw(
        primitive_system,
        &(lna_primitive_raw_config_t)
        {
            .vertices = vertices,
            .indices = indices,
            .vertex_count = 4,
            .index_count = 4,
            .model_matrix = config->model_matrix,
            .view_matrix = config->view_matrix,
            .projection_matrix = config->projection_matrix,
        }
        );
}
