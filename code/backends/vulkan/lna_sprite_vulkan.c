#include <string.h>
#include "graphics/lna_sprite.h"
#include "backends/vulkan/lna_sprite_vulkan.h"
#include "backends/vulkan/lna_texture_vulkan.h"
#include "backends/vulkan/lna_vulkan.h"
#include "core/lna_assert.h"
#include "core/lna_memory_pool.h"
#include "core/lna_file.h"
#include "maths/lna_vec2.h"
#include "maths/lna_vec3.h"
#include "maths/lna_vec4.h"
#include "maths/lna_mat4.h"

typedef struct lna_sprite_vertex_s
{
    lna_vec3_t  position;
    lna_vec2_t  uv;
    lna_vec4_t  color;
} lna_sprite_vertex_t;

typedef struct lna_sprite_uniform_s
{
    lna_mat4_t  model;
    lna_mat4_t  view;
    lna_mat4_t  projection;
} lna_sprite_uniform_t;

static void lna_sprite_system_create_graphics_pipeline(
    lna_sprite_system_t* sprite_system,
    lna_renderer_t* renderer
    )
{
    lna_assert(sprite_system)
    lna_assert(renderer)

    // TODO: avoid direct file load: add binary code directly in code as a static const uint32_t* 
    lna_binary_file_content_uint32_t vertex_shader_file = { 0 };
    lna_binary_file_debug_load_uint32(
        &vertex_shader_file,
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_FRAME],
        "shaders/sprite_vert.spv"
        );
    // TODO: avoid direct file load: add binary code directly in code as a static const uint32_t* 
    lna_binary_file_content_uint32_t fragment_shader_file = { 0 };
    lna_binary_file_debug_load_uint32(
        &fragment_shader_file,
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_FRAME],
        "shaders/sprite_frag.spv"
        );
    
    VkShaderModule vertex_shader_module = lna_vulkan_create_shader_module(
        renderer->device,
        vertex_shader_file.content,
        vertex_shader_file.size
        );
    VkShaderModule fragment_shader_module = lna_vulkan_create_shader_module(
        renderer->device,
        fragment_shader_file.content,
        fragment_shader_file.size
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
            .binding    = 0,
            .location   = 0,
            .format     = VK_FORMAT_R32G32B32_SFLOAT,
            .offset     = offsetof(lna_sprite_vertex_t, position),
        },
        {
            .binding    = 0,
            .location   = 1,
            .format     = VK_FORMAT_R32G32_SFLOAT,
            .offset     = offsetof(lna_sprite_vertex_t, uv),
        },
        {
            .binding    = 0,
            .location   = 2,
            .format     = VK_FORMAT_R32G32B32A32_SFLOAT,
            .offset     = offsetof(lna_sprite_vertex_t, color),
        },
    };
    const VkVertexInputBindingDescription vertex_input_binding_description[] =
    {
        {
            .binding    = 0,
            .stride     = sizeof(lna_sprite_vertex_t),
            .inputRate  = VK_VERTEX_INPUT_RATE_VERTEX,
        },
    };
    const VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info =
    {
        .sType                              = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount      = (uint32_t)(sizeof(vertex_input_binding_description) / sizeof(vertex_input_binding_description[0])),
        .pVertexBindingDescriptions         = vertex_input_binding_description,
        .vertexAttributeDescriptionCount    = (uint32_t)(sizeof(vertex_input_attribute_descriptions) / sizeof(vertex_input_attribute_descriptions[0])),
        .pVertexAttributeDescriptions       = vertex_input_attribute_descriptions,
    };
    const VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info =
    {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };
    const VkViewport viewport =
    {
        .x          = 0.0f,
        .y          = 0.0f,
        .width      = (float)(renderer->swap_chain_extent.width),
        .height     = (float)(renderer->swap_chain_extent.height),
        .minDepth   = 0.0f,
        .maxDepth   = 1.0f,
    };
    const VkRect2D scissor =
    {
        .offset = {0, 0},
        .extent = renderer->swap_chain_extent,
    };
    const VkPipelineViewportStateCreateInfo viewport_state_create_info =
    {
        .sType          = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount  = 1,
        .pViewports     = &viewport,
        .scissorCount   = 1,
        .pScissors      = &scissor,
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
    const VkPipelineMultisampleStateCreateInfo multisample_state_create_info =
    {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable    = VK_FALSE,
        .rasterizationSamples   = VK_SAMPLE_COUNT_1_BIT,
        .minSampleShading       = 1.0f,
        .pSampleMask            = NULL,
        .alphaToCoverageEnable  = VK_FALSE,
        .alphaToOneEnable       = VK_FALSE,
    };
    const VkPipelineColorBlendAttachmentState color_blend_attachment_state =
    {
        .colorWriteMask         = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable            = VK_FALSE,
        .srcColorBlendFactor    = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor    = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp           = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor    = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor    = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp           = VK_BLEND_OP_ADD,
    };
    const VkPipelineColorBlendStateCreateInfo color_blender_state_create_info =
    {
        .sType              = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable      = VK_FALSE,
        .logicOp            = VK_LOGIC_OP_COPY,
        .attachmentCount    = 1,
        .pAttachments       = &color_blend_attachment_state,
        .blendConstants[0]  = 0.0f,
        .blendConstants[1]  = 0.0f,
        .blendConstants[2]  = 0.0f,
        .blendConstants[3]  = 0.0f,
    };
    const VkDynamicState dynamic_states[] =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_LINE_WIDTH
    };
    const VkPipelineDynamicStateCreateInfo dynamic_state_create_info =
    {
        .sType              = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount  = 2,
        .pDynamicStates     = dynamic_states,
    };
    const VkPipelineLayoutCreateInfo pipeline_layout_create_info =
    {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount         = 1,
        .pSetLayouts            = &sprite_system->descriptor_set_layout,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges    = NULL,
    };
    lna_vulkan_check(
        vkCreatePipelineLayout(
            renderer->device,
            &pipeline_layout_create_info,
            NULL,
            &sprite_system->pipeline_layout
            )
        );
    const VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info =
    {
        .sType              = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable    = VK_FALSE,
        .depthWriteEnable   = VK_FALSE,
        .depthCompareOp     = VK_COMPARE_OP_LESS_OR_EQUAL,
        .back.compareOp     = VK_COMPARE_OP_ALWAYS,
    };
    const VkGraphicsPipelineCreateInfo graphics_pipeline_create_info =
    {
        .sType                  = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount             = 2,
        .pStages                = shader_stage_create_infos,
        .pVertexInputState      = &vertex_input_state_create_info,
        .pInputAssemblyState    = &input_assembly_state_create_info,
        .pViewportState         = &viewport_state_create_info,
        .pRasterizationState    = &rasterization_state_create_info,
        .pMultisampleState      = &multisample_state_create_info,
        .pDepthStencilState     = &depth_stencil_state_create_info,
        .pColorBlendState       = &color_blender_state_create_info,
        .pDynamicState          = &dynamic_state_create_info,
        .layout                 = sprite_system->pipeline_layout,
        .renderPass             = renderer->render_pass,
        .subpass                = 0,
        .basePipelineHandle     = VK_NULL_HANDLE,
        .basePipelineIndex      = -1,
    };
    lna_vulkan_check(
        vkCreateGraphicsPipelines(
            renderer->device,
            VK_NULL_HANDLE,
            1,
            &graphics_pipeline_create_info,
            NULL,
            &sprite_system->pipeline
            )
        );

    vkDestroyShaderModule(
        renderer->device,
        fragment_shader_module,
        NULL
        );
    vkDestroyShaderModule(
        renderer->device,
        vertex_shader_module,
        NULL
        );
}

