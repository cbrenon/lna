#ifndef LNA_BACKENDS_VULKAN_LNA_UI_VULKAN_H
#define LNA_BACKENDS_VULKAN_LNA_UI_VULKAN_H

#include "backends/lna_ui.h"
#include "backends/vulkan/lna_renderer_vulkan.h"
#include "core/lna_container.h"

typedef struct lna_ui_vertex_s
{
    lna_vec2_t                          position;
    lna_vec2_t                          uv;
    lna_vec4_t                          color;
} lna_ui_vertex_t;

typedef struct lna_ui_push_const_block_vulkan_s
{
    lna_vec2_t                          scale;
    lna_vec2_t                          translate;
} lna_ui_push_const_block_vulkan_t;

typedef struct lna_ui_buffer_s
{
    lna_ui_vertex_t*                    vertices;
    uint32_t*                           indices;
    uint32_t                            max_vertex_count;
    uint32_t                            cur_vertex_count;
    uint32_t                            max_index_count;
    uint32_t                            cur_index_count;
    lna_texture_t*                      texture;
    VkBuffer                            vertex_buffer;
    VkDeviceMemory                      vertex_buffer_memory;
    VkBuffer                            index_buffer;
    VkDeviceMemory                      index_buffer_memory;
    VkDescriptorSet                     descriptor_set;   
    lna_ui_push_const_block_vulkan_t    push_const_block;
    void*                               vertex_data_mapped;
    void*                               index_data_mapped;
} lna_ui_buffer_t;

lna_vector_def(lna_ui_buffer_t) lna_ui_buffer_vec_t;

typedef struct lna_ui_system_s
{
    lna_ui_buffer_vec_t                 buffers;
    VkPipeline                          pipeline;
    VkPipelineCache                     pipeline_cache;
    VkPipelineLayout                    pipeline_layout;
    VkDescriptorPool                    descriptor_pool;
    VkDescriptorSetLayout               descriptor_set_layout;
    lna_renderer_t*                     renderer;
} lna_ui_system_t;

#endif
