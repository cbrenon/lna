#ifndef LNA_GRAPHICS_LNA_UI_BUFFER_H
#define LNA_GRAPHICS_LNA_UI_BUFFER_H

#include <stdint.h>


typedef struct lna_memory_pool_s    lna_memory_pool_t;
typedef struct lna_texture_s        lna_texture_t;





extern void lna_ui_buffer_init      (lna_ui_buffer_t* buffer, const lna_ui_buffer_config_t* config);
extern void lna_ui_buffer_push_rect (lna_ui_buffer_t* buffer, const lna_ui_buffer_rect_config_t* config);
extern void lna_ui_buffer_push_text (lna_ui_buffer_t* buffer, const lna_ui_buffer_text_config_t* config);
extern void lna_ui_buffer_empty     (lna_ui_buffer_t* buffer);

#endif