static void lna_sprite_system_create_descriptor_pool(
    lna_sprite_system_t* sprite_system
    )
{
    lna_assert(sprite_system)
    lna_assert(sprite_system->sprites.max_element_count > 0)

    lna_renderer_t* renderer = sprite_system->renderer;
    lna_assert(renderer)
    lna_assert(renderer->swap_chain_images.count > 0)

    const VkDescriptorPoolSize pool_sizes[] =
    {
        {
            .type               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount    = renderer->swap_chain_images.count * sprite_system->sprites.max_element_count,
        },
        {
            .type               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount    = renderer->swap_chain_images.count * sprite_system->sprites.max_element_count,
        },
    };

    const VkDescriptorPoolCreateInfo pool_create_info =
    {
        .sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount  = (uint32_t)(sizeof(pool_sizes) / sizeof(pool_sizes[0])),
        .pPoolSizes     = pool_sizes,
        .maxSets        = renderer->swap_chain_images.count * sprite_system->sprites.max_element_count,
    };

    lna_vulkan_check(
        vkCreateDescriptorPool(
            renderer->device,
            &pool_create_info,
            NULL,
            &sprite_system->descriptor_pool
            )
        );

    lna_log_message("----------------------------");
    lna_log_message("sprite descriptor pool info:");
    lna_log_message("----------------------------");
    lna_log_message("\tdescriptor pool address: %p", (void*)sprite_system->descriptor_pool);
    lna_log_message("\tmax sets               : %d", pool_create_info.maxSets);
    lna_log_message("\tpool size count        : %d", pool_create_info.poolSizeCount);
    for (uint32_t i = 0; i < pool_create_info.poolSizeCount; ++i)
    {
        lna_log_message("\tpool %d descriptor count: %d", i, pool_sizes[i].descriptorCount);
    }
}

