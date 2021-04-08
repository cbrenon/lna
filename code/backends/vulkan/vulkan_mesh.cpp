#include <cstring>
#include "backends/mesh_backend.hpp"
#include "backends/vulkan/vulkan_mesh.hpp"
#include "backends/vulkan/vulkan_helpers.hpp"
#include "backends/vulkan/vulkan_texture.hpp"
#include "backends/vulkan/vulkan_backend.hpp"
#include "core/assert.hpp"
#include "core/memory_pool.hpp"
#include "core/log.hpp"
#include "maths/mat4.hpp"

namespace lna
{
    struct vulkan_mesh_uniform_mvp
    {
        alignas(16) mat4    model;
        alignas(16) mat4    view;
        alignas(16) mat4    projection;
    };

    struct vulkan_mesh_uniform_light_info
    {
        alignas(16) vec3   light_position;
        alignas(16) vec3   view_position;
        alignas(16) vec3   light_color;
    };
}

namespace
{
    struct vulkan_mesh_vertex_description
    {
        enum
        {
            MAX_BINDING     = 1,
            MAX_ATTRIBUTES  = 4,
        };

        VkVertexInputBindingDescription     bindings[MAX_BINDING];
        VkVertexInputAttributeDescription   attributes[MAX_ATTRIBUTES];
    };

    vulkan_mesh_vertex_description vulkan_default_mesh_vertex_description()
    {
        vulkan_mesh_vertex_description result;

        result.bindings[0].binding      = 0;
        result.bindings[0].stride       = sizeof(lna::mesh_vertex);
        result.bindings[0].inputRate    = VK_VERTEX_INPUT_RATE_VERTEX;

        result.attributes[0].binding    = 0;
        result.attributes[0].location   = 0;
        result.attributes[0].format     = VK_FORMAT_R32G32B32_SFLOAT;
        result.attributes[0].offset     = offsetof(lna::mesh_vertex, position);

        result.attributes[1].binding    = 0;
        result.attributes[1].location   = 1;
        result.attributes[1].format     = VK_FORMAT_R32G32B32A32_SFLOAT;
        result.attributes[1].offset     = offsetof(lna::mesh_vertex, color);

        result.attributes[2].binding    = 0;
        result.attributes[2].location   = 2;
        result.attributes[2].format     = VK_FORMAT_R32G32_SFLOAT;
        result.attributes[2].offset     = offsetof(lna::mesh_vertex, uv);
        
        result.attributes[3].binding    = 0;
        result.attributes[3].location   = 3;
        result.attributes[3].format     = VK_FORMAT_R32G32B32_SFLOAT;
        result.attributes[3].offset     = offsetof(lna::mesh_vertex, normal);
        return result;
    }

