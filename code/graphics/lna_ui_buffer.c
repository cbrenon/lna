#include <string.h>
#include "graphics/lna_ui_buffer.h"
#include "core/lna_memory_pool.h"
#include "core/lna_assert.h"

void lna_ui_buffer_init(lna_ui_buffer_t* buffer, const lna_ui_buffer_config_t* config)
{
    lna_assert(buffer)
    lna_assert(buffer->vertices == NULL)
    lna_assert(buffer->indices == NULL)
    lna_assert(buffer->max_vertex_count == 0)
    lna_assert(buffer->max_index_count == 0)
    lna_assert(buffer->cur_vertex_count == 0)
    lna_assert(buffer->cur_index_count == 0)
    lna_assert(config)
    lna_assert(config->memory_pool)
    lna_assert(config->max_vertex_count > 0)
    lna_assert(config->max_index_count > 0)

    buffer->max_vertex_count    = config->max_vertex_count;
    buffer->max_index_count     = config->max_index_count;
    buffer->vertices            = lna_memory_alloc(config->memory_pool, lna_ui_vertex_t, buffer->max_vertex_count);
    buffer->indices             = lna_memory_alloc(config->memory_pool, uint32_t, buffer->max_index_count);
    buffer->texture             = config->texture;
}

