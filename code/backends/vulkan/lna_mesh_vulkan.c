#include <string.h>
#include "graphics/lna_mesh.h"
#include "backends/vulkan/lna_mesh_vulkan.h"
#include "backends/vulkan/lna_texture_vulkan.h"
#include "backends/vulkan/lna_vulkan.h"
#include "graphics/lna_material.h"
#include "core/lna_assert.h"
#include "core/lna_memory_pool.h"
#include "core/lna_file.h"
#include "maths/lna_vec2.h"
#include "maths/lna_vec3.h"
#include "maths/lna_vec4.h"
#include "maths/lna_mat4.h"

typedef struct lna_mesh_mvp_uniform_s
{
    lna_mat4_t  model;
    lna_mat4_t  view;
    lna_mat4_t  projection;
} lna_mesh_mvp_uniform_t;

typedef struct lna_mesh_light_uniform_s
{
    lna_vec4_t  light_position;
    lna_vec4_t  view_position;
    lna_vec4_t  light_color;
} lna_mesh_light_uniform_t;

static void lna_mesh_system_create_graphics_pipeline(
    lna_mesh_system_t* mesh_system,
    lna_renderer_t* renderer
    )
{
    lna_assert(mesh_system)
    lna_assert(renderer)

    // TODO: avoid direct file load --------------------------------------------
    lna_binary_file_content_uint32_t vertex_shader_file = { 0 };
    lna_binary_file_debug_load_uint32(
        &vertex_shader_file,
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_FRAME],
        "shaders/default_vert.spv"
        );
    lna_binary_file_content_uint32_t fragment_shader_file = { 0 };
    lna_binary_file_debug_load_uint32(
        &fragment_shader_file,
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_FRAME],
        "shaders/default_frag.spv"
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
            .offset = offsetof(lna_mesh_vertex_t, position),
        },
        {
            .binding = 0,
            .location = 1,
            .format = VK_FORMAT_R32G32B32A32_SFLOAT,
            .offset = offsetof(lna_mesh_vertex_t, color),
        },
        {
            .binding = 0,
            .location = 2,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = offsetof(lna_mesh_vertex_t, uv),
        },
        {
            .binding = 0,
            .location = 3,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(lna_mesh_vertex_t, normal),
        },
    };
    const VkVertexInputBindingDescription vertex_input_binding_description[] =
    {
        {
            .binding = 0,
            .stride = sizeof(lna_mesh_vertex_t),
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
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
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
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_BACK_BIT,
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
        .pSetLayouts = &mesh_system->descriptor_set_layout,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = NULL,
    };
    VULKAN_CHECK_RESULT(
        vkCreatePipelineLayout(
            renderer->device,
            &pipeline_layout_create_info,
            NULL,
            &mesh_system->pipeline_layout
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
        .layout = mesh_system->pipeline_layout,
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
            &mesh_system->pipeline
            )
        )

    vkDestroyShaderModule(renderer->device, fragment_shader_module, NULL);
    vkDestroyShaderModule(renderer->device, vertex_shader_module, NULL);
}

static void lna_mesh_system_create_descriptor_pool(
    lna_mesh_system_t* mesh_system,
    lna_renderer_t* renderer
    )
{
    lna_assert(mesh_system)
    lna_assert(renderer)

    const VkDescriptorPoolSize pool_sizes[] =
    {
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = lna_array_size(&renderer->swap_chain_images) * lna_vector_max_capacity(&mesh_system->meshes),
        },
        {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = lna_array_size(&renderer->swap_chain_images) * lna_vector_max_capacity(&mesh_system->meshes),
        },
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = lna_array_size(&renderer->swap_chain_images) * lna_vector_max_capacity(&mesh_system->meshes),
        },
    };

    const VkDescriptorPoolCreateInfo pool_create_info =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = (uint32_t)(sizeof(pool_sizes) / sizeof(pool_sizes[0])),
        .pPoolSizes = pool_sizes,
        .maxSets = lna_array_size(&renderer->swap_chain_images) * lna_vector_max_capacity(&mesh_system->meshes),
    };

    VULKAN_CHECK_RESULT(
        vkCreateDescriptorPool(
            renderer->device,
            &pool_create_info,
            NULL,
            &mesh_system->descriptor_pool
            )
        )

    lna_log_message("--------------------------");
    lna_log_message("mesh descriptor pool info:");
    lna_log_message("--------------------------");
    lna_log_message("\tdescriptor pool address: %p", (void*)mesh_system->descriptor_pool);
    lna_log_message("\tmax sets               : %d", pool_create_info.maxSets);
    lna_log_message("\tpool size count        : %d", pool_create_info.poolSizeCount);
    for (uint32_t i = 0; i < pool_create_info.poolSizeCount; ++i)
    {
        lna_log_message("\tpool %d descriptor count: %d", i, pool_sizes[i].descriptorCount);
    }
}