static void lna_sprite_create_uniform_buffer(
    lna_sprite_t* sprite,
    lna_renderer_t* renderer
    )
{
    lna_assert(sprite)
    lna_assert(sprite->mvp_uniform_buffers.count == 0)
    lna_assert(sprite->mvp_uniform_buffers.elements == NULL)
    lna_assert(sprite->mvp_uniform_buffers_memory.count == 0)
    lna_assert(sprite->mvp_uniform_buffers_memory.elements == 0)

    sprite->mvp_uniform_buffers.count       = renderer->swap_chain_images.count;
    sprite->mvp_uniform_buffers.elements    = lna_memory_pool_reserve(
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_SWAP_CHAIN],
        sizeof(VkBuffer) * renderer->swap_chain_images.count
        );

    sprite->mvp_uniform_buffers_memory.count    = renderer->swap_chain_images.count;
    sprite->mvp_uniform_buffers_memory.elements = lna_memory_pool_reserve(
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_SWAP_CHAIN],
        sizeof(VkDeviceMemory) * renderer->swap_chain_images.count
        );

    VkDeviceSize uniform_buffer_size = sizeof(lna_sprite_uniform_t);
    for (size_t i = 0; i < sprite->mvp_uniform_buffers.count; ++i)
    {
        lna_vulkan_create_buffer(
            renderer->device,
            renderer->physical_device,
            uniform_buffer_size,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &sprite->mvp_uniform_buffers.elements[i],
            &sprite->mvp_uniform_buffers_memory.elements[i]
            );
    }
}

