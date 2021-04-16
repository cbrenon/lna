#ifndef LNA_BACKENDS_LNA_INPUT_H
#define LNA_BACKENDS_LNA_INPUT_H

#include <stdbool.h>

typedef enum lna_key_e
{
    LNA_KEY_ESC,
    LNA_KEY_UP,
    LNA_KEY_DOWN,
    LNA_KEY_ENTER,
    LNA_KEY_BACKSPACE,
    LNA_KEY_PERIOD,
    LNA_KEY_MINUS,
    LNA_KEY_0,
    LNA_KEY_1,
    LNA_KEY_2,
    LNA_KEY_3,
    LNA_KEY_4,
    LNA_KEY_5,
    LNA_KEY_6,
    LNA_KEY_7,
    LNA_KEY_8,
    LNA_KEY_9,
    LNA_KEY_COUNT,
    LNA_KEY_NONE,
} lna_key_t;

typedef enum lna_input_event_e
{
    LNA_INPUT_EVENT_NONE,
    LNA_INPUT_EVENT_WINDOW_CLOSED,
    LNA_INPUT_EVENT_WINDOW_RESIZED,
} lna_input_event_t;

typedef struct lna_input_s lna_input_t;

extern void                 lna_input_init                      (lna_input_t* input);
extern lna_input_event_t    lna_input_poll_events               (lna_input_t* input);
extern bool                 lna_input_is_key_pressed            (const lna_input_t* input, lna_key_t key);
extern bool                 lna_input_is_key_has_been_pressed   (const lna_input_t* input, lna_key_t key);

#endif