static void lna_mesh_create_uniform_buffer(
    lna_mesh_t* mesh,
    lna_renderer_t* renderer
    )
{
    lna_assert(mesh)
    lna_assert(lna_array_size(&mesh->mvp_uniform_buffers) == 0)
    lna_assert(lna_array_size(&mesh->mvp_uniform_buffers_memory) == 0)
    lna_assert(lna_array_size(&mesh->light_uniform_buffers) == 0)
    lna_assert(lna_array_size(&mesh->light_uniform_buffers_memory) == 0)

    lna_array_init(
        &mesh->mvp_uniform_buffers,
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_SWAP_CHAIN],
        VkBuffer,
        lna_array_size(&renderer->swap_chain_images)
        );

    lna_array_init(
        &mesh->mvp_uniform_buffers_memory,
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_SWAP_CHAIN],
        VkDeviceMemory,
        lna_array_size(&renderer->swap_chain_images)
        );

    VkDeviceSize mvp_uniform_buffer_size = sizeof(lna_mesh_mvp_uniform_t);
    for (size_t i = 0; i < lna_array_size(&mesh->mvp_uniform_buffers); ++i)
    {
        lna_vulkan_create_buffer(
            renderer->device,
            renderer->physical_device,
            mvp_uniform_buffer_size,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            lna_array_at_ptr(&mesh->mvp_uniform_buffers, i),
            lna_array_at_ptr(&mesh->mvp_uniform_buffers_memory, i)
            );
    }

    lna_array_init(
        &mesh->light_uniform_buffers,
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_SWAP_CHAIN],
        VkBuffer,
        lna_array_size(&renderer->swap_chain_images)
        );

    lna_array_init(
        &mesh->light_uniform_buffers_memory,
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_SWAP_CHAIN],
        VkDeviceMemory,
        lna_array_size(&renderer->swap_chain_images)
        );

    VkDeviceSize light_uniform_buffer_size = sizeof(lna_mesh_light_uniform_t);
    for (size_t i = 0; i < lna_array_size(&mesh->light_uniform_buffers); ++i)
    {
        lna_vulkan_create_buffer(
            renderer->device,
            renderer->physical_device,
            light_uniform_buffer_size,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            lna_array_at_ptr(&mesh->light_uniform_buffers, i),
            lna_array_at_ptr(&mesh->light_uniform_buffers_memory, i)
            );
    }
}

static void lna_mesh_create_descriptor_sets(
    lna_mesh_t* mesh,
    lna_mesh_system_t* mesh_system
    )
{
    lna_assert(mesh)
    lna_assert(mesh_system)
    lna_assert(lna_array_size(&mesh->descriptor_sets) == 0)

    const lna_material_t* material = mesh->material;
    lna_assert(material)

    const lna_texture_t* texture = material->texture;
    lna_assert(texture)
    lna_assert(texture->image_view)
    lna_assert(texture->image_sampler)

    lna_renderer_t* renderer = mesh_system->renderer;
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
        lna_array_at_ref(&layouts, i) = mesh_system->descriptor_set_layout;
    }

    const VkDescriptorSetAllocateInfo allocate_info =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = mesh_system->descriptor_pool,
        .descriptorSetCount = lna_array_size(&layouts),
        .pSetLayouts = lna_array_ptr(&layouts),
    };

    lna_array_init(
        &mesh->descriptor_sets,
        &renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_SWAP_CHAIN],
        VkDescriptorSet,
        lna_array_size(&renderer->swap_chain_images)
        );

    VULKAN_CHECK_RESULT(
        vkAllocateDescriptorSets(
            renderer->device,
            &allocate_info,
            lna_array_ptr(&mesh->descriptor_sets)
            )
        )

    for (size_t i = 0; i < lna_array_size(&mesh->descriptor_sets); ++i)
    {
        const VkDescriptorBufferInfo mvp_buffer_info =
        {
            .buffer = lna_array_at_ref(&mesh->mvp_uniform_buffers, i),
            .offset = 0,
            .range = sizeof(lna_mesh_mvp_uniform_t),
        };
        const VkDescriptorImageInfo image_info =
        {
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .imageView = texture->image_view,
            .sampler = texture->image_sampler,
        };
        const VkDescriptorBufferInfo light_buffer_info =
        {
            .buffer = lna_array_at_ref(&mesh->light_uniform_buffers, i),
            .offset = 0,
            .range = sizeof(lna_mesh_light_uniform_t),
        };

        const VkWriteDescriptorSet write_descriptors[] =
        {
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = lna_array_at_ref(&mesh->descriptor_sets, i),
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .pBufferInfo = &mvp_buffer_info,
                .pImageInfo = NULL,
                .pTexelBufferView = NULL,
            },
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = lna_array_at_ref(&mesh->descriptor_sets, i),
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = 1,
                .pBufferInfo = NULL,
                .pImageInfo = &image_info,
                .pTexelBufferView = NULL,
            },
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = lna_array_at_ref(&mesh->descriptor_sets, i),
                .dstBinding = 2,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .pBufferInfo = &light_buffer_info,
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

