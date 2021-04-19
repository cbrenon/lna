#ifndef LNA_GRAPHICS_LNA_UI_BUFFER_H
#define LNA_GRAPHICS_LNA_UI_BUFFER_H

#include <stdint.h>
#include "maths/lna_vec2.h"
#include "maths/lna_vec4.h"

typedef struct lna_memory_pool_s lna_memory_pool_t;

typedef struct lna_ui_vertex_s
{
    lna_vec2_t  position;
    lna_vec2_t  uv;
    lna_vec4_t  color;
} lna_ui_vertex_t;

typedef struct lna_ui_buffer_s
{
    lna_ui_vertex_t*    vertices;
    uint32_t*           indices;
    uint32_t            max_vertex_count;
    uint32_t            cur_vertex_count;
    uint32_t            max_index_count;
    uint32_t            cur_index_count;
} lna_ui_buffer_t;

typedef struct lna_ui_buffer_config_s
{
    lna_memory_pool_t*  memory_pool;
    uint32_t            max_vertex_count;
    uint32_t            max_index_count;
} lna_ui_buffer_config_t;

typedef struct lna_ui_buffer_rect_config_s
{
    const lna_vec2_t*   position;
    const lna_vec2_t*   size;
    const lna_vec4_t*   color;
} lna_ui_buffer_rect_config_t;

typedef struct lna_ui_buffer_text_config_s
{
    const char*         text;
    const lna_vec2_t*   position;
    float               size;
    const lna_vec4_t*   color;
    float               leading;
    float               spacing;
    uint32_t            texture_col_char_count;
    uint32_t            texture_row_char_count;
    lna_vec2_t*         uv_char_size;
} lna_ui_buffer_text_config_t;

extern void lna_ui_buffer_init      (lna_ui_buffer_t* buffer, const lna_ui_buffer_config_t* config);
extern void lna_ui_buffer_push_rect (lna_ui_buffer_t* buffer, const lna_ui_buffer_rect_config_t* config);
extern void lna_ui_buffer_push_text (lna_ui_buffer_t* buffer, const lna_ui_buffer_text_config_t* config);
extern void lna_ui_buffer_empty     (lna_ui_buffer_t* buffer);

#endif
