#include <cstring>
#include <algorithm>
#include "backends/imgui_backend.hpp"
#include "backends/vulkan/vulkan_imgui.hpp"
#include "backends/vulkan/vulkan_helpers.hpp"
#include "backends/vulkan/vulkan_texture.hpp"
#include "backends/vulkan/vulkan_renderer.hpp"
#include "core/assert.hpp"
#include "core/memory_pool.hpp"
#include "imgui.h"

namespace
{
    void imgui_backend_update(
        lna::imgui_backend& backend
        )
    {
        LNA_ASSERT(backend.renderer_backend_ptr);
        LNA_ASSERT(backend.renderer_backend_ptr->device);
        LNA_ASSERT(backend.renderer_backend_ptr->physical_device);

        VkDevice            device              = backend.renderer_backend_ptr->device;
        VkPhysicalDevice    physical_device     = backend.renderer_backend_ptr->physical_device;
        ImDrawData*         im_draw_data        = ImGui::GetDrawData();
        VkDeviceSize        vertex_buffer_size  = im_draw_data->TotalVtxCount * sizeof(ImDrawVert);
        VkDeviceSize        index_buffer_size   = im_draw_data->TotalIdxCount * sizeof (ImDrawIdx);

        if ((vertex_buffer_size == 0) || (index_buffer_size == 0))
        {
            return;
        }

        if ((backend.vertex_buffer == VK_NULL_HANDLE) || (backend.vertex_count != im_draw_data->TotalVtxCount))
        {
            if (backend.vertex_data_mapped)
            {
                vkUnmapMemory(device, backend.vertex_buffer_memory);
                backend.vertex_data_mapped = nullptr;
            }
            if (backend.vertex_buffer)
            {
                vkDestroyBuffer(device, backend.vertex_buffer, nullptr);
                backend.vertex_buffer = nullptr;
            }
            if (backend.vertex_buffer_memory)
            {
                vkFreeMemory(device, backend.vertex_buffer_memory, nullptr);
                backend.vertex_buffer_memory = nullptr;
            }
            VkBufferCreateInfo buffer_create_info{};
            buffer_create_info.sType    = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            buffer_create_info.usage    = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            buffer_create_info.size     = vertex_buffer_size;
            VULKAN_CHECK_RESULT(
                vkCreateBuffer(
                    device,
                    &buffer_create_info,
                    nullptr,
                    &backend.vertex_buffer
                )
            )
            VkMemoryRequirements    memory_requirements;
            vkGetBufferMemoryRequirements(
                device,
                backend.vertex_buffer,
                &memory_requirements
                );
            VkMemoryAllocateInfo    memory_allocate_info{};
            memory_allocate_info.sType              = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            memory_allocate_info.allocationSize     = memory_requirements.size;
            memory_allocate_info.memoryTypeIndex    = lna::vulkan_helpers::find_memory_type(
                physical_device,
                memory_requirements.memoryTypeBits,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                );
            VULKAN_CHECK_RESULT(
                vkAllocateMemory(
                    device,
                    &memory_allocate_info,
                    nullptr,
                    &backend.vertex_buffer_memory
                    )
                )
            VULKAN_CHECK_RESULT(
                vkBindBufferMemory(
                    device,
                    backend.vertex_buffer,
                    backend.vertex_buffer_memory,
                    0
                    )
            )
            backend.vertex_count = im_draw_data->TotalVtxCount;
            VULKAN_CHECK_RESULT(
                vkMapMemory(
                    device,
                    backend.vertex_buffer_memory,
                    0,
                    VK_WHOLE_SIZE,
                    0,
                    &backend.vertex_data_mapped
                    )
                )
        }
        if ((backend.index_buffer == VK_NULL_HANDLE) || (backend.index_count < im_draw_data->TotalIdxCount))
        {
            if (backend.index_data_mapped)
            {
                vkUnmapMemory(device, backend.index_buffer_memory);
                backend.index_data_mapped = nullptr;
            }
            if (backend.index_buffer)
            {
                vkDestroyBuffer(device, backend.index_buffer, nullptr);
                backend.index_buffer = nullptr;
            }
            if (backend.index_buffer_memory)
            {
                vkFreeMemory(device, backend.index_buffer_memory, nullptr);
                backend.index_buffer_memory = nullptr;
            }
            VkBufferCreateInfo buffer_create_info{};
            buffer_create_info.sType    = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            buffer_create_info.usage    = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            buffer_create_info.size     = index_buffer_size;
            VULKAN_CHECK_RESULT(
                vkCreateBuffer(
                    device,
                    &buffer_create_info,
                    nullptr,
                    &backend.index_buffer
                )
            )
            VkMemoryRequirements    memory_requirements;
            vkGetBufferMemoryRequirements(
                device,
                backend.index_buffer,
                &memory_requirements
                );
            VkMemoryAllocateInfo    memory_allocate_info{};
            memory_allocate_info.sType              = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            memory_allocate_info.allocationSize     = memory_requirements.size;
            memory_allocate_info.memoryTypeIndex    = lna::vulkan_helpers::find_memory_type(
                physical_device,
                memory_requirements.memoryTypeBits,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                );
            VULKAN_CHECK_RESULT(
                vkAllocateMemory(
                    device,
                    &memory_allocate_info,
                    nullptr,
                    &backend.index_buffer_memory
                    )
                )
            VULKAN_CHECK_RESULT(
                vkBindBufferMemory(
                    device,
                    backend.index_buffer,
                    backend.index_buffer_memory,
                    0
                    )
            )
            backend.index_count = im_draw_data->TotalIdxCount;
            VULKAN_CHECK_RESULT(
                vkMapMemory(
                    device,
                    backend.index_buffer_memory,
                    0,
                    VK_WHOLE_SIZE,
                    0,
                    &backend.index_data_mapped
                    )
                )
        }

        ImDrawVert* vertex_dst  = (ImDrawVert*)backend.vertex_data_mapped;
        ImDrawIdx*  index_dst   = (ImDrawIdx*)backend.index_data_mapped;
        for (int i = 0; i < im_draw_data->CmdListsCount; ++i)
        {
            const ImDrawList* cmd_list = im_draw_data->CmdLists[i];
            std::memcpy(
                vertex_dst,
                cmd_list->VtxBuffer.Data,
                cmd_list->VtxBuffer.Size * sizeof(ImDrawVert)
                );
            std::memcpy(
                index_dst,
                cmd_list->IdxBuffer.Data,
                cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx)
                );
            vertex_dst += cmd_list->VtxBuffer.Size;
            index_dst += cmd_list->IdxBuffer.Size;
        }

