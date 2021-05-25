#ifndef LNA_BACKENDS_VULKAN_LNA_PRIMITIVE_VULKAN_H
#define LNA_BACKENDS_VULKAN_LNA_PRIMITIVE_VULKAN_H

#include <stdbool.h>
#include "backends/vulkan/lna_renderer_vulkan.h"

typedef struct lna_primitive_s
{
    VkBuffer                            vertex_buffer;
    VkDeviceMemory                      vertex_buffer_memory;
    VkBuffer                            index_buffer;
    VkDeviceMemory                      index_buffer_memory;
    uint32_t                            vertex_count;
    uint32_t                            index_count;
    lna_vulkan_buffer_array_t           mvp_uniform_buffers;
    lna_vulkan_device_memory_array_t    mvp_uniform_buffers_memory;
    lna_vulkan_descriptor_set_array_t   descriptor_sets;
    const lna_mat4_t*                   model_matrix;
    const lna_mat4_t*                   view_matrix;
    const lna_mat4_t*                   projection_matrix;
} lna_primitive_t;

lna_vector_def(lna_primitive_t)         lna_primitive_vec_t;

typedef struct lna_primitive_system_s
{
    lna_renderer_t*                     renderer;
    lna_primitive_vec_t                 primitives;
    VkDescriptorSetLayout               descriptor_set_layout;
    VkDescriptorPool                    descriptor_pool;
    VkPipelineLayout                    pipeline_layout;
    VkPipeline                          pipeline;
    bool                                fill_shapes;
} lna_primitive_system_t;

#endif