static void lna_mesh_system_on_swap_chain_cleanup(void* owner)
{
    lna_assert(owner)
    lna_mesh_system_t* mesh_system = (lna_mesh_system_t*)owner;
    lna_renderer_t* renderer = mesh_system->renderer;
    lna_assert(renderer)

    vkDestroyPipeline(
        renderer->device,
        mesh_system->pipeline,
        NULL
        );
    vkDestroyPipelineLayout(
        renderer->device,
        mesh_system->pipeline_layout,
        NULL
        );

    for (uint32_t i = 0; i < lna_vector_size(&mesh_system->meshes); ++i)
    {
        lna_mesh_t* mesh = lna_vector_at_ptr(&mesh_system->meshes, i);

        for (size_t j = 0; j < lna_array_size(&mesh->mvp_uniform_buffers); ++j)
        {
            vkDestroyBuffer(
                renderer->device,
                lna_array_at_ref(&mesh->mvp_uniform_buffers, j),
                NULL
                );
        }
        for (size_t j = 0; j < lna_array_size(&mesh->mvp_uniform_buffers); ++j)
        {
            vkFreeMemory(
                renderer->device,
                lna_array_at_ref(&mesh->mvp_uniform_buffers_memory, j),
                NULL
                );
        }
        lna_array_release(&mesh->mvp_uniform_buffers);
        lna_array_release(&mesh->mvp_uniform_buffers_memory);

        for (size_t j = 0; j < lna_array_size(&mesh->light_uniform_buffers); ++j)
        {
            vkDestroyBuffer(
                renderer->device,
                lna_array_at_ref(&mesh->light_uniform_buffers, j),
                NULL
                );
        }
        for (size_t j = 0; j < lna_array_size(&mesh->light_uniform_buffers); ++j)
        {
            vkFreeMemory(
                renderer->device,
                lna_array_at_ref(&mesh->light_uniform_buffers_memory, j),
                NULL
                );
        }
        lna_array_release(&mesh->light_uniform_buffers);
        lna_array_release(&mesh->light_uniform_buffers_memory);

        //! NOTE: no need to explicity clean up vulkan descriptor sets objects
        //! because it is done when the vulkan descriptor pool is destroyed.
        //! we just have to reset the array and wait for filling it again.
        //! the memory pool where we reserved memory for descriptor_sets has already
        //! been reset by the vulkan renderer backend.
        lna_array_release(&mesh->descriptor_sets);
    }
    vkDestroyDescriptorPool(
        renderer->device,
        mesh_system->descriptor_pool,
        NULL
        );
}

static void lna_mesh_system_on_swap_chain_recreate(void *owner)
{
    lna_assert(owner)
    lna_mesh_system_t* mesh_system = (lna_mesh_system_t*)owner;
    lna_renderer_t* renderer = mesh_system->renderer;
    lna_assert(renderer)

    lna_mesh_system_create_graphics_pipeline(
        mesh_system,
        renderer
        );
    lna_mesh_system_create_descriptor_pool(
        mesh_system,
        renderer
        );

    for (uint32_t i = 0; i < lna_vector_size(&mesh_system->meshes); ++i)
    {
        lna_mesh_create_uniform_buffer(
            lna_vector_at_ptr(&mesh_system->meshes, i),
            renderer
            );
        lna_mesh_create_descriptor_sets(
            lna_vector_at_ptr(&mesh_system->meshes, i),
            mesh_system
            );
    }
}

