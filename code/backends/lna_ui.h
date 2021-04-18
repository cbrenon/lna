#ifndef LNA_BACKENDS_LNA_UI_H
#define LNA_BACKENDS_LNA_UI_H

#include <stdint.h>

typedef struct lna_texture_s        lna_texture_t;
typedef struct lna_ui_system_s      lna_ui_system_t;
typedef struct lna_ui_buffer_s      lna_ui_buffer_t;
typedef struct lna_memory_pool_s    lna_memory_pool_t;

typedef struct lna_ui_system_config_s
{
    uint32_t            max_buffer_count;
    lna_memory_pool_t*  memory_pool;
} lna_ui_system_config_t;

extern void             lna_ui_system_init                  (lna_ui_system_t* ui_system, const lna_ui_system_config_t* config);
extern lna_ui_buffer_t* lna_ui_system_new_buffer            (lna_ui_system_t* ui_system);
extern void             lna_ui_system_set_texture_buffer    (lna_ui_system_t* ui_system, lna_ui_buffer_t* ui_buffer, lna_texture_t* texture);
extern void             lna_ui_system_draw                  (lna_ui_system_t* ui_system);
extern void             lna_ui_system_release               (lna_ui_system_t* ui_system);

#endif
