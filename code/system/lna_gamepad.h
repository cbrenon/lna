#ifndef LNA_SYSTEM_LNA_GAMEPAD_H
#define LNA_SYSTEM_LNA_GAMEPAD_H

#include <stdint.h>
#include <stdbool.h>
#include "maths/lna_vec2.h"

typedef struct lna_gamepad_system_s lna_gamepad_system_t;
typedef struct lna_memory_pool_s    lna_memory_pool_t;

typedef enum lna_gamepad_value_e
{
    LNA_GAMEPAD_VALUE_LEFT_STICK_AXIS_MAX_VALUE,
    LNA_GAMEPAD_VALUE_LEFT_STICK_AXIS_DEAD_ZONE,
    LNA_GAMEPAD_VALUE_RIGHT_STICK_AXIS_MAX_VALUE,
    LNA_GAMEPAD_VALUE_RIGHT_STICK_AXIS_DEAD_ZONE,
    LNA_GAMEPAD_VALUE_COUNT,
} lna_gamepad_value_t;

typedef enum lna_gamepad_button_e
{
    LNA_GAMEPAD_BUTTON_A,
    LNA_GAMEPAD_BUTTON_B,
    LNA_GAMEPAD_BUTTON_X,
    LNA_GAMEPAD_BUTTON_Y,
    LNA_GAMEPAD_BUTTON_BACK,
    LNA_GAMEPAD_BUTTON_GUIDE,
    LNA_GAMEPAD_BUTTON_START,
    LNA_GAMEPAD_BUTTON_LEFT_STICK,
    LNA_GAMEPAD_BUTTON_RIGHT_STICK,
    LNA_GAMEPAD_BUTTON_LEFT_SHOULDER,
    LNA_GAMEPAD_BUTTON_RIGHT_SHOULDER,
    LNA_GAMEPAD_BUTTON_DPAD_UP,
    LNA_GAMEPAD_BUTTON_DPAD_DOWN,
    LNA_GAMEPAD_BUTTON_DPAD_LEFT,
    LNA_GAMEPAD_BUTTON_DPAD_RIGHT,
    LNA_GAMEPAD_BUTTON_COUNT,
    LNA_GAMEPAD_BUTTON_NONE,
} lna_gamepad_button_t;

typedef struct lna_gamepad_s
{
    lna_vec2_t          left_stick_axis;
    float               values[LNA_GAMEPAD_VALUE_COUNT];
    bool                buttons[LNA_GAMEPAD_BUTTON_COUNT];
} lna_gamepad_t;

typedef struct lna_gamepad_system_config_s
{
    uint32_t            max_gamepad_count;
} lna_gamepad_system_config_t;

typedef struct lna_gamepad_config_s
{
    uint32_t            device_index;
    float               values[LNA_GAMEPAD_VALUE_COUNT];
} lna_gamepad_config_t;

extern uint32_t         lna_gamepad_device_count(void);
extern void             lna_gamepad_system_init(lna_gamepad_system_t* gamepad_system, const lna_gamepad_system_config_t* config);
extern lna_gamepad_t*   lna_gamepad_system_new_gamepad(lna_gamepad_system_t* gamepad_system, const lna_gamepad_config_t* config);
extern void             lna_gamepad_system_update(lna_gamepad_system_t* gamepad_system);
extern void             lna_gamepad_system_release(lna_gamepad_system_t* gamepad_system);

#endif
