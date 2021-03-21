#include <cstring>
#include <algorithm>
#include "platform/vulkan/vulkan_imgui.hpp"
#include "platform/vulkan/vulkan_helpers.hpp"
#include "core/assert.hpp"
#include "core/memory_pool.hpp"

namespace lna
{
    void vulkan_imgui_wrapper_init(
        vulkan_imgui_wrapper& imgui_wrapper
        )
    {
        imgui_wrapper.vertex_buffer             = VK_NULL_HANDLE;
        imgui_wrapper.vertex_buffer_memory      = VK_NULL_HANDLE;
        imgui_wrapper.index_buffer              = VK_NULL_HANDLE;
        imgui_wrapper.index_buffer_memory       = VK_NULL_HANDLE;
        imgui_wrapper.pipeline_cache            = VK_NULL_HANDLE;
        imgui_wrapper.pipeline_layout           = VK_NULL_HANDLE;
        imgui_wrapper.pipeline                  = VK_NULL_HANDLE;
        imgui_wrapper.descriptor_pool           = VK_NULL_HANDLE;
        imgui_wrapper.descriptor_set_layout     = VK_NULL_HANDLE;
        imgui_wrapper.descriptor_set            = VK_NULL_HANDLE;
        imgui_wrapper.font_texture.image        = nullptr;
        imgui_wrapper.font_texture.image_memory = nullptr;
        imgui_wrapper.font_texture.image_view   = nullptr;
        imgui_wrapper.font_texture.sampler      = nullptr;
        imgui_wrapper.vertex_count              = 0;
        imgui_wrapper.index_count               = 0;
        imgui_wrapper.vertex_data_mapped        = nullptr;
        imgui_wrapper.index_data_mapped         = nullptr;
    }

    void vulkan_imgui_wrapper_configure(
        vulkan_imgui_wrapper& imgui_wrapper,
        vulkan_imgui_wrapper_config& config
        )
    {
        LNA_ASSERT(config.device);
        LNA_ASSERT(config.physical_device);
        LNA_ASSERT(config.command_pool);
        LNA_ASSERT(config.graphics_queue);
        LNA_ASSERT(config.render_pass);
        LNA_ASSERT(config.temp_memory_pool_ptr);

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

        vulkan_texture_config_info texture_config_info{};
        texture_config_info.device          = config.device;
        texture_config_info.physical_device = config.physical_device;
        texture_config_info.filename        = nullptr;
        texture_config_info.pixels          = font_data;
        texture_config_info.width           = static_cast<uint32_t>(texture_width);
        texture_config_info.height          = static_cast<uint32_t>(texture_height);
        texture_config_info.graphics_queue  = config.graphics_queue;
        texture_config_info.command_pool    = config.command_pool;
        texture_config_info.format          = VK_FORMAT_R8G8B8A8_UNORM;
        texture_config_info.mag_filter      = VK_FILTER_LINEAR;
        texture_config_info.min_filter      = VK_FILTER_LINEAR;
        texture_config_info.mipmap_mode     = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        texture_config_info.address_mode_u  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        texture_config_info.address_mode_v  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        texture_config_info.address_mode_w  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        vulkan_texture_configure(
            imgui_wrapper.font_texture,
            texture_config_info
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
                config.device,
                &pool_create_info,
                nullptr,
                &imgui_wrapper.descriptor_pool
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
                config.device,
                &set_layout_create_info,
                nullptr,
                &imgui_wrapper.descriptor_set_layout
                )
            )

