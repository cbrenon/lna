#include <cstring>
#include "core/assert.hpp"
#include "core/memory_pool.hpp"
#include "platform/vulkan/vulkan_mesh.hpp"
#include "platform/vulkan/vulkan_helpers.hpp"
#include "platform/vulkan/vulkan_texture.hpp"

namespace lna
{
    struct vulkan_uniform_buffer_object
    {
        alignas(16) lna::mat4       model;
        alignas(16) lna::mat4       view;
        alignas(16) lna::mat4       projection;
    };
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
    LNA_ASSERT(mesh.uniform_buffers == nullptr);
    LNA_ASSERT(mesh.uniform_buffers_memory == nullptr);
    LNA_ASSERT(mesh.swap_chain_image_count == 0);
    LNA_ASSERT(config.device);
    LNA_ASSERT(config.physical_device);
    LNA_ASSERT(config.swap_chain_image_count > 0);
    LNA_ASSERT(config.swap_chain_memory_pool_ptr);

    VkDeviceSize uniform_buffer_size = sizeof(lna::vulkan_uniform_buffer_object);

    mesh.swap_chain_image_count = config.swap_chain_image_count;
    mesh.uniform_buffers        = (VkBuffer*)lna::memory_pool_reserve(
        *config.swap_chain_memory_pool_ptr,
        mesh.swap_chain_image_count * sizeof(VkBuffer)
        );
    mesh.uniform_buffers_memory = (VkDeviceMemory*)lna::memory_pool_reserve(
        *config.swap_chain_memory_pool_ptr,
        mesh.swap_chain_image_count * sizeof(VkDeviceMemory)
        );
    LNA_ASSERT(mesh.uniform_buffers);
    LNA_ASSERT(mesh.uniform_buffers_memory);

    for (size_t i = 0; i < config.swap_chain_image_count; ++i)
    {
        lna::vulkan_helpers::create_buffer(
            config.device,
            config.physical_device,
            uniform_buffer_size,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            mesh.uniform_buffers[i],
            mesh.uniform_buffers_memory[i]
            );
    }
}