    void vulkan_mesh_create_graphics_pipeline(
        lna::mesh_backend& backend
        )
    {
        LNA_ASSERT(backend.renderer_backend_ptr);
        LNA_ASSERT(backend.renderer_backend_ptr->device);
        LNA_ASSERT(backend.renderer_backend_ptr->render_pass);
        LNA_ASSERT(backend.pipeline_layout == VK_NULL_HANDLE);
        LNA_ASSERT(backend.pipeline == VK_NULL_HANDLE);

        VkShaderModule vertex_shader_module = lna::vulkan_helpers::load_shader(
            backend.renderer_backend_ptr->device,
            "shaders/default_vert.spv",
            backend.renderer_backend_ptr->memory_pools[lna::renderer_backend::FRAME_LIFETIME_MEMORY_POOL]
            );
        VkShaderModule fragment_shader_module = lna::vulkan_helpers::load_shader(
            backend.renderer_backend_ptr->device,
            "shaders/default_frag.spv",
            backend.renderer_backend_ptr->memory_pools[lna::renderer_backend::FRAME_LIFETIME_MEMORY_POOL]
            );
        VkPipelineShaderStageCreateInfo shader_stage_create_infos[2]{};
        shader_stage_create_infos[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stage_create_infos[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
        shader_stage_create_infos[0].module = vertex_shader_module;
        shader_stage_create_infos[0].pName  = "main";
        shader_stage_create_infos[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stage_create_infos[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
        shader_stage_create_infos[1].module = fragment_shader_module;
        shader_stage_create_infos[1].pName  = "main";

        auto vertex_description = vulkan_default_mesh_vertex_description();

        VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
        vertex_input_state_create_info.sType                            = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_state_create_info.vertexBindingDescriptionCount    = vulkan_mesh_vertex_description::MAX_BINDING;
        vertex_input_state_create_info.pVertexBindingDescriptions       = vertex_description.bindings;
        vertex_input_state_create_info.vertexAttributeDescriptionCount  = vulkan_mesh_vertex_description::MAX_ATTRIBUTES;
        vertex_input_state_create_info.pVertexAttributeDescriptions     = vertex_description.attributes;

        VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info{};
        input_assembly_state_create_info.sType                          = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly_state_create_info.topology                       = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly_state_create_info.primitiveRestartEnable         = VK_FALSE;

        VkViewport viewport{};
        viewport.x          = 0.0f;
        viewport.y          = 0.0f;
        viewport.width      = static_cast<float>(backend.renderer_backend_ptr->swap_chain_extent.width);
        viewport.height     = static_cast<float>(backend.renderer_backend_ptr->swap_chain_extent.height);
        viewport.minDepth   = 0.0f;
        viewport.maxDepth   = 1.0f;

        VkRect2D scissor{};
        scissor.offset      = { 0, 0 };
        scissor.extent      = backend.renderer_backend_ptr->swap_chain_extent;

        VkPipelineViewportStateCreateInfo viewport_state_create_info{};
        viewport_state_create_info.sType            = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state_create_info.viewportCount    = 1;
        viewport_state_create_info.pViewports       = &viewport;
        viewport_state_create_info.scissorCount     = 1;
        viewport_state_create_info.pScissors        = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterization_state_create_info{};
        rasterization_state_create_info.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterization_state_create_info.depthClampEnable        = VK_FALSE;
        rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
        rasterization_state_create_info.polygonMode             = backend.polygon_mode;
        rasterization_state_create_info.lineWidth               = 1.0f;
        rasterization_state_create_info.cullMode                = VK_CULL_MODE_BACK_BIT;
        rasterization_state_create_info.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterization_state_create_info.depthBiasEnable         = VK_FALSE;
        rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
        rasterization_state_create_info.depthBiasClamp          = 0.0f;
        rasterization_state_create_info.depthBiasSlopeFactor    = 0.0f;

        VkPipelineMultisampleStateCreateInfo multisample_state_create_info{};
        multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample_state_create_info.sampleShadingEnable   = VK_FALSE;
        multisample_state_create_info.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
        multisample_state_create_info.minSampleShading      = 1.0f;
        multisample_state_create_info.pSampleMask           = nullptr;
        multisample_state_create_info.alphaToCoverageEnable = VK_FALSE;
        multisample_state_create_info.alphaToOneEnable      = VK_FALSE;

        VkPipelineColorBlendAttachmentState color_blend_attachment_state{};
        color_blend_attachment_state.colorWriteMask =
              VK_COLOR_COMPONENT_R_BIT
            | VK_COLOR_COMPONENT_G_BIT
            | VK_COLOR_COMPONENT_B_BIT
            | VK_COLOR_COMPONENT_A_BIT
            ;
        color_blend_attachment_state.blendEnable            = VK_FALSE;
        color_blend_attachment_state.srcColorBlendFactor    = VK_BLEND_FACTOR_ONE;
        color_blend_attachment_state.dstColorBlendFactor    = VK_BLEND_FACTOR_ZERO;
        color_blend_attachment_state.colorBlendOp           = VK_BLEND_OP_ADD;
        color_blend_attachment_state.srcAlphaBlendFactor    = VK_BLEND_FACTOR_ONE;
        color_blend_attachment_state.dstAlphaBlendFactor    = VK_BLEND_FACTOR_ZERO;
        color_blend_attachment_state.alphaBlendOp           = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo color_blender_state_create_info{};
        color_blender_state_create_info.sType               = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blender_state_create_info.logicOpEnable       = VK_FALSE;
        color_blender_state_create_info.logicOp             = VK_LOGIC_OP_COPY;
        color_blender_state_create_info.attachmentCount     = 1;
        color_blender_state_create_info.pAttachments        = &color_blend_attachment_state;
        color_blender_state_create_info.blendConstants[0]   = 0.0f;
        color_blender_state_create_info.blendConstants[1]   = 0.0f;
        color_blender_state_create_info.blendConstants[2]   = 0.0f;
        color_blender_state_create_info.blendConstants[3]   = 0.0f;

        VkDynamicState dynamic_states[] =
        {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_LINE_WIDTH
        };

        VkPipelineDynamicStateCreateInfo dynamic_state_create_info{};
        dynamic_state_create_info.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state_create_info.dynamicStateCount = 2;
        dynamic_state_create_info.pDynamicStates    = dynamic_states;

        VkPipelineLayoutCreateInfo  layout_create_info{};
        layout_create_info.sType                    = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layout_create_info.setLayoutCount           = 1;
        layout_create_info.pSetLayouts              = &backend.descriptor_set_layout;
        layout_create_info.pushConstantRangeCount   = 0;
        layout_create_info.pPushConstantRanges      = nullptr;

        VULKAN_CHECK_RESULT(
            vkCreatePipelineLayout(
                backend.renderer_backend_ptr->device,
                &layout_create_info,
                nullptr,
                &backend.pipeline_layout
                )
            )

        VkPipelineDepthStencilStateCreateInfo depth_stencial_state_create_info{};
        depth_stencial_state_create_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencial_state_create_info.depthTestEnable        = VK_TRUE;
        depth_stencial_state_create_info.depthWriteEnable       = VK_TRUE;
        depth_stencial_state_create_info.depthCompareOp         = VK_COMPARE_OP_LESS;
        depth_stencial_state_create_info.depthBoundsTestEnable  = VK_FALSE;
        depth_stencial_state_create_info.minDepthBounds         = 0.0f;
        depth_stencial_state_create_info.maxDepthBounds         = 1.0f;
        depth_stencial_state_create_info.stencilTestEnable      = VK_FALSE;
        depth_stencial_state_create_info.front                  = {};
        depth_stencial_state_create_info.back                   = {};

        VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{};
        graphics_pipeline_create_info.sType                 = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        graphics_pipeline_create_info.stageCount            = 2;
        graphics_pipeline_create_info.pStages               = shader_stage_create_infos;
        graphics_pipeline_create_info.pVertexInputState     = &vertex_input_state_create_info;
        graphics_pipeline_create_info.pInputAssemblyState   = &input_assembly_state_create_info;
        graphics_pipeline_create_info.pViewportState        = &viewport_state_create_info;
        graphics_pipeline_create_info.pRasterizationState   = &rasterization_state_create_info;
        graphics_pipeline_create_info.pMultisampleState     = &multisample_state_create_info;
        graphics_pipeline_create_info.pDepthStencilState    = &depth_stencial_state_create_info;
        graphics_pipeline_create_info.pColorBlendState      = &color_blender_state_create_info;
        graphics_pipeline_create_info.pDynamicState         = nullptr;
        graphics_pipeline_create_info.layout                = backend.pipeline_layout;
        graphics_pipeline_create_info.renderPass            = backend.renderer_backend_ptr->render_pass;
        graphics_pipeline_create_info.subpass               = 0;
        graphics_pipeline_create_info.basePipelineHandle    = VK_NULL_HANDLE;
        graphics_pipeline_create_info.basePipelineIndex     = -1;

        VULKAN_CHECK_RESULT(
            vkCreateGraphicsPipelines(
                backend.renderer_backend_ptr->device,
                VK_NULL_HANDLE,
                1,
                &graphics_pipeline_create_info,
                nullptr,
                &backend.pipeline
                )
            )

        vkDestroyShaderModule(backend.renderer_backend_ptr->device, fragment_shader_module, nullptr);
        vkDestroyShaderModule(backend.renderer_backend_ptr->device, vertex_shader_module, nullptr);
    }

    void vulkan_mesh_create_descriptor_pool(
        lna::mesh_backend& backend
        )
    {

        LNA_ASSERT(backend.renderer_backend_ptr);
        LNA_ASSERT(backend.renderer_backend_ptr->device);
        LNA_ASSERT(backend.descriptor_pool == VK_NULL_HANDLE);
        LNA_ASSERT(backend.max_mesh_count > 0);

        VkDescriptorPoolSize pool_sizes[3] {};
        pool_sizes[0].type              = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        pool_sizes[0].descriptorCount   = backend.renderer_backend_ptr->swap_chain_image_count;
        pool_sizes[1].type              = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        pool_sizes[1].descriptorCount   = backend.renderer_backend_ptr->swap_chain_image_count;
        pool_sizes[2].type              = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        pool_sizes[2].descriptorCount   = backend.renderer_backend_ptr->swap_chain_image_count;

        VkDescriptorPoolCreateInfo pool_create_info{};
        pool_create_info.sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_create_info.poolSizeCount  = static_cast<uint32_t>(sizeof(pool_sizes) / sizeof(pool_sizes[0]));
        pool_create_info.pPoolSizes     = pool_sizes;
        pool_create_info.maxSets        = backend.renderer_backend_ptr->swap_chain_image_count * backend.max_mesh_count;

        VULKAN_CHECK_RESULT(
            vkCreateDescriptorPool(
                backend.renderer_backend_ptr->device,
                &pool_create_info,
                nullptr,
                &backend.descriptor_pool
                )
            )
    }

    void vulkan_mesh_create_uniform_buffer(
        lna::mesh& mesh,
        lna::mesh_backend& backend
        )
    {
        LNA_ASSERT(mesh.mvp_uniform_buffers == nullptr);
        LNA_ASSERT(mesh.mvp_uniform_buffers_memory == nullptr);
        LNA_ASSERT(mesh.light_uniform_buffers == nullptr);
        LNA_ASSERT(mesh.light_uniform_buffers_memory == nullptr);
        LNA_ASSERT(mesh.swap_chain_image_count == 0);
        LNA_ASSERT(backend.renderer_backend_ptr);
        LNA_ASSERT(backend.renderer_backend_ptr->device);
        LNA_ASSERT(backend.renderer_backend_ptr->physical_device);
        LNA_ASSERT(backend.renderer_backend_ptr->swap_chain_image_count > 0);

        mesh.swap_chain_image_count = backend.renderer_backend_ptr->swap_chain_image_count;

        {
            VkDeviceSize uniform_buffer_size    = sizeof(lna::vulkan_mesh_uniform_mvp);
            mesh.mvp_uniform_buffers                = LNA_ALLOC(
                backend.renderer_backend_ptr->memory_pools[lna::renderer_backend::SWAP_CHAIN_LIFETIME_MEMORY_POOL],
                VkBuffer,
                mesh.swap_chain_image_count
                );
            mesh.mvp_uniform_buffers_memory         = LNA_ALLOC(
                backend.renderer_backend_ptr->memory_pools[lna::renderer_backend::SWAP_CHAIN_LIFETIME_MEMORY_POOL],
                VkDeviceMemory,
                mesh.swap_chain_image_count
                );

            LNA_ASSERT(mesh.mvp_uniform_buffers);
            LNA_ASSERT(mesh.mvp_uniform_buffers_memory);

            for (size_t i = 0; i < backend.renderer_backend_ptr->swap_chain_image_count; ++i)
            {
                lna::vulkan_helpers::create_buffer(
                    backend.renderer_backend_ptr->device,
                    backend.renderer_backend_ptr->physical_device,
                    uniform_buffer_size,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    mesh.mvp_uniform_buffers[i],
                    mesh.mvp_uniform_buffers_memory[i]
                    );
            }
        }

        {
            VkDeviceSize uniform_buffer_size    = sizeof(lna::vulkan_mesh_uniform_light_info);
            mesh.light_uniform_buffers          = LNA_ALLOC(
                backend.renderer_backend_ptr->memory_pools[lna::renderer_backend::SWAP_CHAIN_LIFETIME_MEMORY_POOL],
                VkBuffer,
                mesh.swap_chain_image_count
                );
            mesh.light_uniform_buffers_memory   = LNA_ALLOC(
                backend.renderer_backend_ptr->memory_pools[lna::renderer_backend::SWAP_CHAIN_LIFETIME_MEMORY_POOL],
                VkDeviceMemory,
                mesh.swap_chain_image_count
                );
    
            LNA_ASSERT(mesh.light_uniform_buffers);
            LNA_ASSERT(mesh.light_uniform_buffers_memory);
    
            for (size_t i = 0; i < backend.renderer_backend_ptr->swap_chain_image_count; ++i)
            {
                lna::vulkan_helpers::create_buffer(
                    backend.renderer_backend_ptr->device,
                    backend.renderer_backend_ptr->physical_device,
                    uniform_buffer_size,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    mesh.light_uniform_buffers[i],
                    mesh.light_uniform_buffers_memory[i]
                    );
            }
        }
    }

    void vulkan_mesh_create_descriptor_sets(
        lna::mesh& mesh,
        lna::mesh_backend& backend
        )
    {
        LNA_ASSERT(mesh.descriptor_sets == nullptr);
        LNA_ASSERT(mesh.swap_chain_image_count != 0 );
        LNA_ASSERT(mesh.mvp_uniform_buffers_memory);
        LNA_ASSERT(backend.renderer_backend_ptr->device);
        LNA_ASSERT(backend.renderer_backend_ptr->physical_device);
        LNA_ASSERT(backend.descriptor_pool);
        LNA_ASSERT(backend.descriptor_set_layout);
        LNA_ASSERT(mesh.texture_ptr);                   // texture must be assigned before calling create descriptor sets
        LNA_ASSERT(mesh.texture_ptr->image_view);
        LNA_ASSERT(mesh.texture_ptr->sampler);

        VkDescriptorSetLayout* layouts = LNA_ALLOC(
            backend.renderer_backend_ptr->memory_pools[lna::renderer_backend::FRAME_LIFETIME_MEMORY_POOL],
            VkDescriptorSetLayout,
            mesh.swap_chain_image_count
            );
        LNA_ASSERT(layouts);
        for (uint32_t i = 0; i < mesh.swap_chain_image_count; ++i)
        {
            layouts[i] = backend.descriptor_set_layout;
        }

        VkDescriptorSetAllocateInfo allocate_info{};
        allocate_info.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocate_info.descriptorPool        = backend.descriptor_pool;
        allocate_info.descriptorSetCount    = mesh.swap_chain_image_count;
        allocate_info.pSetLayouts           = layouts;

        mesh.descriptor_sets = LNA_ALLOC(
            backend.renderer_backend_ptr->memory_pools[lna::renderer_backend::SWAP_CHAIN_LIFETIME_MEMORY_POOL],
            VkDescriptorSet,
            mesh.swap_chain_image_count
            );
        LNA_ASSERT(mesh.descriptor_sets);

        VULKAN_CHECK_RESULT(
            vkAllocateDescriptorSets(
                backend.renderer_backend_ptr->device,
                &allocate_info,
                mesh.descriptor_sets
                )
            )

        for (size_t i = 0; i < mesh.swap_chain_image_count; ++i)
        {
            VkDescriptorBufferInfo buffer_info{};
            buffer_info.buffer  = mesh.mvp_uniform_buffers[i];
            buffer_info.offset  = 0;
            buffer_info.range   = sizeof(lna::vulkan_mesh_uniform_mvp);

            VkDescriptorImageInfo image_info{};
            image_info.imageLayout  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            image_info.imageView    = mesh.texture_ptr->image_view;
            image_info.sampler      = mesh.texture_ptr->sampler;

            VkDescriptorBufferInfo light_buffer_info{};
            light_buffer_info.buffer    = mesh.light_uniform_buffers[i];
            light_buffer_info.offset    = 0;
            light_buffer_info.range     = sizeof(lna::vulkan_mesh_uniform_light_info);

            VkWriteDescriptorSet write_descriptors[3] {};

            write_descriptors[0].sType              = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_descriptors[0].dstSet             = mesh.descriptor_sets[i];
            write_descriptors[0].dstBinding         = 0;
            write_descriptors[0].dstArrayElement    = 0;
            write_descriptors[0].descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            write_descriptors[0].descriptorCount    = 1;
            write_descriptors[0].pBufferInfo        = &buffer_info;
            write_descriptors[0].pImageInfo         = nullptr;
            write_descriptors[0].pTexelBufferView   = nullptr;

            write_descriptors[1].sType              = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_descriptors[1].dstSet             = mesh.descriptor_sets[i];
            write_descriptors[1].dstBinding         = 1;
            write_descriptors[1].dstArrayElement    = 0;
            write_descriptors[1].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write_descriptors[1].descriptorCount    = 1;
            write_descriptors[1].pBufferInfo        = nullptr;
            write_descriptors[1].pImageInfo         = &image_info;
            write_descriptors[1].pTexelBufferView   = nullptr;

            write_descriptors[2].sType              = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write_descriptors[2].dstSet             = mesh.descriptor_sets[i];
            write_descriptors[2].dstBinding         = 2;
            write_descriptors[2].dstArrayElement    = 0;
            write_descriptors[2].descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            write_descriptors[2].descriptorCount    = 1;
            write_descriptors[2].pBufferInfo        = &light_buffer_info;
            write_descriptors[2].pTexelBufferView   = nullptr;
            write_descriptors[2].pImageInfo         = nullptr;

            vkUpdateDescriptorSets(
                backend.renderer_backend_ptr->device,
                static_cast<uint32_t>(sizeof(write_descriptors) / sizeof(write_descriptors[0])),
                write_descriptors,
                0,
                nullptr
                );
        }
    }

    void vulkan_mesh_on_swap_chain_cleanup(void* owner)
    {
        LNA_ASSERT(owner);

        lna::mesh_backend* backend = (lna::mesh_backend*)owner;

        LNA_ASSERT(backend->renderer_backend_ptr);

        vkDestroyPipeline(
            backend->renderer_backend_ptr->device,
            backend->pipeline,
            nullptr
            );
        vkDestroyPipelineLayout(
            backend->renderer_backend_ptr->device,
            backend->pipeline_layout,
            nullptr
            );

        for (uint32_t i = 0; i < backend->cur_mesh_count; ++i)
        {
            for (size_t j = 0; j < backend->meshes[i].swap_chain_image_count; ++j)
            {
                vkDestroyBuffer(
                    backend->renderer_backend_ptr->device,
                    backend->meshes[i].mvp_uniform_buffers[j],
                    nullptr
                    );
                vkFreeMemory(
                    backend->renderer_backend_ptr->device,
                    backend->meshes[i].mvp_uniform_buffers_memory[j],
                    nullptr
                    );
                vkDestroyBuffer(
                    backend->renderer_backend_ptr->device,
                    backend->meshes[i].light_uniform_buffers[j],
                    nullptr
                    );
                vkFreeMemory(
                    backend->renderer_backend_ptr->device,
                    backend->meshes[i].light_uniform_buffers_memory[j],
                    nullptr
                    );
            }
            backend->meshes[i].mvp_uniform_buffers          = nullptr;
            backend->meshes[i].mvp_uniform_buffers_memory   = nullptr;
            backend->meshes[i].light_uniform_buffers        = nullptr;
            backend->meshes[i].light_uniform_buffers_memory = nullptr;
            backend->meshes[i].swap_chain_image_count   = 0;

            //! NOTE: no need to explicity clean up vulkan descriptor sets objects
            //! because it is done when the vulkan descriptor pool is destroyed.
            //! we just have to reset the array and wait for filling it again.
            //! the memory pool where we reserved memory for descriptor_sets has already
            //! been reset by the vulkan renderer backend.
            backend->meshes[i].descriptor_sets = nullptr;
        }
        vkDestroyDescriptorPool(
            backend->renderer_backend_ptr->device,
            backend->descriptor_pool,
            nullptr
            );
    }

    void vulkan_mesh_on_swap_chain_recreate(void* owner)
    {
        LNA_ASSERT(owner);

        lna::mesh_backend* backend = (lna::mesh_backend*)owner;

        vulkan_mesh_create_graphics_pipeline(*backend);
        vulkan_mesh_create_descriptor_pool(*backend);

        for (uint32_t i = 0; i < backend->cur_mesh_count; ++i)
        {
            vulkan_mesh_create_uniform_buffer(
                backend->meshes[i],
                *backend
                );
            vulkan_mesh_create_descriptor_sets(
                backend->meshes[i],
                *backend
                );
        }
    }

    void vulkan_mesh_on_draw(
        void* owner,
        uint32_t command_buffer_image_index
        )
    {
        LNA_ASSERT(owner);

        lna::mesh_backend* backend = (lna::mesh_backend*)owner;

        LNA_ASSERT(backend->renderer_backend_ptr);
        LNA_ASSERT(command_buffer_image_index < backend->renderer_backend_ptr->swap_chain_image_count);
        LNA_ASSERT(backend->renderer_backend_ptr->device);

         vkCmdBindPipeline(
                backend->renderer_backend_ptr->command_buffers[command_buffer_image_index],
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                backend->pipeline
                );
        for (uint32_t m = 0; m < backend->cur_mesh_count; ++m)
        {
            LNA_ASSERT(backend->meshes[m].model_mat_ptr);
            LNA_ASSERT(backend->meshes[m].view_mat_ptr);
            LNA_ASSERT(backend->meshes[m].projection_mat_ptr);

            {
                lna::vulkan_mesh_uniform_mvp ubo{};
                ubo.model       = *backend->meshes[m].model_mat_ptr;
                ubo.view        = *backend->meshes[m].view_mat_ptr;
                ubo.projection  = *backend->meshes[m].projection_mat_ptr;
                void* data;
                VULKAN_CHECK_RESULT(
                    vkMapMemory(
                        backend->renderer_backend_ptr->device,
                        backend->meshes[m].mvp_uniform_buffers_memory[command_buffer_image_index],
                        0,
                        sizeof(ubo),
                        0,
                        &data
                        )
                    )
                memcpy(
                    data,
                    &ubo,
                    sizeof(ubo)
                    );
                vkUnmapMemory(
                    backend->renderer_backend_ptr->device,
                    backend->meshes[m].mvp_uniform_buffers_memory[command_buffer_image_index]
                    );
            }

            {
                lna::vulkan_mesh_uniform_light_info light_info{};
                light_info.light_color      = { 1.0f, 1.0f, 1.0f }; // TODO: remove hard coded value!
                light_info.light_position   = { 7.0f, 8.0f, 7.0f }; // TODO: remove hard coded value!
                light_info.view_position    = { 6.0f, 4.0f, 6.0f }; // TODO: remove hard coded value!
                void* data;
                VULKAN_CHECK_RESULT(
                    vkMapMemory(
                        backend->renderer_backend_ptr->device,
                        backend->meshes[m].light_uniform_buffers_memory[command_buffer_image_index],
                        0,
                        sizeof(light_info),
                        0,
                        &data
                        )
                    )
                memcpy(
                    data,
                    &light_info,
                    sizeof(light_info)
                    );
                vkUnmapMemory(
                    backend->renderer_backend_ptr->device,
                    backend->meshes[m].light_uniform_buffers_memory[command_buffer_image_index]
                    );
            }

            vkCmdBindDescriptorSets(
                backend->renderer_backend_ptr->command_buffers[command_buffer_image_index],
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                backend->pipeline_layout,
                0,
                1,
                &backend->meshes[m].descriptor_sets[command_buffer_image_index],
                0,
                nullptr
                );
            VkBuffer        vertex_buffers[]    = { backend->meshes[m].vertex_buffer };
            VkDeviceSize    offsets[]           = { 0 };
            vkCmdBindVertexBuffers(
                backend->renderer_backend_ptr->command_buffers[command_buffer_image_index],
                0,
                1,
                vertex_buffers,
                offsets
                );
            vkCmdBindIndexBuffer(
                backend->renderer_backend_ptr->command_buffers[command_buffer_image_index],
                backend->meshes[m].index_buffer,
                0,
                VK_INDEX_TYPE_UINT16
                );
            vkCmdDrawIndexed(
                backend->renderer_backend_ptr->command_buffers[command_buffer_image_index],
                backend->meshes[m].index_count,
                1,
                0,
                0,
                0
                );
        }
    }
}

namespace lna
{
    void mesh_backend_configure(
        mesh_backend& backend,
        mesh_backend_config& config
        )
    {
        LNA_ASSERT(backend.renderer_backend_ptr == nullptr);
        LNA_ASSERT(backend.descriptor_set_layout == VK_NULL_HANDLE);
        LNA_ASSERT(backend.meshes == nullptr);
        LNA_ASSERT(backend.cur_mesh_count == 0);
        LNA_ASSERT(backend.max_mesh_count == 0);
        LNA_ASSERT(config.renderer_backend_ptr);
        LNA_ASSERT(config.persistent_memory_pool_ptr);
        LNA_ASSERT(config.max_mesh_count > 0 );

        switch(config.polygon_mode)
        {
            case mesh_polygon_mode::FILL:
                backend.polygon_mode = VK_POLYGON_MODE_FILL;
                break;
            case mesh_polygon_mode::LINE:
                backend.polygon_mode = VK_POLYGON_MODE_LINE;
                break;
            default:
                LNA_ASSERT(0);
                break;
        }

        vulkan_renderer_backend_register_swap_chain_callbacks(
            *config.renderer_backend_ptr,
            vulkan_mesh_on_swap_chain_cleanup,
            vulkan_mesh_on_swap_chain_recreate,
            vulkan_mesh_on_draw,
            &backend
            );

        backend.renderer_backend_ptr    = config.renderer_backend_ptr;
        backend.max_mesh_count          = config.max_mesh_count;
        backend.meshes                  = LNA_ALLOC(
            *config.persistent_memory_pool_ptr,
            mesh,
            config.max_mesh_count
            );
        LNA_ASSERT(backend.meshes);
        
        for (uint32_t i = 0; i < backend.max_mesh_count; ++i)
        {
            backend.meshes[i].vertex_buffer                 = VK_NULL_HANDLE;
            backend.meshes[i].vertex_buffer_memory          = VK_NULL_HANDLE;
            backend.meshes[i].index_buffer                  = VK_NULL_HANDLE;
            backend.meshes[i].index_buffer_memory           = VK_NULL_HANDLE;
            backend.meshes[i].vertex_count                  = 0;
            backend.meshes[i].index_count                   = 0;
            backend.meshes[i].mvp_uniform_buffers           = nullptr;
            backend.meshes[i].mvp_uniform_buffers_memory    = nullptr;
            backend.meshes[i].light_uniform_buffers         = nullptr;
            backend.meshes[i].light_uniform_buffers_memory  = nullptr;
            backend.meshes[i].descriptor_sets               = nullptr;
            backend.meshes[i].swap_chain_image_count        = 0;
            backend.meshes[i].texture_ptr                   = nullptr;
            backend.meshes[i].view_mat_ptr                  = nullptr;
            backend.meshes[i].projection_mat_ptr            = nullptr;
        }

        //! DESCRIPTOR SET LAYOUT

        VkDescriptorSetLayoutBinding mvp_layout_binding{};
        mvp_layout_binding.binding                  = 0;
        mvp_layout_binding.descriptorCount          = 1;
        mvp_layout_binding.descriptorType           = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        mvp_layout_binding.stageFlags               = VK_SHADER_STAGE_VERTEX_BIT;
        mvp_layout_binding.pImmutableSamplers       = nullptr;

        VkDescriptorSetLayoutBinding sampler_layout_binding{};
        sampler_layout_binding.binding              = 1;
        sampler_layout_binding.descriptorCount      = 1;
        sampler_layout_binding.descriptorType       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        sampler_layout_binding.stageFlags           = VK_SHADER_STAGE_FRAGMENT_BIT;
        sampler_layout_binding.pImmutableSamplers   = nullptr;

        VkDescriptorSetLayoutBinding light_layout_binding{};
        light_layout_binding.binding                = 2;
        light_layout_binding.descriptorCount        = 1;
        light_layout_binding.descriptorType         = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        light_layout_binding.stageFlags             = VK_SHADER_STAGE_FRAGMENT_BIT;
        light_layout_binding.pImmutableSamplers     = nullptr;

        VkDescriptorSetLayoutBinding bindings[3];
        bindings[0] = mvp_layout_binding;
        bindings[1] = sampler_layout_binding;
        bindings[2] = light_layout_binding;

        VkDescriptorSetLayoutCreateInfo layout_create_info{};
        layout_create_info.sType                    = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_create_info.bindingCount             = static_cast<uint32_t>(sizeof(bindings) / sizeof(bindings[0]));
        layout_create_info.pBindings                = bindings;

        VULKAN_CHECK_RESULT(
            vkCreateDescriptorSetLayout(
                backend.renderer_backend_ptr->device,
                &layout_create_info,
                nullptr,
                &backend.descriptor_set_layout
                )
            )

        //! GRAPHICS PIPELINE

        vulkan_mesh_create_graphics_pipeline(backend);

        //! DESCRIPTOR POOL

        vulkan_mesh_create_descriptor_pool(backend);
    }

    mesh* mesh_backend_new_mesh(
        mesh_backend& backend,
        mesh_config& config
        )
    {
        LNA_ASSERT(backend.renderer_backend_ptr);
        LNA_ASSERT(backend.renderer_backend_ptr->device);
        LNA_ASSERT(backend.renderer_backend_ptr->physical_device);
        LNA_ASSERT(backend.renderer_backend_ptr->command_pool);
        LNA_ASSERT(backend.renderer_backend_ptr->graphics_queue);
        LNA_ASSERT(backend.meshes);
        LNA_ASSERT(backend.cur_mesh_count < backend.max_mesh_count);
        LNA_ASSERT(config.vertices);
        LNA_ASSERT(config.indices);
        LNA_ASSERT(config.vertex_count > 0);
        LNA_ASSERT(config.index_count > 0);
        LNA_ASSERT(config.texture_ptr);
        LNA_ASSERT(config.model_mat_ptr);
        LNA_ASSERT(config.view_mat_ptr);
        LNA_ASSERT(config.projection_mat_ptr);

        mesh& new_mesh = backend.meshes[backend.cur_mesh_count++];

        LNA_ASSERT(new_mesh.vertex_buffer == VK_NULL_HANDLE);
        LNA_ASSERT(new_mesh.vertex_buffer_memory == VK_NULL_HANDLE);
        LNA_ASSERT(new_mesh.index_buffer == VK_NULL_HANDLE);
        LNA_ASSERT(new_mesh.index_buffer_memory == VK_NULL_HANDLE);
        LNA_ASSERT(new_mesh.vertex_count == 0);
        LNA_ASSERT(new_mesh.index_count == 0);
        LNA_ASSERT(new_mesh.mvp_uniform_buffers == nullptr);
        LNA_ASSERT(new_mesh.mvp_uniform_buffers_memory == nullptr);
        LNA_ASSERT(new_mesh.descriptor_sets == nullptr);
        LNA_ASSERT(new_mesh.swap_chain_image_count == 0);
        LNA_ASSERT(new_mesh.texture_ptr == nullptr);
        LNA_ASSERT(new_mesh.model_mat_ptr == nullptr);
        LNA_ASSERT(new_mesh.view_mat_ptr == nullptr);
        LNA_ASSERT(new_mesh.projection_mat_ptr == nullptr);

        new_mesh.model_mat_ptr      = config.model_mat_ptr;
        new_mesh.view_mat_ptr       = config.view_mat_ptr;
        new_mesh.projection_mat_ptr = config.projection_mat_ptr;
        new_mesh.texture_ptr        = config.texture_ptr;

        //! VERTEX BUFFER PART

        {
            new_mesh.vertex_count   = config.vertex_count;
            auto vertex_buffer_size = sizeof(config.vertices[0]) * new_mesh.vertex_count;
            VkBuffer        staging_buffer;
            VkDeviceMemory  staging_buffer_memory;
            lna::vulkan_helpers::create_buffer(
                backend.renderer_backend_ptr->device,
                backend.renderer_backend_ptr->physical_device,
                vertex_buffer_size,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                staging_buffer,
                staging_buffer_memory
                );
            void* vertices_data;
            VULKAN_CHECK_RESULT(
                vkMapMemory(
                    backend.renderer_backend_ptr->device,
                    staging_buffer_memory,
                    0,
                    vertex_buffer_size,
                    0,
                    &vertices_data
                    )
                )
	        std::memcpy(
                vertices_data,
                config.vertices,
                vertex_buffer_size
                );
	        vkUnmapMemory(
                backend.renderer_backend_ptr->device,
                staging_buffer_memory
                );
            lna::vulkan_helpers::create_buffer(
                backend.renderer_backend_ptr->device,
                backend.renderer_backend_ptr->physical_device,
                vertex_buffer_size,
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                new_mesh.vertex_buffer,
                new_mesh.vertex_buffer_memory
                );
            lna::vulkan_helpers::copy_buffer(
                backend.renderer_backend_ptr->device,
                backend.renderer_backend_ptr->command_pool,
                backend.renderer_backend_ptr->graphics_queue,
                staging_buffer,
                new_mesh.vertex_buffer,
                vertex_buffer_size
                );
            vkDestroyBuffer(
                backend.renderer_backend_ptr->device,
                staging_buffer,
                nullptr
                );
            vkFreeMemory(
                backend.renderer_backend_ptr->device,
                staging_buffer_memory,
                nullptr
                );
        }

        //! INDEX BUFFER PART

        {
            new_mesh.index_count   = config.index_count;
            auto index_buffer_size = sizeof(config.indices[0]) * new_mesh.index_count;
            VkBuffer        staging_buffer;
            VkDeviceMemory  staging_buffer_memory;
            lna::vulkan_helpers::create_buffer(
                backend.renderer_backend_ptr->device,
                backend.renderer_backend_ptr->physical_device,
                index_buffer_size,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                staging_buffer,
                staging_buffer_memory
                );
            void* indices_data;
            VULKAN_CHECK_RESULT(
                vkMapMemory(
                    backend.renderer_backend_ptr->device,
                    staging_buffer_memory,
                    0,
                    index_buffer_size,
                    0,
                    &indices_data
                    )
                )
            std::memcpy(
                indices_data,
                config.indices,
                index_buffer_size
                );
            vkUnmapMemory(
                backend.renderer_backend_ptr->device,
                staging_buffer_memory
                );
            lna::vulkan_helpers::create_buffer(
                backend.renderer_backend_ptr->device,
                backend.renderer_backend_ptr->physical_device,
                index_buffer_size,
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                new_mesh.index_buffer,
                new_mesh.index_buffer_memory
                );
            lna::vulkan_helpers::copy_buffer(
                backend.renderer_backend_ptr->device,
                backend.renderer_backend_ptr->command_pool,
                backend.renderer_backend_ptr->graphics_queue,
                staging_buffer,
                new_mesh.index_buffer,
                index_buffer_size
                );
            vkDestroyBuffer(
                backend.renderer_backend_ptr->device,
                staging_buffer,
                nullptr
                );
            vkFreeMemory(
                backend.renderer_backend_ptr->device,
                staging_buffer_memory,
                nullptr
                );
        }

        //! UNIFORM BUFFER

        vulkan_mesh_create_uniform_buffer(
            new_mesh,
            backend
            );

        //! DESCRIPTOR SETS

        vulkan_mesh_create_descriptor_sets(
            new_mesh,
            backend
            );

        return &new_mesh;
    }

    void mesh_backend_release(
        mesh_backend& backend
        )
    {
        LNA_ASSERT(backend.renderer_backend_ptr);

        if (backend.descriptor_set_layout)
        {
            vkDestroyDescriptorSetLayout(
                backend.renderer_backend_ptr->device,
                backend.descriptor_set_layout,
                nullptr
                );
        }

        for (uint32_t i = 0; i < backend.cur_mesh_count; ++i)
        {
            vkDestroyBuffer(
                backend.renderer_backend_ptr->device,
                backend.meshes[i].index_buffer,
                nullptr
                );
            vkFreeMemory(
                backend.renderer_backend_ptr->device,
                backend.meshes[i].index_buffer_memory,
                nullptr
                );
            vkDestroyBuffer(
                backend.renderer_backend_ptr->device,
                backend.meshes[i].vertex_buffer,
                nullptr
                );
            vkFreeMemory(
                backend.renderer_backend_ptr->device,
                backend.meshes[i].vertex_buffer_memory,
                nullptr
                );
        }
    }
}