        VkDescriptorSetAllocateInfo set_allocate_info{};
        set_allocate_info.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        set_allocate_info.descriptorPool        = imgui_wrapper.descriptor_pool;
        set_allocate_info.pSetLayouts           = &imgui_wrapper.descriptor_set_layout;
        set_allocate_info.descriptorSetCount    = 1;
        VULKAN_CHECK_RESULT(
            vkAllocateDescriptorSets(
                config.device,
                &set_allocate_info,
                &imgui_wrapper.descriptor_set
                )
            )
        VkDescriptorImageInfo descriptor_image_info{};
        descriptor_image_info.sampler               = imgui_wrapper.font_texture.sampler;
        descriptor_image_info.imageView             = imgui_wrapper.font_texture.image_view;
        descriptor_image_info.imageLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkWriteDescriptorSet write_descriptor_sets[1]{};
        write_descriptor_sets[0].sType              = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_sets[0].dstSet             = imgui_wrapper.descriptor_set;
        write_descriptor_sets[0].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write_descriptor_sets[0].dstBinding         = 0;
        write_descriptor_sets[0].pImageInfo         = &descriptor_image_info;
        write_descriptor_sets[0].descriptorCount    = 1;
        vkUpdateDescriptorSets(
            config.device,
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
                config.device,
                &pipeline_cache_create_info,
                nullptr,
                &imgui_wrapper.pipeline_cache
                )
            )

        VkPushConstantRange push_constant_range{};
        push_constant_range.stageFlags              = VK_SHADER_STAGE_VERTEX_BIT;
        push_constant_range.size                    = sizeof(vulkan_imgui_push_const_block);
        push_constant_range.offset                  = 0;
        VkPipelineLayoutCreateInfo layout_create_info{};
        layout_create_info.sType                    = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layout_create_info.setLayoutCount           = 1;
        layout_create_info.pSetLayouts              = &imgui_wrapper.descriptor_set_layout;
        layout_create_info.pushConstantRangeCount   = 1;
        layout_create_info.pPushConstantRanges      = &push_constant_range;
        VULKAN_CHECK_RESULT(
            vkCreatePipelineLayout(
                config.device,
                &layout_create_info,
                nullptr,
                &imgui_wrapper.pipeline_layout
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
		pipeline_create_info.layout                             = imgui_wrapper.pipeline_layout;
		pipeline_create_info.renderPass                         = config.render_pass;
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
            config.device,
            "shaders/imgui_vert.spv",
            *config.temp_memory_pool_ptr
            );
        VkShaderModule fragment_shader_module = lna::vulkan_helpers::load_shader(
            config.device,
            "shaders/imgui_frag.spv",
            *config.temp_memory_pool_ptr
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
                config.device,
                imgui_wrapper.pipeline_cache,
                1,
                &pipeline_create_info,
                nullptr,
                &imgui_wrapper.pipeline
                )
            )

        vkDestroyShaderModule(config.device, fragment_shader_module, nullptr);
        vkDestroyShaderModule(config.device, vertex_shader_module, nullptr);
    }

    void vulkan_imgui_wrapper_update(
        vulkan_imgui_wrapper& imgui_wrapper,
        VkDevice device,
        VkPhysicalDevice physical_device
        )
    {
        LNA_ASSERT(device);
        LNA_ASSERT(physical_device);

        ImDrawData* im_draw_data = ImGui::GetDrawData();

        VkDeviceSize vertex_buffer_size = im_draw_data->TotalVtxCount * sizeof(ImDrawVert);
        VkDeviceSize index_buffer_size = im_draw_data->TotalIdxCount * sizeof (ImDrawIdx);

        if ((vertex_buffer_size == 0) || (index_buffer_size == 0))
        {
            return;
        }

        if ((imgui_wrapper.vertex_buffer == VK_NULL_HANDLE) || (imgui_wrapper.vertex_count != im_draw_data->TotalVtxCount))
        {
            if (imgui_wrapper.vertex_data_mapped)
            {
                vkUnmapMemory(device, imgui_wrapper.vertex_buffer_memory);
                imgui_wrapper.vertex_data_mapped = nullptr;
            }
            if (imgui_wrapper.vertex_buffer)
            {
                vkDestroyBuffer(device, imgui_wrapper.vertex_buffer, nullptr);
                imgui_wrapper.vertex_buffer = nullptr;
            }
            if (imgui_wrapper.vertex_buffer_memory)
            {
                vkFreeMemory(device, imgui_wrapper.vertex_buffer_memory, nullptr);
                imgui_wrapper.vertex_buffer_memory = nullptr;
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
                    &imgui_wrapper.vertex_buffer
                )
            )
            VkMemoryRequirements    memory_requirements;
            vkGetBufferMemoryRequirements(
                device,
                imgui_wrapper.vertex_buffer,
                &memory_requirements
                );
            VkMemoryAllocateInfo    memory_allocate_info{};
            memory_allocate_info.sType              = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            memory_allocate_info.allocationSize     = memory_requirements.size;
            memory_allocate_info.memoryTypeIndex    = vulkan_helpers::find_memory_type(
                physical_device,
                memory_requirements.memoryTypeBits,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                );
            VULKAN_CHECK_RESULT(
                vkAllocateMemory(
                    device,
                    &memory_allocate_info,
                    nullptr,
                    &imgui_wrapper.vertex_buffer_memory
                    )
                )
            VULKAN_CHECK_RESULT(
                vkBindBufferMemory(
                    device,
                    imgui_wrapper.vertex_buffer,
                    imgui_wrapper.vertex_buffer_memory,
                    0
                    )
            )
            imgui_wrapper.vertex_count = im_draw_data->TotalVtxCount;
            VULKAN_CHECK_RESULT(
                vkMapMemory(
                    device,
                    imgui_wrapper.vertex_buffer_memory,
                    0,
                    VK_WHOLE_SIZE,
                    0,
                    &imgui_wrapper.vertex_data_mapped
                    )
                )
        }
        if ((imgui_wrapper.index_buffer == VK_NULL_HANDLE) || (imgui_wrapper.index_count < im_draw_data->TotalIdxCount))
        {
            if (imgui_wrapper.index_data_mapped)
            {
                vkUnmapMemory(device, imgui_wrapper.index_buffer_memory);
                imgui_wrapper.index_data_mapped = nullptr;
            }
            if (imgui_wrapper.index_buffer)
            {
                vkDestroyBuffer(device, imgui_wrapper.index_buffer, nullptr);
                imgui_wrapper.index_buffer = nullptr;
            }
            if (imgui_wrapper.index_buffer_memory)
            {
                vkFreeMemory(device, imgui_wrapper.index_buffer_memory, nullptr);
                imgui_wrapper.index_buffer_memory = nullptr;
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
                    &imgui_wrapper.index_buffer
                )
            )
            VkMemoryRequirements    memory_requirements;
            vkGetBufferMemoryRequirements(
                device,
                imgui_wrapper.index_buffer,
                &memory_requirements
                );
            VkMemoryAllocateInfo    memory_allocate_info{};
            memory_allocate_info.sType              = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            memory_allocate_info.allocationSize     = memory_requirements.size;
            memory_allocate_info.memoryTypeIndex    = vulkan_helpers::find_memory_type(
                physical_device,
                memory_requirements.memoryTypeBits,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                );
            VULKAN_CHECK_RESULT(
                vkAllocateMemory(
                    device,
                    &memory_allocate_info,
                    nullptr,
                    &imgui_wrapper.index_buffer_memory
                    )
                )
            VULKAN_CHECK_RESULT(
                vkBindBufferMemory(
                    device,
                    imgui_wrapper.index_buffer,
                    imgui_wrapper.index_buffer_memory,
                    0
                    )
            )
            imgui_wrapper.index_count = im_draw_data->TotalIdxCount;
            VULKAN_CHECK_RESULT(
                vkMapMemory(
                    device,
                    imgui_wrapper.index_buffer_memory,
                    0,
                    VK_WHOLE_SIZE,
                    0,
                    &imgui_wrapper.index_data_mapped
                    )
                )
        }

        ImDrawVert* vertex_dst  = (ImDrawVert*)imgui_wrapper.vertex_data_mapped;
        ImDrawIdx*  index_dst   = (ImDrawIdx*)imgui_wrapper.index_data_mapped;
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
            mapped_memory_range.memory  = imgui_wrapper.vertex_buffer_memory;
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
            mapped_memory_range.memory  = imgui_wrapper.index_buffer_memory;
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

    void vulkan_imgui_wrapper_render_frame(
        vulkan_imgui_wrapper& imgui_wrapper,
        VkCommandBuffer command_buffer
        )
    {
        LNA_ASSERT(command_buffer);

        ImGuiIO& io = ImGui::GetIO();

        vkCmdBindDescriptorSets(
            command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            imgui_wrapper.pipeline_layout,
            0,
            1,
            &imgui_wrapper.descriptor_set,
            0,
            nullptr
            );
        vkCmdBindPipeline(
            command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            imgui_wrapper.pipeline
            );

        VkViewport viewport;
        viewport.width      = io.DisplaySize.x;
		viewport.height     = io.DisplaySize.y;
		viewport.minDepth   = 0.0f;
		viewport.maxDepth   = 1.0f;
        vkCmdSetViewport(
            command_buffer,
            0,
            1,
            &viewport
            );

        imgui_wrapper.push_const_block.scale.x = 2.0f / io.DisplaySize.x;
        imgui_wrapper.push_const_block.scale.y = 2.0f / io.DisplaySize.y;
        imgui_wrapper.push_const_block.translate.x = -1.0f;
        imgui_wrapper.push_const_block.translate.y = -1.0f;

        vkCmdPushConstants(
            command_buffer,
            imgui_wrapper.pipeline_layout,
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(vulkan_imgui_push_const_block),
            &imgui_wrapper.push_const_block
            );

        ImDrawData* im_draw_data    = ImGui::GetDrawData();
        int         vertex_offset   = 0;
        int         index_offset    = 0;
        if (im_draw_data->CmdListsCount > 0)
        {
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(
                command_buffer,
                0,
                1,
                &imgui_wrapper.vertex_buffer,
                offsets
                );
            vkCmdBindIndexBuffer(
                command_buffer,
                imgui_wrapper.index_buffer,
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
                    scissor_rect.offset.x = std::max((int32_t)(cmd->ClipRect.x), 0);
                    scissor_rect.offset.y = std::max((int32_t)(cmd->ClipRect.y), 0);
                    scissor_rect.extent.width   = (uint32_t)(cmd->ClipRect.z - cmd->ClipRect.x);
                    scissor_rect.extent.height  = (uint32_t)(cmd->ClipRect.w - cmd->ClipRect.y);
                    vkCmdSetScissor(
                        command_buffer,
                        0,
                        1,
                        &scissor_rect
                        );
                    vkCmdDrawIndexed(
                        command_buffer,
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

    void vulkan_imgui_wrapper_release(
        vulkan_imgui_wrapper& imgui_wrapper,
        VkDevice device
        )
    {
        ImGui::DestroyContext();

        if (imgui_wrapper.vertex_buffer)        vkDestroyBuffer(device, imgui_wrapper.vertex_buffer, nullptr);
        if (imgui_wrapper.vertex_buffer_memory) vkFreeMemory(device, imgui_wrapper.vertex_buffer_memory, nullptr);
        if (imgui_wrapper.index_buffer)         vkDestroyBuffer(device, imgui_wrapper.index_buffer, nullptr);
        if (imgui_wrapper.index_buffer_memory)  vkFreeMemory(device, imgui_wrapper.index_buffer_memory, nullptr);

        vulkan_texture_release(imgui_wrapper.font_texture, device);
        vkDestroyPipelineCache(device, imgui_wrapper.pipeline_cache, nullptr);
        vkDestroyPipeline(device, imgui_wrapper.pipeline, nullptr);
        vkDestroyPipelineLayout(device, imgui_wrapper.pipeline_layout, nullptr);
        vkDestroyDescriptorPool(device, imgui_wrapper.descriptor_pool, nullptr);
        vkDestroyDescriptorSetLayout(device, imgui_wrapper.descriptor_set_layout, nullptr);

        vulkan_imgui_wrapper_init(imgui_wrapper);
    }
}
