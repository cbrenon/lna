#include "backends/sdl/lna_gamepad_sdl.h"
#include "core/lna_assert.h"
#include "core/lna_memory_pool.h"

uint32_t lna_gamepad_device_count(void)
{
    uint32_t result = 0;
    int joystick_count = SDL_NumJoysticks(); 
    for (int i = 0; i < joystick_count; ++i)
    {
        if (SDL_IsGameController(i))
        {
            ++result;
        }
    }
    return result;
}

void lna_gamepad_system_init(lna_gamepad_system_t* gamepad_system, const lna_gamepad_system_config_t* config)
{
    lna_assert(gamepad_system)
    lna_assert(config)

    lna_array_init(
        &gamepad_system->gamepads,
        config->memory_pool,
        lna_gamepad_t,
        config->max_gamepad_count
        );

    lna_array_init(
        &gamepad_system->gamepad_devices,
        config->memory_pool,
        SDL_GameController*,
        config->max_gamepad_count
        );
}

lna_gamepad_t* lna_gamepad_system_new_gamepad(lna_gamepad_system_t* gamepad_system, const lna_gamepad_config_t* config)
{
    lna_assert(gamepad_system)
    lna_assert(config)
    lna_assert(config->device_index < lna_gamepad_device_count())
    lna_assert(config->device_index < lna_array_size(&gamepad_system->gamepads))
    lna_assert(config->device_index < lna_array_size(&gamepad_system->gamepad_devices))
    lna_assert(lna_array_at_ref(&gamepad_system->gamepad_devices, config->device_index) == NULL)
    
    lna_gamepad_t*  gamepad         = lna_array_at_ptr(&gamepad_system->gamepads, config->device_index);
    int             joystick_count  = SDL_NumJoysticks();
    uint32_t        gamepad_count   = 0;
    for (int i = 0; i < joystick_count; ++i)
    {
        if (
            SDL_IsGameController(i)
            && gamepad_count == config->device_index
            )
        {
            gamepad->left_stick_dead_zone = config->left_stick_dead_zone;
            gamepad->left_stick_max_value = config->left_stick_max_value;
            lna_array_at_ref(&gamepad_system->gamepad_devices, config->device_index) = SDL_GameControllerOpen(i);
            gamepad->device_state = lna_array_at_ref(&gamepad_system->gamepad_devices, config->device_index) != NULL ? LNA_GAMEPAD_DEVICE_STATE_OPENED : LNA_GAMEPAD_DEVICE_STATE_CLOSED;
            break;
        }
        ++gamepad_count;
    }
    return gamepad;
}

void lna_gamepad_system_update(lna_gamepad_system_t* gamepad_system)
{
    lna_assert(gamepad_system)
    lna_assert(lna_array_size(&gamepad_system->gamepads) == lna_array_size(&gamepad_system->gamepad_devices))

    for (uint32_t i = 0; i < lna_array_size(&gamepad_system->gamepad_devices); ++i)
    {
        SDL_GameController* device  = lna_array_at_ref(&gamepad_system->gamepad_devices, i);
        lna_gamepad_t*      gamepad = lna_array_at_ptr(&gamepad_system->gamepads, i);
        if(gamepad->device_state == LNA_GAMEPAD_DEVICE_STATE_OPENED)
        {
            if (device == NULL)
            {
                gamepad->device_state = LNA_GAMEPAD_DEVICE_STATE_CLOSED;
                continue;
            }
            else if (SDL_GameControllerGetAttached(device) == SDL_FALSE)
            {
                gamepad->device_state = LNA_GAMEPAD_DEVICE_STATE_REMOVED;
                SDL_GameControllerClose(device);
                continue;
            }

            for (size_t b = 0; b < LNA_GAMEPAD_BUTTON_COUNT; ++b)
            {
                gamepad->buttons[i] = SDL_GameControllerGetButton(device, (SDL_GameControllerButton)b) == 1;
            }
            float x = (float)((uint32_t)SDL_GameControllerGetAxis(device, SDL_CONTROLLER_AXIS_LEFTX)); // TODO: fix this ugly "warning safe" double cast
            float y = (float)((uint32_t)SDL_GameControllerGetAxis(device, SDL_CONTROLLER_AXIS_LEFTY)); // TODO: fix this ugly "warning safe" double cast
            float nx = x / gamepad->left_stick_max_value;
            float ny = y / gamepad->left_stick_max_value;
            float tx = 0.0f;
            float ty = 0.0f;
            if (fabsf(nx) > gamepad->left_stick_dead_zone)
            {
                tx = nx > 1.0f ? 1.0f : nx;
                tx = nx < -1.0f ? -1.0f : nx;
            }
            else
            {
                tx = 0.0f;
            }
            if (fabsf(ny) > gamepad->left_stick_dead_zone)
            {
                ty = ny > 1.0f ? 1.0f : ny;
                ty = ny < -1.0f ? -1.0f : ny;
            }
            else
            {
                ty = 0.0f;
            }
            gamepad->left_stick_axis.x = tx;
            gamepad->left_stick_axis.y = ty;
        }
    }
}

void lna_gamepad_system_release(lna_gamepad_system_t* gamepad_system)
{
    lna_assert(gamepad_system)
    lna_assert(lna_array_size(&gamepad_system->gamepads) == lna_array_size(&gamepad_system->gamepad_devices))

    for (uint32_t i = 0; i < lna_array_size(&gamepad_system->gamepad_devices); ++i)
    {
        SDL_GameController* device = lna_array_at_ref(&gamepad_system->gamepad_devices, i);
        lna_gamepad_t*      gamepad = lna_array_at_ptr(&gamepad_system->gamepads, i);
        if (device)
        {
            SDL_GameControllerClose(device);
        }
        gamepad->device_state = LNA_GAMEPAD_DEVICE_STATE_UNKNOWN;
    }
}
