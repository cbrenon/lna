#include <cstring>
#include "core/assert.hpp"
#include "platform/vulkan/vulkan_mesh.hpp"
#include "platform/vulkan/vulkan_helpers.hpp"
#include "platform/vulkan/vulkan_texture.hpp"

namespace
{
}

void lna::vulkan_mesh_init(
    lna::vulkan_mesh& mesh
    )
{
    mesh.vertex_buffer          = nullptr;
    mesh.vertex_buffer_memory   = nullptr;
    mesh.index_buffer           = nullptr;
    mesh.index_buffer_memory    = nullptr;
    mesh.vertex_count           = 0;
    mesh.index_count            = 0;

    lna::heap_array_init(mesh.uniform_buffers);
    lna::heap_array_init(mesh.uniform_buffers_memory);
    lna::heap_array_init(mesh.descriptor_sets);
}

void lna::vulkan_mesh_create_vertex_and_index_buffer(
    lna::vulkan_mesh& mesh,
    lna::vulkan_mesh_create_vertex_and_index_info& config
    )
{
    LNA_ASSERT(mesh.vertex_buffer == nullptr);
    LNA_ASSERT(mesh.vertex_buffer_memory == nullptr);
    LNA_ASSERT(mesh.index_buffer == nullptr);
    LNA_ASSERT(mesh.index_buffer_memory == nullptr);
    LNA_ASSERT(mesh.vertex_count == 0);
    LNA_ASSERT(mesh.index_count == 0);
    LNA_ASSERT(config.device);
    LNA_ASSERT(config.physical_device);
    LNA_ASSERT(config.command_pool);
    LNA_ASSERT(config.graphics_queue);
    LNA_ASSERT(config.vertices);
    LNA_ASSERT(config.indices);
    LNA_ASSERT(config.vertex_count > 0);
    LNA_ASSERT(config.index_count > 0);

    
    //! VERTEX BUFFER PART
    {
        mesh.vertex_count  = config.vertex_count;
        auto vertex_buffer_size = sizeof(config.vertices[0]) * mesh.vertex_count;
        VkBuffer        staging_buffer;
        VkDeviceMemory  staging_buffer_memory;
        lna::vulkan_helpers::create_buffer(
            config.device,
            config.physical_device,
            vertex_buffer_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            staging_buffer,
            staging_buffer_memory
            );
        void* vertices_data;
        VULKAN_CHECK_RESULT(
            vkMapMemory(
                config.device,
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
            config.device,
            staging_buffer_memory
            );
        lna::vulkan_helpers::create_buffer(
            config.device,
            config.physical_device,
            vertex_buffer_size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            mesh.vertex_buffer,
            mesh.vertex_buffer_memory
            );
        lna::vulkan_helpers::copy_buffer(
            config.device,
            config.command_pool,
            config.graphics_queue,
            staging_buffer,
            mesh.vertex_buffer,
            vertex_buffer_size
            );
        vkDestroyBuffer(
            config.device,
            staging_buffer,
            nullptr
            );
        vkFreeMemory(
            config.device,
            staging_buffer_memory,
            nullptr
            );
    }

    //! INDEX BUFFER PART
    {
        mesh.index_count   = config.index_count;
        auto index_buffer_size = sizeof(config.indices[0]) * mesh.index_count;
        VkBuffer        staging_buffer;
        VkDeviceMemory  staging_buffer_memory;
        lna::vulkan_helpers::create_buffer(
            config.device,
            config.physical_device,
            index_buffer_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            staging_buffer,
            staging_buffer_memory
            );
        void* indices_data;
        VULKAN_CHECK_RESULT(
            vkMapMemory(
                config.device,
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
            config.device,
            staging_buffer_memory
            );
        lna::vulkan_helpers::create_buffer(
            config.device,
            config.physical_device,
            index_buffer_size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            mesh.index_buffer,
            mesh.index_buffer_memory
            );
        lna::vulkan_helpers::copy_buffer(
            config.device,
            config.command_pool,
            config.graphics_queue,
            staging_buffer,
            mesh.index_buffer,
            index_buffer_size
            );
        vkDestroyBuffer(
            config.device,
            staging_buffer,
            nullptr
            );
        vkFreeMemory(
            config.device,
            staging_buffer_memory,
            nullptr
            );
    }
}

void lna::vulkan_mesh_create_uniform_buffer(
    lna::vulkan_mesh& mesh,
    lna::vulkan_mesh_create_uniform_buffer_info& config
    )
{
    LNA_ASSERT(lna::heap_array_has_been_reset(mesh.uniform_buffers));
    LNA_ASSERT(lna::heap_array_has_been_reset(mesh.uniform_buffers_memory));
    LNA_ASSERT(config.device);
    LNA_ASSERT(config.physical_device);
    LNA_ASSERT(config.swap_chain_image_count > 0);
    LNA_ASSERT(config.swap_chain_memory_pool_ptr);

    VkDeviceSize uniform_buffer_size = sizeof(vulkan_uniform_buffer_object);

    lna::heap_array_set_max_element_count(
        mesh.uniform_buffers,
        *config.swap_chain_memory_pool_ptr,
        config.swap_chain_image_count
        );
    lna::heap_array_set_max_element_count(
        mesh.uniform_buffers_memory,
        *config.swap_chain_memory_pool_ptr,
        config.swap_chain_image_count
        );
    for (size_t i = 0; i < config.swap_chain_image_count; ++i)
    {
        lna::vulkan_helpers::create_buffer(
            config.device,
            config.physical_device,
            uniform_buffer_size,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            mesh.uniform_buffers.elements[i],
            mesh.uniform_buffers_memory.elements[i]
            );
    }
}

void lna::vulkan_mesh_upate_uniform_buffer(
    lna::vulkan_mesh& mesh,
    lna::vulkan_mesh_update_uniform_buffer_info& config
    )
{
    LNA_ASSERT(mesh.uniform_buffers_memory.elements);
    LNA_ASSERT(config.device);
    LNA_ASSERT(config.image_index < mesh.uniform_buffers_memory.element_count);
    // TODO: to uncomment when config will contain mvp matrices pointers
    //LNA_ASSERT(config.model_ptr);
    //LNA_ASSERT(config.view_ptr);
    //LNA_ASSERT(config.projection_ptr);

    vulkan_uniform_buffer_object ubo{};


    // TODO: to remove when config will contain mvp matrices pointers
    const lna::vec3 eye     = { 0.0f, 0.0f, 2.0f };
    const lna::vec3 target  = { 0.0f, 0.0f, 0.0f };
    const lna::vec3 up      = { 0.0f, -1.0f, 0.0f };
    const float     fov     = 45.0f;
    const float     aspect  = static_cast<float>(config.swap_chain_extent.width) / static_cast<float>(config.swap_chain_extent.height);
    const float     near    = 1.0f;
    const float     far     = 10.0f;
    lna::mat4_identity(
        ubo.model
        );
    lna::mat4_loot_at(
        ubo.view,
        eye,
        target,
        up
        );
    lna::mat4_perspective(
        ubo.projection,
        fov,
        aspect,
        near,
        far
        );

    // TODO: to uncomment when config will contain mvp matrices pointers
    // ubo.model        = *config->model_ptr;
    // ubo.view         = *config->view_ptr;
    // ubo.projection   = *config->projection_ptr;

    void* data;
    VULKAN_CHECK_RESULT(
        vkMapMemory(
            config.device,
            mesh.uniform_buffers_memory.elements[config.image_index],
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
        config.device,
        mesh.uniform_buffers_memory.elements[config.image_index]
        );
}

void lna::vulkan_mesh_create_descriptor_sets(
    lna::vulkan_mesh& mesh,
    lna::vulkan_mesh_create_descriptor_sets_info& config
    )
{
    LNA_ASSERT(lna::heap_array_has_been_reset(mesh.descriptor_sets));

    LNA_ASSERT(config.device);
    LNA_ASSERT(config.physical_device);
    LNA_ASSERT(config.descriptor_pool);
    LNA_ASSERT(config.descriptor_set_layout);
    LNA_ASSERT(config.swap_chain_image_count > 0);
    LNA_ASSERT(config.temp_memory_pool_ptr);
    LNA_ASSERT(config.swap_chain_memory_pool_ptr);
    LNA_ASSERT(config.texture_ptr);
    LNA_ASSERT(config.texture_ptr->image_view);
    LNA_ASSERT(config.texture_ptr->sampler);

    lna::heap_array<VkDescriptorSetLayout> layouts;
    lna::heap_array_init(
        layouts
        );
    lna::heap_array_set_max_element_count(
        layouts,
        *config.temp_memory_pool_ptr,
        config.swap_chain_image_count
        );
    lna::heap_array_fill_with_unique_value(
        layouts,
        config.descriptor_set_layout
        );

    VkDescriptorSetAllocateInfo allocate_info{};
    allocate_info.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.descriptorPool        = config.descriptor_pool;
    allocate_info.descriptorSetCount    = config.swap_chain_image_count;
    allocate_info.pSetLayouts           = layouts.elements;

    lna::heap_array_set_max_element_count(
        mesh.descriptor_sets,
        *config.swap_chain_memory_pool_ptr,
        config.swap_chain_image_count
        );
        
    VULKAN_CHECK_RESULT(
        vkAllocateDescriptorSets(
            config.device,
            &allocate_info,
            mesh.descriptor_sets.elements
            )
        )

    for (size_t i = 0; i < config.swap_chain_image_count; ++i)
    {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer  = mesh.uniform_buffers.elements[i];
        buffer_info.offset  = 0;
        buffer_info.range   = sizeof(vulkan_uniform_buffer_object);

        VkDescriptorImageInfo image_info{};
        image_info.imageLayout  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView    = config.texture_ptr->image_view;
        image_info.sampler      = config.texture_ptr->sampler;

        VkWriteDescriptorSet write_descriptors[2] {};
        write_descriptors[0].sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptors[0].dstSet           = mesh.descriptor_sets.elements[i];
        write_descriptors[0].dstBinding       = 0;
        write_descriptors[0].dstArrayElement  = 0;
        write_descriptors[0].descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write_descriptors[0].descriptorCount  = 1;
        write_descriptors[0].pBufferInfo      = &buffer_info;
        write_descriptors[0].pImageInfo       = nullptr;
        write_descriptors[0].pTexelBufferView = nullptr;
        write_descriptors[1].sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptors[1].dstSet           = mesh.descriptor_sets.elements[i];
        write_descriptors[1].dstBinding       = 1;
        write_descriptors[1].dstArrayElement  = 0;
        write_descriptors[1].descriptorType   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write_descriptors[1].descriptorCount  = 1;
        write_descriptors[1].pBufferInfo      = nullptr;
        write_descriptors[1].pImageInfo       = &image_info;
        write_descriptors[1].pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(
            config.device,
            static_cast<uint32_t>(sizeof(write_descriptors) / sizeof(write_descriptors[0])),
            write_descriptors,
            0,
            nullptr
            );
    }
}

void lna::vulkan_mesh_clean_uniform_buffer(
    lna::vulkan_mesh& mesh,
    VkDevice device
    )
{
    LNA_ASSERT(device);

    for (size_t i = 0; i < mesh.uniform_buffers.element_count; ++i)
    {
        vkDestroyBuffer(
            device,
            mesh.uniform_buffers.elements[i],
            nullptr
            );
        vkFreeMemory(
            device,
            mesh.uniform_buffers_memory.elements[i],
            nullptr
            );
    }
    lna::heap_array_reset(mesh.uniform_buffers);
}

void lna::vulkan_mesh_clean_descriptor_sets(
    lna::vulkan_mesh& mesh
    )
{
    // NOTE: no need to explicity clean up vulkan descriptor sets objects
    // because it is done when the vulkan descriptor pool is destroyed.
    // we just have to reset the array and wait for filling it again.
    lna::heap_array_reset(mesh.descriptor_sets);
}

void lna::vulkan_mesh_release(
    lna::vulkan_mesh& mesh,
    VkDevice device
    )
{
    LNA_ASSERT(mesh.vertex_buffer);
    LNA_ASSERT(mesh.vertex_buffer_memory);
    LNA_ASSERT(mesh.index_buffer);
    LNA_ASSERT(mesh.index_buffer_memory);
    LNA_ASSERT(device);

    lna::vulkan_mesh_clean_uniform_buffer(
        mesh,
        device
        );

    lna::vulkan_mesh_clean_descriptor_sets(
        mesh
        );

    vkDestroyBuffer(
        device,
        mesh.index_buffer,
        nullptr
        );
    vkFreeMemory(
        device,
        mesh.index_buffer_memory,
        nullptr
        );
    vkDestroyBuffer(
        device,
        mesh.vertex_buffer,
        nullptr
        );
    vkFreeMemory(
        device,
        mesh.vertex_buffer_memory,
        nullptr
        );
    
    lna::vulkan_mesh_init(
        mesh
        );
}