        {
            VkMappedMemoryRange mapped_memory_range{};
            mapped_memory_range.sType   = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            mapped_memory_range.memory  = backend.vertex_buffer_memory;
            mapped_memory_range.offset  = 0;
            mapped_memory_range.size    = vertex_buffer_size;
            VULKAN_CHECK_RESULT(
                vkFlushMappedMemoryRanges(
                    device,
                    1,
                    &mapped_memory_range
                )   
            )
        }
        {
            VkMappedMemoryRange mapped_memory_range{};
            mapped_memory_range.sType   = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            mapped_memory_range.memory  = backend.index_buffer_memory;
            mapped_memory_range.offset  = 0;
            mapped_memory_range.size    = index_buffer_size;
            VULKAN_CHECK_RESULT(
                vkFlushMappedMemoryRanges(
                    device,
                    1,
                    &mapped_memory_range
                )   
            )
        }
    }

    void vulkan_imgui_on_draw(
        void* owner,
        uint32_t command_buffer_image_index
        )
    {
        LNA_ASSERT(owner);

        lna::imgui_backend* backend = (lna::imgui_backend*)owner;

        LNA_ASSERT(backend->renderer_backend_ptr);

        // TODO: see if I need to extract this call and move it elsewhere.
        imgui_backend_update(
            *backend
            );

        ImGuiIO& io = ImGui::GetIO();

        vkCmdBindDescriptorSets(
            backend->renderer_backend_ptr->command_buffers[command_buffer_image_index],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            backend->pipeline_layout,
            0,
            1,
            &backend->descriptor_set,
            0,
            nullptr
            );
        vkCmdBindPipeline(
            backend->renderer_backend_ptr->command_buffers[command_buffer_image_index],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            backend->pipeline
            );

        VkViewport viewport;
        viewport.width      = io.DisplaySize.x;
		viewport.height     = io.DisplaySize.y;
		viewport.minDepth   = 0.0f;
		viewport.maxDepth   = 1.0f;
        vkCmdSetViewport(
            backend->renderer_backend_ptr->command_buffers[command_buffer_image_index],
            0,
            1,
            &viewport
            );

        backend->push_const_block.scale.x = 2.0f / io.DisplaySize.x;
        backend->push_const_block.scale.y = 2.0f / io.DisplaySize.y;
        backend->push_const_block.translate.x = -1.0f;
        backend->push_const_block.translate.y = -1.0f;

        vkCmdPushConstants(
            backend->renderer_backend_ptr->command_buffers[command_buffer_image_index],
            backend->pipeline_layout,
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(lna::vulkan_imgui_push_const_block),
            &backend->push_const_block
            );

        ImDrawData* im_draw_data    = ImGui::GetDrawData();
        int         vertex_offset   = 0;
        int         index_offset    = 0;
        if (im_draw_data->CmdListsCount > 0)
        {
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(
                backend->renderer_backend_ptr->command_buffers[command_buffer_image_index],
                0,
                1,
                &backend->vertex_buffer,
                offsets
                );
            vkCmdBindIndexBuffer(
                backend->renderer_backend_ptr->command_buffers[command_buffer_image_index],
                backend->index_buffer,
                0,
                VK_INDEX_TYPE_UINT16
                );
            for (int32_t i = 0; i < im_draw_data->CmdListsCount; ++i)
            {
                const ImDrawList* cmd_list = im_draw_data->CmdLists[i];
                for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; ++j)
                {
                    const ImDrawCmd* cmd = &cmd_list->CmdBuffer[j];
                    VkRect2D scissor_rect;
                    scissor_rect.offset.x       = std::max((int32_t)(cmd->ClipRect.x), 0);
                    scissor_rect.offset.y       = std::max((int32_t)(cmd->ClipRect.y), 0);
                    scissor_rect.extent.width   = (uint32_t)(cmd->ClipRect.z - cmd->ClipRect.x);
                    scissor_rect.extent.height  = (uint32_t)(cmd->ClipRect.w - cmd->ClipRect.y);
                    vkCmdSetScissor(
                        backend->renderer_backend_ptr->command_buffers[command_buffer_image_index],
                        0,
                        1,
                        &scissor_rect
                        );
                    vkCmdDrawIndexed(
                        backend->renderer_backend_ptr->command_buffers[command_buffer_image_index],
                        cmd->ElemCount,
                        1,
                        index_offset,
                        vertex_offset,
                        0
                        );
                    index_offset += cmd->ElemCount;
                }
                vertex_offset += cmd_list->VtxBuffer.Size;
            }
        }
    }
}