static void lna_sprite_create_descriptor_sets(
    lna_sprite_t* sprite,
    lna_sprite_system_t* sprite_system
    )
{
    lna_assert(sprite)
    lna_assert(sprite_system)
    lna_assert(sprite->descriptor_sets.count == 0)
    lna_assert(sprite->descriptor_sets.elements == NULL)

    const lna_texture_t* texture = sprite->texture;
    lna_assert(texture)
    lna_assert(texture->image_view)
    lna_assert(texture->image_sampler)

    lna_renderer_t* renderer = sprite_system->renderer;
    lna_assert(renderer)
    lna_assert(renderer->swap_chain_images.count > 0)

    lna_vulkan_descriptor_set_layout_array_t layouts = { 0 };
    layouts.count       = renderer->swap_chain_images.count;
    layouts.elements    = lna_memory_pool_reserve(
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_FRAME],
        sizeof(VkDescriptorSetLayout) * renderer->swap_chain_images.count
        );

    for (uint32_t i = 0; i < layouts.count; ++i)
    {
        layouts.elements[i] = sprite_system->descriptor_set_layout;
    }

    const VkDescriptorSetAllocateInfo allocate_info =
    {
        .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool     = sprite_system->descriptor_pool,
        .descriptorSetCount = layouts.count,
        .pSetLayouts        = layouts.elements,
    };

    sprite->descriptor_sets.count       = renderer->swap_chain_images.count;
    sprite->descriptor_sets.elements    = lna_memory_pool_reserve(
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_SWAP_CHAIN],
        sizeof(VkDescriptorSet) * renderer->swap_chain_images.count
        );

    lna_vulkan_check(
        vkAllocateDescriptorSets(
            renderer->device,
            &allocate_info,
            sprite->descriptor_sets.elements
            )
        );

    for (size_t i = 0; i < sprite->descriptor_sets.count; ++i)
    {
        const VkDescriptorBufferInfo buffer_info =
        {
            .buffer = sprite->mvp_uniform_buffers.elements[i],
            .offset = 0,
            .range  = sizeof(lna_sprite_uniform_t),
        };
        const VkDescriptorImageInfo image_info =
        {
            .imageLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .imageView      = texture->image_view,
            .sampler        = texture->image_sampler,
        };
        const VkWriteDescriptorSet write_descriptors[] =
        {
            {
                .sType              = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet             = sprite->descriptor_sets.elements[i],
                .dstBinding         = 0,
                .dstArrayElement    = 0,
                .descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount    = 1,
                .pBufferInfo        = &buffer_info,
                .pImageInfo         = NULL,
                .pTexelBufferView   = NULL,
            },
            {
                .sType              = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet             = sprite->descriptor_sets.elements[i],
                .dstBinding         = 1,
                .dstArrayElement    = 0,
                .descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount    = 1,
                .pBufferInfo        = NULL,
                .pImageInfo         = &image_info,
                .pTexelBufferView   = NULL,
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

static void lna_sprite_system_on_swap_chain_cleanup(void* owner)
{
    lna_assert(owner)
    lna_sprite_system_t*    sprite_system   = (lna_sprite_system_t*)owner;
    lna_renderer_t*         renderer        = sprite_system->renderer;
    lna_assert(renderer)

    vkDestroyPipeline(
        renderer->device,
        sprite_system->pipeline,
        NULL
        );
    vkDestroyPipelineLayout(
        renderer->device,
        sprite_system->pipeline_layout,
        NULL
        );

    for (uint32_t i = 0; i < sprite_system->sprites.cur_element_count; ++i)
    {
        lna_sprite_t* sprite = &sprite_system->sprites.elements[i];
        lna_assert(sprite->mvp_uniform_buffers.elements)
        lna_assert(sprite->mvp_uniform_buffers_memory.elements)

        for (size_t j = 0; j < sprite->mvp_uniform_buffers.count; ++j)
        {
            vkDestroyBuffer(
                renderer->device,
                sprite->mvp_uniform_buffers.elements[j],
                NULL
                );
        }
        for (size_t j = 0; j < sprite->mvp_uniform_buffers.count; ++j)
        {
            vkFreeMemory(
                renderer->device,
                sprite->mvp_uniform_buffers_memory.elements[j],
                NULL
                );
        }
        sprite->mvp_uniform_buffers.count           = 0;
        sprite->mvp_uniform_buffers.elements        = NULL;
        sprite->mvp_uniform_buffers_memory.count    = 0;
        sprite->mvp_uniform_buffers_memory.elements = NULL;
        //! NOTE: no need to explicity clean up vulkan descriptor sets objects
        //! because it is done when the vulkan descriptor pool is destroyed.
        //! we just have to reset the array and wait for filling it again.
        //! the memory pool where we reserved memory for descriptor_sets has already
        //! been reset by the vulkan renderer backend.
        sprite->descriptor_sets.count       = 0;
        sprite->descriptor_sets.elements    = 0;
    }
    vkDestroyDescriptorPool(
        renderer->device,
        sprite_system->descriptor_pool,
        NULL
        );
}

static void lna_sprite_system_on_swap_chain_recreate(void *owner)
{
    lna_assert(owner)
    lna_sprite_system_t* sprite_system = (lna_sprite_system_t*)owner;
    lna_assert(sprite_system->sprites.elements)
    lna_renderer_t* renderer = sprite_system->renderer;
    lna_assert(renderer)

    lna_sprite_system_create_graphics_pipeline(
        sprite_system,
        renderer
        );
    lna_sprite_system_create_descriptor_pool(
        sprite_system
        );

    for (uint32_t i = 0; i < sprite_system->sprites.cur_element_count; ++i)
    {
        lna_sprite_create_uniform_buffer(
            &sprite_system->sprites.elements[i],
            renderer
            );
        lna_sprite_create_descriptor_sets(
            &sprite_system->sprites.elements[i],
            sprite_system
            );
    }
}

void lna_sprite_system_init(lna_sprite_system_t* sprite_system, const lna_sprite_system_config_t* config)
{
    lna_assert(sprite_system)
    lna_assert(sprite_system->renderer == NULL)
    lna_assert(sprite_system->sprites.cur_element_count == 0)
    lna_assert(sprite_system->sprites.max_element_count == 0)
    lna_assert(sprite_system->sprites.elements == NULL)
    lna_assert(sprite_system->descriptor_pool == VK_NULL_HANDLE)
    lna_assert(sprite_system->descriptor_set_layout == VK_NULL_HANDLE)
    lna_assert(sprite_system->pipeline == VK_NULL_HANDLE)
    lna_assert(sprite_system->pipeline_layout == VK_NULL_HANDLE)
    lna_assert(config)
    lna_assert(config->renderer)
    lna_assert(config->renderer->device)
    lna_assert(config->renderer->render_pass)

    sprite_system->renderer = config->renderer;

    lna_renderer_register_listener(
        config->renderer,
        lna_sprite_system_on_swap_chain_cleanup,
        lna_sprite_system_on_swap_chain_recreate,
        (void*)sprite_system
        );

    sprite_system->sprites.max_element_count    = config->max_sprite_count;
    sprite_system->sprites.elements             = lna_memory_pool_reserve(
        config->memory_pool,
        sizeof(lna_sprite_t) * config->max_sprite_count
        );

    //! DESCRIPTOR SET LAYOUT

    const VkDescriptorSetLayoutBinding bindings[] =
    {
        {
            .binding            = 0,
            .descriptorCount    = 1,
            .descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .stageFlags         = VK_SHADER_STAGE_VERTEX_BIT,
            .pImmutableSamplers = NULL,
        },
        {
            .binding            = 1,
            .descriptorCount    = 1,
            .descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = NULL,
        },
    };
    const VkDescriptorSetLayoutCreateInfo layout_create_info =
    {
        .sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount   = (uint32_t)(sizeof(bindings) / sizeof(bindings[0])),
        .pBindings      = bindings,
    };
    lna_vulkan_check(
        vkCreateDescriptorSetLayout(
            config->renderer->device,
            &layout_create_info,
            NULL,
            &sprite_system->descriptor_set_layout
            )
        );

    //! GRAPHICS PIPELINE

    lna_sprite_system_create_graphics_pipeline(
        sprite_system,
        config->renderer
        );

    //! DESCRIPTOR POOL

    lna_sprite_system_create_descriptor_pool(
        sprite_system
        );
}

lna_sprite_t* lna_sprite_system_new_sprite(lna_sprite_system_t* sprite_system, const lna_sprite_config_t* config)
{
    lna_assert(sprite_system)
    lna_assert(config)
    lna_assert(sprite_system->sprites.elements)
    lna_assert(sprite_system->sprites.cur_element_count < sprite_system->sprites.max_element_count)

    lna_sprite_t* sprite = &sprite_system->sprites.elements[sprite_system->sprites.cur_element_count++];

    lna_assert(sprite)
    lna_assert(sprite->texture == NULL)
    lna_assert(sprite->vertex_buffer == VK_NULL_HANDLE)
    lna_assert(sprite->vertex_buffer_memory == VK_NULL_HANDLE)
    lna_assert(sprite->index_buffer == VK_NULL_HANDLE)
    lna_assert(sprite->index_buffer_memory == VK_NULL_HANDLE)
    lna_assert(sprite->mvp_uniform_buffers.count == 0)
    lna_assert(sprite->mvp_uniform_buffers.elements == 0)
    lna_assert(sprite->mvp_uniform_buffers_memory.count == 0)
    lna_assert(sprite->mvp_uniform_buffers_memory.elements == NULL)
    lna_assert(sprite->descriptor_sets.count == 0)
    lna_assert(sprite->descriptor_sets.elements == 0)
    lna_assert(sprite->model_matrix == NULL)
    lna_assert(sprite->view_matrix == NULL)
    lna_assert(sprite->projection_matrix == NULL)
    lna_assert(config)
    lna_assert(config->model_matrix)
    lna_assert(config->view_matrix)
    lna_assert(config->projection_matrix)
    lna_assert(sprite_system)

    lna_renderer_t* renderer = sprite_system->renderer;
    lna_assert(renderer)

    sprite->model_matrix        = config->model_matrix;
    sprite->view_matrix         = config->view_matrix;
    sprite->projection_matrix   = config->projection_matrix;
    sprite->texture             = config->texture;

    //! VERTEX BUFFER PART

    {
        const lna_vec3_t default_position           = { 0.0f, 0.0f, 0.0f };
        const lna_vec2_t default_size               = { 1.0f, 1.0f };
        const lna_vec2_t default_uv_offset_position = { 0.0f, 0.0f };
        const lna_vec2_t default_uv_offset_size     = { 1.0f, 1.0f };
        const lna_vec4_t default_color              = { 1.0f, 1.0f, 1.0f, 1.0f };
        const lna_vec3_t* position                  = config->local_position ? config->local_position : &default_position;
        const lna_vec2_t* size                      = config->size ? config->size : &default_size; 
        const lna_vec2_t* uv_offset_position        = config->uv_offset_position ? config->uv_offset_position : &default_uv_offset_position;
        const lna_vec2_t* uv_offset_size            = config->uv_offset_size ? config->uv_offset_size : &default_uv_offset_size;
        const lna_vec4_t* color                     = config->blend_color ? config->blend_color : &default_color;

        lna_sprite_vertex_t vertices[] =
        {
            {
                .position =
                {
                    position->x,
                    position->y,
                    position->z
                },
                .uv =
                {
                    uv_offset_position->x,
                    uv_offset_position->y
                },
                .color =
                {
                    color->r,
                    color->g,
                    color->b,
                    color->a
                },
            },
            {
                .position =
                {
                    position->x,
                    position->y + size->height,
                    position->z
                },
                .uv =
                {
                    uv_offset_position->x,
                    uv_offset_position->y + uv_offset_size->height
                },
                .color =
                {
                    color->r,
                    color->g,
                    color->b,
                    color->a
                },
            },
            {
                .position =
                {
                    position->x + size->width,
                    position->y + size->height,
                    position->z
                },
                .uv =
                {
                    uv_offset_position->x + uv_offset_size->width,
                    uv_offset_position->y + uv_offset_size->height
                },
                .color =
                {
                    color->r,
                    color->g,
                    color->b,
                    color->a
                },
            },
            {
                .position =
                {
                    position->x + size->width,
                    position->y,
                    position->z
                },
                .uv =
                {
                    uv_offset_position->x + uv_offset_size->width,
                    uv_offset_position->y
                },
                .color =
                {
                    color->r,
                    color->g,
                    color->b,
                    color->a
                },
            },
        };

        size_t vertex_buffer_size = sizeof(vertices);
        
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
        lna_vulkan_check(
            vkMapMemory(
                renderer->device,
                staging_buffer_memory,
                0,
                vertex_buffer_size,
                0,
                &vertices_data
                )
            );
        memcpy(
            vertices_data,
            vertices,
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
            &sprite->vertex_buffer,
            &sprite->vertex_buffer_memory
            );
        lna_vulkan_copy_buffer(
            renderer->device,
            renderer->command_pool,
            renderer->graphics_queue,
            staging_buffer,
            sprite->vertex_buffer,
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
        const uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };
        const size_t index_buffer_size = sizeof(indices);

        sprite->index_count = (uint32_t)(sizeof(indices) / sizeof(indices[0]));

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
        lna_vulkan_check(
            vkMapMemory(
                renderer->device,
                staging_buffer_memory,
                0,
                index_buffer_size,
                0,
                &indices_data
                )
            );
        memcpy(
            indices_data,
            indices,
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
            &sprite->index_buffer,
            &sprite->index_buffer_memory
            );
        lna_vulkan_copy_buffer(
            renderer->device,
            renderer->command_pool,
            renderer->graphics_queue,
            staging_buffer,
            sprite->index_buffer,
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

    lna_sprite_create_uniform_buffer(
        sprite,
        renderer
        );

    //! DESCRIPTOR SETS

    lna_sprite_create_descriptor_sets(
        sprite,
        sprite_system
        );

    return sprite;
}

void lna_sprite_system_draw(lna_sprite_system_t* sprite_system)
{
    lna_assert(sprite_system)

    lna_renderer_t* renderer = sprite_system->renderer;
    lna_assert(renderer)
    lna_assert(renderer->command_buffers.elements)
    lna_assert(renderer->command_buffers.count > renderer->image_index)

    VkCommandBuffer command_buffer = renderer->command_buffers.elements[renderer->image_index];

    vkCmdBindPipeline(
        command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        sprite_system->pipeline
        );
    for (uint32_t i = 0; i < sprite_system->sprites.cur_element_count; ++i)
    {
        lna_sprite_t* sprite = &sprite_system->sprites.elements[i];
        lna_assert(sprite->model_matrix)
        lna_assert(sprite->view_matrix)
        lna_assert(sprite->projection_matrix)
        lna_assert(sprite->mvp_uniform_buffers_memory.count > renderer->image_index)
        lna_assert(sprite->descriptor_sets.count > renderer->image_index)

        const lna_sprite_uniform_t ubo =
        {
            .model      = *sprite->model_matrix,
            .view       = *sprite->view_matrix,
            .projection = *sprite->projection_matrix,
        };
        void *data;
        lna_vulkan_check(
            vkMapMemory(
                renderer->device,
                sprite->mvp_uniform_buffers_memory.elements[renderer->image_index],
                0,
                sizeof(ubo),
                0,
                &data
                )
            );
        memcpy(
            data,
            &ubo,
            sizeof(ubo));
        vkUnmapMemory(
            renderer->device,
            sprite->mvp_uniform_buffers_memory.elements[renderer->image_index]
            );

        vkCmdBindDescriptorSets(
            command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            sprite_system->pipeline_layout,
            0,
            1,
            &sprite->descriptor_sets.elements[renderer->image_index],
            0,
            NULL
            );
        const VkBuffer vertex_buffers[] =
        {
            sprite->vertex_buffer
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
            sprite->index_buffer,
            0,
            VK_INDEX_TYPE_UINT32
            );
        vkCmdDrawIndexed(
            command_buffer,
            sprite->index_count,
            1,
            0,
            0,
            0
            );
    }
}

void lna_sprite_system_release(lna_sprite_system_t* sprite_system)
{
    lna_assert(sprite_system)
    lna_assert(sprite_system->sprites.elements)
    lna_assert(sprite_system->renderer)
    lna_assert(sprite_system->renderer->device)

    vkDestroyDescriptorSetLayout(
        sprite_system->renderer->device,
        sprite_system->descriptor_set_layout,
        NULL
        );
    for (uint32_t i = 0; i < sprite_system->sprites.cur_element_count; ++i)
    {
        lna_sprite_t* sprite = &sprite_system->sprites.elements[i];

        vkDestroyBuffer(
            sprite_system->renderer->device,
            sprite->index_buffer,
            NULL
            );
        vkFreeMemory(
            sprite_system->renderer->device,
            sprite->index_buffer_memory,
            NULL
            );
        vkDestroyBuffer(
            sprite_system->renderer->device,
            sprite->vertex_buffer,
            NULL
            );
        vkFreeMemory(
            sprite_system->renderer->device,
            sprite->vertex_buffer_memory,
            NULL
            );
    }
}
