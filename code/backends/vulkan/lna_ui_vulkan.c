#include "backends/vulkan/lna_ui_vulkan.h"
#include "backends/vulkan/lna_vulkan.h"
#include "core/lna_assert.h"
#include "core/lna_file.h"

static void lna_ui_buffer_init(lna_ui_buffer_t* buffer, lna_ui_buffer_config_t* config, lna_renderer_t* renderer)
{
    lna_assert(buffer)
    lna_assert(buffer->vertices == NULL)
    lna_assert(buffer->indices == NULL)
    lna_assert(buffer->max_vertex_count == 0)
    lna_assert(buffer->max_index_count == 0)
    lna_assert(buffer->cur_vertex_count == 0)
    lna_assert(buffer->cur_index_count == 0)
    lna_assert(lna_array_is_empty(&buffer->vertex_buffers))
    lna_assert(lna_array_is_empty(&buffer->vertex_buffers_memory))
    lna_assert(buffer->vertex_data_mapped)
    lna_assert(lna_array_is_empty(&buffer->index_buffers))
    lna_assert(lna_array_is_empty(&buffer->index_buffers_memory))
    lna_assert(buffer->index_data_mapped)
    lna_assert(lna_array_is_empty(&buffer->descriptor_sets))
    lna_assert(buffer->texture == NULL)
    lna_assert(config)
    lna_assert(config->memory_pool)
    lna_assert(config->max_vertex_count > 0)
    lna_assert(config->max_index_count > 0)
    lna_assert(renderer)

    buffer->max_vertex_count    = config->max_vertex_count;
    buffer->max_index_count     = config->max_index_count;
    buffer->vertices            = lna_memory_alloc(config->memory_pool, lna_ui_vertex_t, buffer->max_vertex_count);
    buffer->indices             = lna_memory_alloc(config->memory_pool, uint32_t, buffer->max_index_count);
    buffer->texture             = config->texture;
}

static void lna_ui_buffer_release(lna_ui_buffer_t* buffer, VkDevice device)
{
    lna_assert(buffer)
    lna_assert(device)
}

void lna_ui_system_init(lna_ui_system_t* ui_system, const lna_ui_system_config_t* config)
{
    lna_assert(ui_system)
    lna_assert(lna_vector_size(&ui_system->buffers) == 0)
    lna_assert(ui_system->pipeline == VK_NULL_HANDLE)
    lna_assert(ui_system->pipeline_cache == VK_NULL_HANDLE)
    lna_assert(ui_system->pipeline_layout == VK_NULL_HANDLE)
    lna_assert(ui_system->descriptor_pool == VK_NULL_HANDLE)
    lna_assert(ui_system->descriptor_set_layout == VK_NULL_HANDLE)
    lna_assert(config)
    lna_assert(config->memory_pool)
    lna_assert(config->renderer)
    lna_assert(config->renderer->device)
    lna_assert(config->max_buffer_count > 0)

    ui_system->renderer = config->renderer;
    
    lna_vector_init(
        &ui_system->buffers,
        config->memory_pool,
        lna_ui_buffer_t,
        config->max_buffer_count
        );

    //! DESCRIPTOR POOL

    const VkDescriptorPoolSize descriptor_pool_sizes[1] =
    {
        {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = lna_array_size(&config->renderer->swap_chain_images),
        },
    };
    const VkDescriptorPoolCreateInfo pool_create_info =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = 1,
        .pPoolSizes = descriptor_pool_sizes,
        .maxSets = 2,
    };
    VULKAN_CHECK_RESULT(
        vkCreateDescriptorPool(
            config->renderer->device,
            &pool_create_info,
            NULL,
            &ui_system->descriptor_pool
            )
        )

    //! DESCRIPTOR SET LAYOUT

    const VkDescriptorSetLayoutBinding set_layout_bindings[1] =
    {
        {
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .binding = 0,
            .descriptorCount = 1,
        },
    };
    const VkDescriptorSetLayoutCreateInfo set_layout_create_info =
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pBindings = set_layout_bindings,
        .bindingCount = 1,
    };
    VULKAN_CHECK_RESULT(
        vkCreateDescriptorSetLayout(
            config->renderer->device,
            &set_layout_create_info,
            NULL,
            &ui_system->descriptor_set_layout
            )
        )

    //! PIPELINE

    // TODO: avoid direct file load
    lna_file_content_t vertex_shader_file;
    lna_file_debug_load(
        &vertex_shader_file,
        &config->renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_FRAME],
        "shaders/ui_vert.spv",
        true
        );
    lna_file_content_t fragment_shader_file;
    lna_file_debug_load(
        &fragment_shader_file,
        &config->renderer->memory_pools[LNA_VULKAN_RENDERER_MEMORY_POOL_FRAME],
        "shaders/ui_frag.spv",
        true
        );

    VkShaderModule vertex_shader_module = lna_vulkan_create_shader_module(
        config->renderer->device,
        (uint32_t*)vertex_shader_file.elements,
        vertex_shader_file.element_count
        );
    VkShaderModule fragment_shader_module = lna_vulkan_create_shader_module(
        config->renderer->device,
        (uint32_t*)fragment_shader_file.elements,
        fragment_shader_file.element_count
        );

    const VkPipelineShaderStageCreateInfo shader_stage_create_infos[2] =
    {
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertex_shader_module,
            .pName = "main",
        },
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragment_shader_module,
            .pName = "main",
        },
    };
}

