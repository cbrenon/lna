#ifndef LNA_BACKENDS_VULKAN_LNA_MESH_VULKAN_H
#define LNA_BACKENDS_VULKAN_LNA_MESH_VULKAN_H

#include "backends/vulkan/lna_renderer_vulkan.h"

typedef struct lna_material_s           lna_material_t;
typedef struct lna_mat4_s               lna_mat4_t;

typedef struct lna_mesh_s
{
    const lna_material_t*               material;
    VkBuffer                            vertex_buffer;
    VkDeviceMemory                      vertex_buffer_memory;
    VkBuffer                            index_buffer;
    VkDeviceMemory                      index_buffer_memory;
    lna_vulkan_buffer_array_t           mvp_uniform_buffers;
    lna_vulkan_device_memory_array_t    mvp_uniform_buffers_memory;
    lna_vulkan_buffer_array_t           light_uniform_buffers;
    lna_vulkan_device_memory_array_t    light_uniform_buffers_memory;
    lna_vulkan_descriptor_set_array_t   descriptor_sets;
    const lna_mat4_t*                   model_matrix;
    const lna_mat4_t*                   view_matrix;
    const lna_mat4_t*                   projection_matrix;
    uint32_t                            index_count;
} lna_mesh_t;

typedef struct lna_mesh_vec_s
{
    uint32_t                            cur_element_count;
    uint32_t                            max_element_count;
    lna_mesh_t*                         elements;
} lna_mesh_vec_t;

typedef struct lna_mesh_system_s
{
    lna_renderer_t*                     renderer;
    lna_mesh_vec_t                      meshes;
    VkDescriptorSetLayout               descriptor_set_layout;
    VkDescriptorPool                    descriptor_pool;
    VkPipelineLayout                    pipeline_layout;
    VkPipeline                          pipeline;
} lna_mesh_system_t;

#endif
