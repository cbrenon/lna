#ifndef LNA_BACKENCS_SDL_LNA_GAMEPAD_SLD_H
#define LNA_BACKENCS_SDL_LNA_GAMEPAD_SLD_H

#include <stdint.h>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <SDL.h>
#pragma clang diagnostic pop

#include "system/lna_gamepad.h"

typedef struct lna_gamepad_array_s
{
    lna_gamepad_t*              states;
    uint32_t                    count;
} lna_gamepad_array_t;

typedef struct lna_gamepad_device_array_s
{
    SDL_GameController**        devices;
    uint32_t                    count;
} lna_gamepad_device_array_t;

typedef struct lna_gamepad_system_s
{
    lna_gamepad_array_t         gamepads;
    lna_gamepad_device_array_t  gamepad_devices;
} lna_gamepad_system_t;

#endif
