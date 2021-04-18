#ifndef LNA_BACKENDS_VULKAN_LNA_UI_VULKAN_H
#define LNA_BACKENDS_VULKAN_LNA_UI_VULKAN_H

#include <vulkan/vulkan.h>
#include "graphics/lna_ui_buffer.h"
#include "core/lna_container.h"

typedef struct lna_ui_push_const_block_vulkan_s
{
    lna_vec2_t                          scale;
    lna_vec2_t                          translate;
} lna_ui_push_const_block_vulkan_t;

typedef struct lna_ui_buffer_vulkan_s
{
    lna_ui_buffer_t                     buffer;
    VkBuffer                            vertex_buffer;
    VkDeviceMemory                      vertex_buffer_memory;
    VkBuffer                            index_buffer;
    VkDeviceMemory                      index_buffer_memory;
    //texture
    lna_ui_push_const_block_vulkan_t    push_const_block;
    void*                               vertex_data_mapped;
    void*                               index_data_mapped;
} lna_ui_buffer_vulkan_t;

lna_array_def(lna_ui_buffer_vulkan_t) lna_ui_buffer_vulkan_array_t;

typedef struct lna_ui_s
{
    lna_ui_buffer_vulkan_array_t        buffers;
    VkPipeline                          pipeline;
    VkPipelineCache                     pipeline_cache;
    VkPipelineLayout                    pipeline_layout;
    VkDescriptorPool                    descriptor_pool;
    VkDescriptorSetLayout               descriptor_set_layout;
    VkDescriptorSet                     descriptor_set;
} lna_ui_t;

#endif
