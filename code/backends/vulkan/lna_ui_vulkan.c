#include "backends/vulkan/lna_ui_vulkan.h"
#include "core/lna_assert.h"

void lna_ui_system_init(lna_ui_system_t* ui_system, const lna_ui_system_config_t* config)
{
    lna_assert(ui_system)
    VkPipeline                          pipeline;
    VkPipelineCache                     pipeline_cache;
    VkPipelineLayout                    pipeline_layout;
    VkDescriptorPool                    descriptor_pool;
    VkDescriptorSetLayout               descriptor_set_layout;
    VkDescriptorSet                     descriptor_set;
} lna_ui_system_t;
}

lna_ui_buffer_t* lna_ui_system_new_buffer(lna_ui_system_t* ui_system, const lna_ui_buffer_config_t* config)
{

}

void lna_ui_system_draw(lna_ui_system_t* ui_system)
{

}

void lna_ui_system_release(lna_ui_system_t* ui_system)
{

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