lna_ui_buffer_t* lna_ui_system_new_buffer(lna_ui_system_t* ui_system, const lna_ui_buffer_config_t* config)
{
    lna_assert(ui_system)
    
    lna_ui_buffer_t* buffer;
    lna_vector_new_element(&ui_system->buffers, buffer);

    lna_ui_buffer_init(
        ui_system,
        config,
        ui_system->renderer
        );
}

void lna_ui_system_draw(lna_ui_system_t* ui_system)
{

}

void lna_ui_system_release(lna_ui_system_t* ui_system)
{
    lna_assert(ui_system)
    lna_assert(ui_system->renderer)

    for (uint32_t index = 0; index < lna_vector_size(&ui_system->buffers); ++index)
    {
        lna_ui_buffer_t* buffer = lna_vector_at(&ui_system->buffers, index);
        lna_ui_buffer_release(buffer, ui_system->renderer->device);
    }
}

static const uint32_t LNA_UI_VERTEX_COUNT_PER_RECT   = 4;
static const uint32_t LNA_UI_INDEX_COUNT_PER_RECT    = 6;

void lna_ui_buffer_push_rect(lna_ui_buffer_t* buffer, const lna_ui_buffer_rect_config_t* config)
{
    lna_assert(buffer)
    lna_assert(buffer->vertices)
    lna_assert(buffer->indices)
    lna_assert(buffer->cur_vertex_count + LNA_UI_VERTEX_COUNT_PER_RECT <= buffer->max_vertex_count)
    lna_assert(buffer->cur_index_count + LNA_UI_INDEX_COUNT_PER_RECT <= buffer->max_index_count)
    lna_assert(config)
    lna_assert(config->position)
    lna_assert(config->size)
    lna_assert(config->color)

    buffer->vertices[buffer->cur_vertex_count].position     = *config->position;
    buffer->vertices[buffer->cur_vertex_count].uv           = (lna_vec2_t){ 0.0f, 0.0f };
    buffer->vertices[buffer->cur_vertex_count].color        = *config->color;

    buffer->vertices[buffer->cur_vertex_count + 1].position = (lna_vec2_t){ config->position->x, config->position->y + config->size->height };
    buffer->vertices[buffer->cur_vertex_count + 1].uv       = (lna_vec2_t){ 0.0f, 0.0f };
    buffer->vertices[buffer->cur_vertex_count + 1].color    = *config->color;

    buffer->vertices[buffer->cur_vertex_count + 2].position = (lna_vec2_t){ config->position->x + config->size->width, config->position->y + config->size->height };
    buffer->vertices[buffer->cur_vertex_count + 2].uv       = (lna_vec2_t){ 0.0f, 0.0f };
    buffer->vertices[buffer->cur_vertex_count + 2].color    = *config->color;

    buffer->vertices[buffer->cur_vertex_count + 3].position = (lna_vec2_t){ config->position->x + config->size->width, config->position->y };
    buffer->vertices[buffer->cur_vertex_count + 3].uv       = (lna_vec2_t){ 0.0f, 0.0f };
    buffer->vertices[buffer->cur_vertex_count + 3].color    = *config->color;

    buffer->indices[buffer->cur_index_count]                = buffer->cur_vertex_count;
    buffer->indices[buffer->cur_index_count + 1]            = buffer->cur_vertex_count + 1;
    buffer->indices[buffer->cur_index_count + 2]            = buffer->cur_vertex_count + 2;
    buffer->indices[buffer->cur_index_count + 3]            = buffer->cur_vertex_count + 2;
    buffer->indices[buffer->cur_index_count + 4]            = buffer->cur_vertex_count + 3;
    buffer->indices[buffer->cur_index_count + 5]            = buffer->cur_vertex_count;

    buffer->cur_vertex_count += LNA_UI_VERTEX_COUNT_PER_RECT;
    buffer->cur_index_count += LNA_UI_INDEX_COUNT_PER_RECT;
}

