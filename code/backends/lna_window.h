#ifndef LNA_BACKENDS_LNA_WINDOW_H
#define LNA_BACKENDS_LNA_WINDOW_H

#include <stdint.h>
#include <stdbool.h>

typedef struct lna_window_s lna_window_t;

typedef struct lna_window_config_s
{
    const char* title;
    uint32_t    width;
    uint32_t    height;
    bool        fullscreen;
} lna_window_config_t;


extern void     lna_window_init         (lna_window_t* window, const lna_window_config_t* config);
extern void     lna_window_set_title    (lna_window_t* window, const char* title);
extern void     lna_window_on_resize    (lna_window_t* window);
extern uint32_t lna_window_width        (const lna_window_t* window);
extern uint32_t lna_window_height       (const lna_window_t* window);
extern void     lna_window_release      (lna_window_t* window);

#endif
