#ifndef LNA_BACKENDS_VULKAN_LNA_SPRITE_VULKAN_H
#define LNA_BACKENDS_VULKAN_LNA_SPRITE_VULKAN_H

#include "backends/lna_sprite.h"
#include "backends/vulkan/lna_renderer_vulkan.h"
#include "core/lna_container.h"

typedef struct lna_mat4_s lna_mat4_t;

typedef struct lna_sprite_s
{
    lna_texture_t*                      texture;
    VkBuffer                            vertex_buffer;
    VkDeviceMemory                      vertex_buffer_memory;
    VkBuffer                            index_buffer;
    VkDeviceMemory                      index_buffer_memory;
    lna_vulkan_buffer_array_t           mv_uniform_buffers;
    lna_vulkan_device_memory_array_t    mv_uniform_buffers_memory;
    lna_vulkan_descriptor_set_array_t   descriptor_sets;
    lna_mat4_t*                         model_matrix;
    lna_mat4_t*                         view_matrix;
    lna_mat4_t*                         projection_matrix;
} lna_sprite_t;

lna_vector_def(lna_sprite_t) lna_sprite_vec_t;

typedef struct lna_sprite_system_s
{
    lna_renderer_t*                     renderer;
    lna_sprite_vec_t                    sprites;
    VkDescriptorSetLayout               descriptor_set_layout;
    VkDescriptorPool                    descriptor_pool;
    VkPipelineLayout                    pipeline_layout;
    VkPipeline                          pipeline;
} lna_sprite_system_t;

#endif