void lna::vulkan_mesh_upate_uniform_buffer(
    lna::vulkan_mesh& mesh,
    lna::vulkan_mesh_update_uniform_buffer_info& config
    )
{
    LNA_ASSERT(mesh.uniform_buffers_memory);
    LNA_ASSERT(config.device);
    LNA_ASSERT(config.image_index < mesh.swap_chain_image_count);
    LNA_ASSERT(config.model_matrix_ptr);
    LNA_ASSERT(config.view_matrix_ptr);
    LNA_ASSERT(config.projection_matrix_ptr);

    lna::vulkan_uniform_buffer_object ubo{};

    ubo.model       = *config.model_matrix_ptr;
    ubo.view        = *config.view_matrix_ptr;
    ubo.projection  = *config.projection_matrix_ptr;

    void* data;
    VULKAN_CHECK_RESULT(
        vkMapMemory(
            config.device,
            mesh.uniform_buffers_memory[config.image_index],
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
        mesh.uniform_buffers_memory[config.image_index]
        );
}

void lna::vulkan_mesh_create_descriptor_sets(
    lna::vulkan_mesh& mesh,
    lna::vulkan_mesh_create_descriptor_sets_info& config
    )
{
    LNA_ASSERT(mesh.descriptor_sets == nullptr);
    LNA_ASSERT(mesh.swap_chain_image_count != 0 );  // vulkan_mesh_upate_uniform_buffer must have been called before
    LNA_ASSERT(mesh.uniform_buffers_memory);        // vulkan_mesh_upate_uniform_buffer must have been called before
    LNA_ASSERT(config.device);
    LNA_ASSERT(config.physical_device);
    LNA_ASSERT(config.descriptor_pool);
    LNA_ASSERT(config.descriptor_set_layout);
    LNA_ASSERT(config.temp_memory_pool_ptr);
    LNA_ASSERT(config.swap_chain_memory_pool_ptr);
    LNA_ASSERT(mesh.texture_ptr);                   // texture must be assigned before calling create descriptor sets
    LNA_ASSERT(mesh.texture_ptr->image_view);
    LNA_ASSERT(mesh.texture_ptr->sampler);

    VkDescriptorSetLayout* layouts = (VkDescriptorSetLayout*)lna::memory_pool_reserve(
        *config.temp_memory_pool_ptr,
        mesh.swap_chain_image_count * sizeof(VkDescriptorSetLayout)
        );
    LNA_ASSERT(layouts);
    for (uint32_t i = 0; i < mesh.swap_chain_image_count; ++i)
    {
        layouts[i] = config.descriptor_set_layout;
    }

    VkDescriptorSetAllocateInfo allocate_info{};
    allocate_info.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.descriptorPool        = config.descriptor_pool;
    allocate_info.descriptorSetCount    = mesh.swap_chain_image_count;
    allocate_info.pSetLayouts           = layouts;

    mesh.descriptor_sets = (VkDescriptorSet*)lna::memory_pool_reserve(
        *config.swap_chain_memory_pool_ptr,
        mesh.swap_chain_image_count * sizeof(VkDescriptorSet)
        );
    LNA_ASSERT(mesh.descriptor_sets);
        
    VULKAN_CHECK_RESULT(
        vkAllocateDescriptorSets(
            config.device,
            &allocate_info,
            mesh.descriptor_sets
            )
        )

    for (size_t i = 0; i < mesh.swap_chain_image_count; ++i)
    {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer  = mesh.uniform_buffers[i];
        buffer_info.offset  = 0;
        buffer_info.range   = sizeof(vulkan_uniform_buffer_object);

        VkDescriptorImageInfo image_info{};
        image_info.imageLayout  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView    = mesh.texture_ptr->image_view;
        image_info.sampler      = mesh.texture_ptr->sampler;

        VkWriteDescriptorSet write_descriptors[2] {};
        write_descriptors[0].sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptors[0].dstSet           = mesh.descriptor_sets[i];
        write_descriptors[0].dstBinding       = 0;
        write_descriptors[0].dstArrayElement  = 0;
        write_descriptors[0].descriptorType   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write_descriptors[0].descriptorCount  = 1;
        write_descriptors[0].pBufferInfo      = &buffer_info;
        write_descriptors[0].pImageInfo       = nullptr;
        write_descriptors[0].pTexelBufferView = nullptr;
        write_descriptors[1].sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptors[1].dstSet           = mesh.descriptor_sets[i];
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

    for (size_t i = 0; i < mesh.swap_chain_image_count; ++i)
    {
        vkDestroyBuffer(
            device,
            mesh.uniform_buffers[i],
            nullptr
            );
        vkFreeMemory(
            device,
            mesh.uniform_buffers_memory[i],
            nullptr
            );
    }
    mesh.uniform_buffers = nullptr;
    mesh.uniform_buffers_memory = nullptr;
    mesh.swap_chain_image_count = 0;
}

void lna::vulkan_mesh_clean_descriptor_sets(
    lna::vulkan_mesh& mesh
    )
{
    // NOTE: no need to explicity clean up vulkan descriptor sets objects
    // because it is done when the vulkan descriptor pool is destroyed.
    // we just have to reset the array and wait for filling it again.
    mesh.descriptor_sets = nullptr;
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
    
    mesh.vertex_buffer          = nullptr;
    mesh.vertex_buffer_memory   = nullptr;
    mesh.index_buffer           = nullptr;
    mesh.index_buffer_memory    = nullptr;
    mesh.vertex_count           = 0;
    mesh.index_count            = 0;
    mesh.uniform_buffers        = nullptr;
    mesh.uniform_buffers_memory = nullptr;
    mesh.descriptor_sets        = nullptr;
    mesh.swap_chain_image_count = 0;
}

lna::vulkan_mesh_vertex_description lna::vulkan_default_mesh_vertex_description()
{
    lna::vulkan_mesh_vertex_description result;
    result.bindings[0].binding      = 0;
    result.bindings[0].stride       = sizeof(lna::vertex);
    result.bindings[0].inputRate    = VK_VERTEX_INPUT_RATE_VERTEX;
    result.attributes[0].binding    = 0;
    result.attributes[0].location   = 0;
    result.attributes[0].format     = VK_FORMAT_R32G32B32_SFLOAT;
    result.attributes[0].offset     = offsetof(lna::vertex, position);
    result.attributes[1].binding    = 0;
    result.attributes[1].location   = 1;
    result.attributes[1].format     = VK_FORMAT_R32G32B32A32_SFLOAT;
    result.attributes[1].offset     = offsetof(lna::vertex, color);
    result.attributes[2].binding    = 0;
    result.attributes[2].location   = 2;
    result.attributes[2].format     = VK_FORMAT_R32G32_SFLOAT;
    result.attributes[2].offset     = offsetof(lna::vertex, uv);
    return result;
}