void lna_mesh_system_init(lna_mesh_system_t* mesh_system, const lna_mesh_system_config_t* config)
{
    lna_assert(mesh_system)
    lna_assert(mesh_system->renderer == NULL)
    lna_assert(lna_vector_max_capacity(&mesh_system->meshes) == 0)
    lna_assert(mesh_system->descriptor_pool == VK_NULL_HANDLE)
    lna_assert(mesh_system->descriptor_set_layout == VK_NULL_HANDLE)
    lna_assert(mesh_system->pipeline == VK_NULL_HANDLE)
    lna_assert(mesh_system->pipeline_layout == VK_NULL_HANDLE)
    lna_assert(config)
    lna_assert(config->renderer)
    lna_assert(config->renderer->device)
    lna_assert(config->renderer->render_pass)

    mesh_system->renderer = config->renderer;

    lna_renderer_listener_t* listener = NULL;
    lna_vector_new_element(
        &config->renderer->listeners,
        listener
        );
    listener->handle        = (void*)mesh_system;
    listener->on_cleanup    = lna_mesh_system_on_swap_chain_cleanup;
    listener->on_recreate   = lna_mesh_system_on_swap_chain_recreate;

    lna_vector_init(
        &mesh_system->meshes,
        config->memory_pool,
        lna_mesh_t,
        config->max_mesh_count
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
        {
            .binding = 1,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = NULL,
        },
        {
            .binding = 2,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
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
            &mesh_system->descriptor_set_layout
            )
        )

    //! GRAPHICS PIPELINE

    lna_mesh_system_create_graphics_pipeline(
        mesh_system,
        config->renderer
        );

    //! DESCRIPTOR POOL

    lna_mesh_system_create_descriptor_pool(
        mesh_system,
        config->renderer
        );
}

lna_mesh_t* lna_mesh_system_new_mesh(lna_mesh_system_t* mesh_system, const lna_mesh_config_t* config)
{
    lna_assert(mesh_system)
    lna_assert(config)

    lna_mesh_t* mesh = NULL;
    lna_vector_new_element(&mesh_system->meshes, mesh);

    lna_assert(mesh)
    lna_assert(mesh->material == NULL)
    lna_assert(mesh->vertex_buffer == VK_NULL_HANDLE)
    lna_assert(mesh->vertex_buffer_memory == VK_NULL_HANDLE)
    lna_assert(mesh->index_buffer == VK_NULL_HANDLE)
    lna_assert(mesh->index_buffer_memory == VK_NULL_HANDLE)
    lna_assert(lna_array_is_empty(&mesh->mvp_uniform_buffers))
    lna_assert(lna_array_is_empty(&mesh->mvp_uniform_buffers_memory))
    lna_assert(lna_array_is_empty(&mesh->descriptor_sets))
    lna_assert(mesh->model_matrix == NULL)
    lna_assert(mesh->view_matrix == NULL)
    lna_assert(mesh->projection_matrix == NULL)
    lna_assert(config)
    lna_assert(config->vertices)
    lna_assert(config->vertex_count > 0)
    lna_assert(config->indices)
    lna_assert(config->index_count > 0)
    lna_assert(config->model_matrix)
    lna_assert(config->view_matrix)
    lna_assert(config->projection_matrix)
    lna_assert(mesh_system)

    lna_renderer_t* renderer = mesh_system->renderer;
    lna_assert(renderer)

    mesh->model_matrix      = config->model_matrix;
    mesh->view_matrix       = config->view_matrix;
    mesh->projection_matrix = config->projection_matrix;
    mesh->material          = config->material;
    mesh->index_count       = config->index_count;  

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
            &mesh->vertex_buffer,
            &mesh->vertex_buffer_memory
            );
        lna_vulkan_copy_buffer(
            renderer->device,
            renderer->command_pool,
            renderer->graphics_queue,
            staging_buffer,
            mesh->vertex_buffer,
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
            &mesh->index_buffer,
            &mesh->index_buffer_memory
            );
        lna_vulkan_copy_buffer(
            renderer->device,
            renderer->command_pool,
            renderer->graphics_queue,
            staging_buffer,
            mesh->index_buffer,
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

    lna_mesh_create_uniform_buffer(
        mesh,
        renderer
        );

    //! DESCRIPTOR SETS

    lna_mesh_create_descriptor_sets(
        mesh,
        mesh_system
        );

    return mesh;
}