void lna_ui_buffer_push_text(lna_ui_buffer_t* buffer, const lna_ui_buffer_text_config_t* config)
{
    lna_assert(buffer)
    lna_assert(config)
    lna_assert(config->text)
    lna_assert(config->position)
    lna_assert(config->color)

    lna_vec2_t  char_pos            = *config->position;
    size_t      text_length         = strlen(config->text);
    lna_vec2_t  texture_offset_pos  = { 0.0f, 0.0f };

    lna_assert(buffer->cur_vertex_count + ((uint32_t)text_length * LNA_UI_VERTEX_COUNT_PER_RECT) < buffer->max_vertex_count)
    lna_assert(buffer->cur_vertex_count + ((uint32_t)text_length * LNA_UI_INDEX_COUNT_PER_RECT) < buffer->max_vertex_count)

    for (size_t i = 0; i < text_length; ++i)
    {
        if (config->text[i] == '\n')
        {
            char_pos = (lna_vec2_t) { config->position->x, char_pos.y + config->leading };
        }
        else
        {
            texture_offset_pos = (lna_vec2_t)
            {
                (float)((uint32_t)config->text[i] % config->texture_col_char_count) * config->uv_char_size->width,
                (float)((uint32_t)config->text[i] / config->texture_row_char_count) * config->uv_char_size->height
            };

            buffer->vertices[buffer->cur_vertex_count].position     = (lna_vec2_t){ char_pos.x, char_pos.y };
            buffer->vertices[buffer->cur_vertex_count].uv           = (lna_vec2_t){ texture_offset_pos.x, texture_offset_pos.y };
            buffer->vertices[buffer->cur_vertex_count].color        = *config->color;

            buffer->vertices[buffer->cur_vertex_count + 1].position = (lna_vec2_t){ char_pos.x, char_pos.y + config->size };
            buffer->vertices[buffer->cur_vertex_count + 1].uv       = (lna_vec2_t){ texture_offset_pos.x, texture_offset_pos.y + config->uv_char_size->height };
            buffer->vertices[buffer->cur_vertex_count + 1].color    = *config->color;

            buffer->vertices[buffer->cur_vertex_count + 2].position = (lna_vec2_t){ char_pos.x + config->size, char_pos.y + config->size };
            buffer->vertices[buffer->cur_vertex_count + 2].uv       = (lna_vec2_t){ texture_offset_pos.x + config->uv_char_size->width, texture_offset_pos.y + config->uv_char_size->height };
            buffer->vertices[buffer->cur_vertex_count + 2].color    = *config->color;

            buffer->vertices[buffer->cur_vertex_count + 3].position = (lna_vec2_t){ char_pos.x + config->size, char_pos.y };
            buffer->vertices[buffer->cur_vertex_count + 3].uv       = (lna_vec2_t){ texture_offset_pos.x + config->uv_char_size->width, texture_offset_pos.y };
            buffer->vertices[buffer->cur_vertex_count + 3].color    = *config->color;

            buffer->indices[buffer->cur_index_count]                = buffer->cur_vertex_count;
            buffer->indices[buffer->cur_index_count + 1]            = buffer->cur_vertex_count + 1;
            buffer->indices[buffer->cur_index_count + 2]            = buffer->cur_vertex_count + 2;
            buffer->indices[buffer->cur_index_count + 3]            = buffer->cur_vertex_count + 2;
            buffer->indices[buffer->cur_index_count + 4]            = buffer->cur_vertex_count + 3;
            buffer->indices[buffer->cur_index_count + 5]            = buffer->cur_vertex_count;

            buffer->cur_vertex_count    += LNA_UI_VERTEX_COUNT_PER_RECT;
            buffer->cur_index_count     += LNA_UI_INDEX_COUNT_PER_RECT;

            char_pos.x += config->size + config->spacing;
        }
    }
}

void lna_ui_buffer_empty(lna_ui_buffer_t* buffer)
{
    lna_assert(buffer)
    buffer->cur_vertex_count = 0;
    buffer->cur_index_count = 0;
}
