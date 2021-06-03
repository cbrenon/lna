#ifndef LNA_BACKENCS_SDL_LNA_GAMEPAD_SLD_H
#define LNA_BACKENCS_SDL_LNA_GAMEPAD_SLD_H

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <SDL.h>
#pragma clang diagnostic pop

#include "system/lna_gamepad.h"
#include "core/lna_container.h"

lna_array_def(lna_gamepad_t)        lna_gamepad_array_t;
lna_array_def(SDL_GameController*)  lna_gamepad_device_array_t;

typedef struct lna_gamepad_system_s
{
    lna_gamepad_array_t         gamepads;
    lna_gamepad_device_array_t  gamepad_devices;
} lna_gamepad_system_t;

#endif