void lna_mesh_system_draw(lna_mesh_system_t* mesh_system)
{
    lna_assert(mesh_system)

    lna_renderer_t* renderer = mesh_system->renderer;
    lna_assert(renderer)

    VkCommandBuffer command_buffer = lna_array_at_ref(&mesh_system->renderer->command_buffers, mesh_system->renderer->image_index);

    vkCmdBindPipeline(
        command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        mesh_system->pipeline
        );
    for (uint32_t i = 0; i < lna_vector_size(&mesh_system->meshes); ++i)
    {
        lna_mesh_t* mesh = lna_vector_at_ptr(&mesh_system->meshes, i);

        lna_assert(mesh->model_matrix)
        lna_assert(mesh->view_matrix)
        lna_assert(mesh->projection_matrix)

        const lna_mesh_mvp_uniform_t mvp_ubo =
        {
            .model = *mesh->model_matrix,
            .view = *mesh->view_matrix,
            .projection = *mesh->projection_matrix,
        };
        void *mvp_data;
        VULKAN_CHECK_RESULT(
            vkMapMemory(
                renderer->device,
                lna_array_at_ref(&mesh->mvp_uniform_buffers_memory, renderer->image_index),
                0,
                sizeof(mvp_ubo),
                0,
                &mvp_data
                )
            )
        memcpy(
            mvp_data,
            &mvp_ubo,
            sizeof(mvp_ubo));
        vkUnmapMemory(
            renderer->device,
            lna_array_at_ref(&mesh->mvp_uniform_buffers_memory, renderer->image_index)
            );

        const lna_mesh_light_uniform_t light_ubo =
        {
            .light_position   = { 1.5f, 1.5f, 1.5f, 0.0f }, // TODO: remove hard coded value!
            .view_position    = { 2.0f, 2.0f, 2.0f, 0.0f }, // TODO: remove hard coded value!
            .light_color      = { 1.0f, 1.0f, 1.0f, 0.0f }, // TODO: remove hard coded value!
        };
        void *light_data;
        VULKAN_CHECK_RESULT(
            vkMapMemory(
                renderer->device,
                lna_array_at_ref(&mesh->light_uniform_buffers_memory, renderer->image_index),
                0,
                sizeof(light_ubo),
                0,
                &light_data
                )
            )
        memcpy(
            light_data,
            &light_ubo,
            sizeof(light_ubo));
        vkUnmapMemory(
            renderer->device,
            lna_array_at_ref(&mesh->light_uniform_buffers_memory, renderer->image_index)
            );

        vkCmdBindDescriptorSets(
            command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            mesh_system->pipeline_layout,
            0,
            1,
            lna_array_at_ptr(&mesh->descriptor_sets, renderer->image_index),
            0,
            NULL
            );
        const VkBuffer vertex_buffers[] =
        {
            mesh->vertex_buffer
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
            mesh->index_buffer,
            0,
            VK_INDEX_TYPE_UINT32
            );
        vkCmdDrawIndexed(
            command_buffer,
            mesh->index_count,
            1,
            0,
            0,
            0
            );
    }
}

void lna_mesh_system_release(lna_mesh_system_t* mesh_system)
{
    lna_assert(mesh_system)
    lna_assert(mesh_system->renderer)
    lna_assert(mesh_system->renderer->device)

    vkDestroyDescriptorSetLayout(
        mesh_system->renderer->device,
        mesh_system->descriptor_set_layout,
        NULL
        );
    for (uint32_t i = 0; i < lna_vector_size(&mesh_system->meshes); ++i)
    {
        lna_mesh_t* mesh = lna_vector_at_ptr(&mesh_system->meshes, i);

        vkDestroyBuffer(
            mesh_system->renderer->device,
            mesh->index_buffer,
            NULL
            );
        vkFreeMemory(
            mesh_system->renderer->device,
            mesh->index_buffer_memory,
            NULL
            );
        vkDestroyBuffer(
            mesh_system->renderer->device,
            mesh->vertex_buffer,
            NULL
            );
        vkFreeMemory(
            mesh_system->renderer->device,
            mesh->vertex_buffer_memory,
            NULL
            );
    }
}
