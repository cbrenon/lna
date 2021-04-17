#ifndef LNA_BACKENDS_LNA_UI_H
#define LNA_BACKENDS_LNA_UI_H

#include <stdint.h>

typedef struct lna_ui_s             lna_ui_t;
typedef struct lna_ui_buffer_s      lna_ui_buffer_t;
typedef struct lna_memory_pool_s    lna_memory_pool_t;

typedef struct lna_ui_config_s
{
    uint32_t            max_buffer_count;
    lna_memory_pool_t*  memory_pool;
} lna_ui_config_t;

extern void             lna_ui_init         (lna_ui_t* ui, const lna_ui_config_t* config);
extern lna_ui_buffer_t* lna_ui_new_buffer   (lna_ui_t* ui);
extern void             lna_ui_release      (lna_ui_t* ui);

#endif