namespace lna
{
    void imgui_backend_configure(
        imgui_backend& backend,
        imgui_backend_config& config
        )
    {
        LNA_ASSERT(backend.vertex_buffer == VK_NULL_HANDLE);
        LNA_ASSERT(backend.vertex_buffer_memory == VK_NULL_HANDLE);
        LNA_ASSERT(backend.index_buffer == VK_NULL_HANDLE);
        LNA_ASSERT(backend.index_buffer_memory == VK_NULL_HANDLE);
        LNA_ASSERT(backend.pipeline_cache == VK_NULL_HANDLE);
        LNA_ASSERT(backend.pipeline_layout == VK_NULL_HANDLE);
        LNA_ASSERT(backend.pipeline == VK_NULL_HANDLE);
        LNA_ASSERT(backend.descriptor_pool == VK_NULL_HANDLE);
        LNA_ASSERT(backend.descriptor_set_layout == VK_NULL_HANDLE);
        LNA_ASSERT(backend.descriptor_set == VK_NULL_HANDLE);
        LNA_ASSERT(backend.font_texture_ptr == nullptr);
        LNA_ASSERT(backend.vertex_count == 0);
        LNA_ASSERT(backend.index_count == 0);
        LNA_ASSERT(backend.vertex_data_mapped == nullptr);
        LNA_ASSERT(backend.index_data_mapped == nullptr);

        LNA_ASSERT(config.renderer_backend_ptr);
        LNA_ASSERT(config.renderer_backend_ptr->device);
        LNA_ASSERT(config.renderer_backend_ptr->physical_device);
        LNA_ASSERT(config.renderer_backend_ptr->command_pool);
        LNA_ASSERT(config.renderer_backend_ptr->graphics_queue);
        LNA_ASSERT(config.renderer_backend_ptr->render_pass);
        LNA_ASSERT(config.texture_backend_ptr);

        vulkan_renderer_backend_register_swap_chain_callbacks(
            *config.renderer_backend_ptr,
            nullptr,
            nullptr,
            vulkan_imgui_on_draw,
            &backend
            );

        backend.renderer_backend_ptr = config.renderer_backend_ptr;

        //! STYLE

        ImGui::StyleColorsDark();

        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(
            config.window_width,
            config.window_height
            );
        io.DisplayFramebufferScale = ImVec2(
            1.0f,
            1.0f
            );

        //! TEXTURE

        unsigned char* font_data;
        int texture_width;
        int texture_height;
        io.Fonts->GetTexDataAsRGBA32(
            &font_data,
            &texture_width,
            &texture_height
            );
        texture_config font_texture_config{};
        font_texture_config.pixels      = font_data;
        font_texture_config.width       = static_cast<uint32_t>(texture_width);
        font_texture_config.height      = static_cast<uint32_t>(texture_height);
        font_texture_config.fmt         = texture_config::format::R8G8B8A8_UNORM;
        font_texture_config.mag         = texture_config::filter::LINEAR;
        font_texture_config.min         = texture_config::filter::LINEAR;
        font_texture_config.mipmap      = texture_config::mipmap_mode::LINEAR;
        font_texture_config.u           = texture_config::sampler_address_mode::CLAMP_TO_EDGE;
        font_texture_config.v           = texture_config::sampler_address_mode::CLAMP_TO_EDGE;
        font_texture_config.w           = texture_config::sampler_address_mode::CLAMP_TO_EDGE;
        backend.font_texture_ptr        = texture_backend_new_texture(
            *config.texture_backend_ptr,
            font_texture_config
            );

        //! DESCRIPTORS

        VkDescriptorPoolSize descriptor_pool_sizes[1]{};
        descriptor_pool_sizes[0].type               = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_pool_sizes[0].descriptorCount    = 1;
        VkDescriptorPoolCreateInfo pool_create_info{};
        pool_create_info.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_create_info.poolSizeCount              = 1;
        pool_create_info.pPoolSizes                 = descriptor_pool_sizes;
        pool_create_info.maxSets                    = 2;
        VULKAN_CHECK_RESULT(
            vkCreateDescriptorPool(
                config.renderer_backend_ptr->device,
                &pool_create_info,
                nullptr,
                &backend.descriptor_pool
                )
        )

        VkDescriptorSetLayoutBinding set_layout_bindings[1]{};
        set_layout_bindings[0].descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        set_layout_bindings[0].stageFlags       = VK_SHADER_STAGE_FRAGMENT_BIT;
        set_layout_bindings[0].binding          = 0;
        set_layout_bindings[0].descriptorCount  = 1;
        VkDescriptorSetLayoutCreateInfo set_layout_create_info{};
        set_layout_create_info.sType            = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        set_layout_create_info.pBindings        = set_layout_bindings;
        set_layout_create_info.bindingCount     = 1;
        VULKAN_CHECK_RESULT(
            vkCreateDescriptorSetLayout(
                config.renderer_backend_ptr->device,
                &set_layout_create_info,
                nullptr,
                &backend.descriptor_set_layout
                )
            )

        VkDescriptorSetAllocateInfo set_allocate_info{};
        set_allocate_info.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        set_allocate_info.descriptorPool        = backend.descriptor_pool;
        set_allocate_info.pSetLayouts           = &backend.descriptor_set_layout;
        set_allocate_info.descriptorSetCount    = 1;
        VULKAN_CHECK_RESULT(
            vkAllocateDescriptorSets(
                config.renderer_backend_ptr->device,
                &set_allocate_info,
                &backend.descriptor_set
                )
            )
        VkDescriptorImageInfo descriptor_image_info{};
        descriptor_image_info.sampler               = backend.font_texture_ptr->sampler;
        descriptor_image_info.imageView             = backend.font_texture_ptr->image_view;
        descriptor_image_info.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkWriteDescriptorSet write_descriptor_sets[1]{};
        write_descriptor_sets[0].sType              = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_sets[0].dstSet             = backend.descriptor_set;
        write_descriptor_sets[0].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write_descriptor_sets[0].dstBinding         = 0;
        write_descriptor_sets[0].pImageInfo         = &descriptor_image_info;
        write_descriptor_sets[0].descriptorCount    = 1;
        vkUpdateDescriptorSets(
            config.renderer_backend_ptr->device,
            1,
            write_descriptor_sets,
            0,
            nullptr
            );

        //! PIPELINE

        VkPipelineCacheCreateInfo pipeline_cache_create_info{};
        pipeline_cache_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        VULKAN_CHECK_RESULT(
            vkCreatePipelineCache(
                config.renderer_backend_ptr->device,
                &pipeline_cache_create_info,
                nullptr,
                &backend.pipeline_cache
                )
            )

        VkPushConstantRange push_constant_range{};
        push_constant_range.stageFlags              = VK_SHADER_STAGE_VERTEX_BIT;
        push_constant_range.size                    = sizeof(vulkan_imgui_push_const_block);
        push_constant_range.offset                  = 0;
        VkPipelineLayoutCreateInfo layout_create_info{};
        layout_create_info.sType                    = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layout_create_info.setLayoutCount           = 1;
        layout_create_info.pSetLayouts              = &backend.descriptor_set_layout;
        layout_create_info.pushConstantRangeCount   = 1;
        layout_create_info.pPushConstantRanges      = &push_constant_range;
        VULKAN_CHECK_RESULT(
            vkCreatePipelineLayout(
                config.renderer_backend_ptr->device,
                &layout_create_info,
                nullptr,
                &backend.pipeline_layout
                )
            )

        VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info{};
        input_assembly_state_create_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly_state_create_info.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly_state_create_info.flags                  = 0;
        input_assembly_state_create_info.primitiveRestartEnable = VK_FALSE;
        VkPipelineRasterizationStateCreateInfo rasterization_state_create_info{};
        rasterization_state_create_info.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterization_state_create_info.polygonMode             = VK_POLYGON_MODE_FILL;
        rasterization_state_create_info.cullMode                = VK_CULL_MODE_NONE;
        rasterization_state_create_info.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterization_state_create_info.flags                   = 0;
        rasterization_state_create_info.depthClampEnable        = VK_FALSE;
        rasterization_state_create_info.lineWidth               = 1.0f;
        VkPipelineColorBlendAttachmentState blend_attachment_state{};
        blend_attachment_state.blendEnable                      = VK_TRUE;
        blend_attachment_state.colorWriteMask                   = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blend_attachment_state.srcColorBlendFactor              = VK_BLEND_FACTOR_SRC_ALPHA;
		blend_attachment_state.dstColorBlendFactor              = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blend_attachment_state.colorBlendOp                     = VK_BLEND_OP_ADD;
		blend_attachment_state.srcAlphaBlendFactor              = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blend_attachment_state.dstAlphaBlendFactor              = VK_BLEND_FACTOR_ZERO;
		blend_attachment_state.alphaBlendOp                     = VK_BLEND_OP_ADD;
        VkPipelineColorBlendStateCreateInfo color_blend_state_create_info{};
        color_blend_state_create_info.sType                     = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend_state_create_info.attachmentCount           = 1;
        color_blend_state_create_info.pAttachments              = &blend_attachment_state;
        VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info{};
        depth_stencil_state_create_info.sType                   = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil_state_create_info.depthTestEnable         = VK_FALSE;
		depth_stencil_state_create_info.depthWriteEnable        = VK_FALSE;
		depth_stencil_state_create_info.depthCompareOp          = VK_COMPARE_OP_LESS_OR_EQUAL;
		depth_stencil_state_create_info.back.compareOp          = VK_COMPARE_OP_ALWAYS;
        VkPipelineViewportStateCreateInfo viewport_state_create_info{};
        viewport_state_create_info.sType                        = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state_create_info.viewportCount                = 1;
		viewport_state_create_info.scissorCount                 = 1;
		viewport_state_create_info.flags                        = 0;
        VkPipelineMultisampleStateCreateInfo multisample_state_create_info{};
        multisample_state_create_info.sType                     = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_state_create_info.rasterizationSamples      = VK_SAMPLE_COUNT_1_BIT;
		multisample_state_create_info.flags                     = 0;
        VkDynamicState dynamic_states[] =
        {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamic_state_create_info{};
		dynamic_state_create_info.sType                         = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state_create_info.pDynamicStates                = dynamic_states;
		dynamic_state_create_info.dynamicStateCount             = 2;
		dynamic_state_create_info.flags                         = 0;
        VkPipelineShaderStageCreateInfo shader_stage_create_infos[2]{};
        VkGraphicsPipelineCreateInfo pipeline_create_info {};
		pipeline_create_info.sType                              = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_create_info.layout                             = backend.pipeline_layout;
		pipeline_create_info.renderPass                         = config.renderer_backend_ptr->render_pass;
		pipeline_create_info.flags                              = 0;
		pipeline_create_info.basePipelineIndex                  = -1;
		pipeline_create_info.basePipelineHandle                 = VK_NULL_HANDLE;
        pipeline_create_info.pInputAssemblyState                = &input_assembly_state_create_info;
		pipeline_create_info.pRasterizationState                = &rasterization_state_create_info;
		pipeline_create_info.pColorBlendState                   = &color_blend_state_create_info;
		pipeline_create_info.pMultisampleState                  = &multisample_state_create_info;
		pipeline_create_info.pViewportState                     = &viewport_state_create_info;
		pipeline_create_info.pDepthStencilState                 = &depth_stencil_state_create_info;
		pipeline_create_info.pDynamicState                      = &dynamic_state_create_info;
		pipeline_create_info.stageCount                         = 2;
		pipeline_create_info.pStages                            = shader_stage_create_infos;
        VkVertexInputBindingDescription vertex_input_binding_descriptions[1]{};
        vertex_input_binding_descriptions[0].binding            = 0;
        vertex_input_binding_descriptions[0].stride             = sizeof(ImDrawVert);
        vertex_input_binding_descriptions[0].inputRate          = VK_VERTEX_INPUT_RATE_VERTEX;
        VkVertexInputAttributeDescription vertex_input_attribute_descriptions[3]{};
        vertex_input_attribute_descriptions[0].binding          = 0;
        vertex_input_attribute_descriptions[0].location         = 0;
        vertex_input_attribute_descriptions[0].format           = VK_FORMAT_R32G32_SFLOAT;
        vertex_input_attribute_descriptions[0].offset           = offsetof(ImDrawVert, pos);
        vertex_input_attribute_descriptions[1].binding          = 0;
        vertex_input_attribute_descriptions[1].location         = 1;
        vertex_input_attribute_descriptions[1].format           = VK_FORMAT_R32G32_SFLOAT;
        vertex_input_attribute_descriptions[1].offset           = offsetof(ImDrawVert, uv);
        vertex_input_attribute_descriptions[2].binding          = 0;
        vertex_input_attribute_descriptions[2].location         = 2;
        vertex_input_attribute_descriptions[2].format           = VK_FORMAT_R8G8B8A8_UNORM;
        vertex_input_attribute_descriptions[2].offset           = offsetof(ImDrawVert, col);
        VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_create_info{};
		pipeline_vertex_input_state_create_info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        pipeline_vertex_input_state_create_info.vertexBindingDescriptionCount   = 1;
		pipeline_vertex_input_state_create_info.pVertexBindingDescriptions      = vertex_input_binding_descriptions;
		pipeline_vertex_input_state_create_info.vertexAttributeDescriptionCount = 3;
		pipeline_vertex_input_state_create_info.pVertexAttributeDescriptions    = vertex_input_attribute_descriptions;
        pipeline_create_info.pVertexInputState                                  = &pipeline_vertex_input_state_create_info;


        VkShaderModule vertex_shader_module = lna::vulkan_helpers::load_shader(
            config.renderer_backend_ptr->device,
            "shaders/imgui_vert.spv",
            config.renderer_backend_ptr->memory_pools[backend_renderer::FRAME_LIFETIME_MEMORY_POOL]
            );
        VkShaderModule fragment_shader_module = lna::vulkan_helpers::load_shader(
            config.renderer_backend_ptr->device,
            "shaders/imgui_frag.spv",
            config.renderer_backend_ptr->memory_pools[backend_renderer::FRAME_LIFETIME_MEMORY_POOL]
            );
        shader_stage_create_infos[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stage_create_infos[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
        shader_stage_create_infos[0].module = vertex_shader_module;
        shader_stage_create_infos[0].pName  = "main";
        shader_stage_create_infos[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stage_create_infos[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
        shader_stage_create_infos[1].module = fragment_shader_module;
        shader_stage_create_infos[1].pName  = "main";

        VULKAN_CHECK_RESULT(
            vkCreateGraphicsPipelines(
                config.renderer_backend_ptr->device,
                backend.pipeline_cache,
                1,
                &pipeline_create_info,
                nullptr,
                &backend.pipeline
                )
            )

        vkDestroyShaderModule(config.renderer_backend_ptr->device, fragment_shader_module, nullptr);
        vkDestroyShaderModule(config.renderer_backend_ptr->device, vertex_shader_module, nullptr);
    }

    void imgui_backend_release(
        imgui_backend& backend
        )
    {
        ImGui::DestroyContext();

        LNA_ASSERT(backend.renderer_backend_ptr);
        LNA_ASSERT(backend.renderer_backend_ptr->device);

        if (backend.vertex_buffer)        vkDestroyBuffer(backend.renderer_backend_ptr->device, backend.vertex_buffer, nullptr);
        if (backend.vertex_buffer_memory) vkFreeMemory(backend.renderer_backend_ptr->device, backend.vertex_buffer_memory, nullptr);
        if (backend.index_buffer)         vkDestroyBuffer(backend.renderer_backend_ptr->device, backend.index_buffer, nullptr);
        if (backend.index_buffer_memory)  vkFreeMemory(backend.renderer_backend_ptr->device, backend.index_buffer_memory, nullptr);

        vkDestroyPipelineCache(backend.renderer_backend_ptr->device, backend.pipeline_cache, nullptr);
        vkDestroyPipeline(backend.renderer_backend_ptr->device, backend.pipeline, nullptr);
        vkDestroyPipelineLayout(backend.renderer_backend_ptr->device, backend.pipeline_layout, nullptr);
        vkDestroyDescriptorPool(backend.renderer_backend_ptr->device, backend.descriptor_pool, nullptr);
        vkDestroyDescriptorSetLayout(backend.renderer_backend_ptr->device, backend.descriptor_set_layout, nullptr);
    }
}
