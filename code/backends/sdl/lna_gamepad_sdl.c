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

    gamepad_system->gamepads.count  = config->max_gamepad_count;
    gamepad_system->gamepads.states = lna_memory_pool_reserve(
        config->memory_pool,
        config->max_gamepad_count * sizeof(lna_gamepad_t)
        );
    
    gamepad_system->gamepad_devices.count   = config->max_gamepad_count;
    gamepad_system->gamepad_devices.devices = lna_memory_pool_reserve(
        config->memory_pool,
        config->max_gamepad_count * sizeof(SDL_GameController*)
        );
}

lna_gamepad_t* lna_gamepad_system_new_gamepad(lna_gamepad_system_t* gamepad_system, const lna_gamepad_config_t* config)
{
    lna_assert(gamepad_system)
    lna_assert(config)
    lna_assert(config->device_index < lna_gamepad_device_count())
    lna_assert(config->device_index < gamepad_system->gamepads.count)
    lna_assert(config->device_index < gamepad_system->gamepad_devices.count)
    lna_assert(gamepad_system->gamepad_devices.devices[config->device_index] == NULL)
    
    lna_gamepad_t*  gamepad         = &gamepad_system->gamepads.states[config->device_index];
    int             joystick_count  = SDL_NumJoysticks();
    uint32_t        gamepad_count   = 0;
    for (int i = 0; i < joystick_count; ++i)
    {
        if (
            SDL_IsGameController(i)
            && gamepad_count == config->device_index
            )
        {
            gamepad->left_stick_dead_zone                                   = config->left_stick_dead_zone;
            gamepad->left_stick_max_value                                   = config->left_stick_max_value;
            gamepad_system->gamepad_devices.devices[config->device_index]   = SDL_GameControllerOpen(i);
            gamepad->device_state                                           = gamepad_system->gamepad_devices.devices[config->device_index] != NULL ? LNA_GAMEPAD_DEVICE_STATE_OPENED : LNA_GAMEPAD_DEVICE_STATE_CLOSED;
            break;
        }
        ++gamepad_count;
    }
    return gamepad;
}

void lna_gamepad_system_update(lna_gamepad_system_t* gamepad_system)
{
    lna_assert(gamepad_system)
    lna_assert(gamepad_system->gamepads.count == gamepad_system->gamepad_devices.count)

    for (uint32_t i = 0; i < gamepad_system->gamepad_devices.count; ++i)
    {
        SDL_GameController* device  = gamepad_system->gamepad_devices.devices[i];
        lna_gamepad_t*      gamepad = &gamepad_system->gamepads.states[i];
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
    lna_assert(gamepad_system->gamepads.count == gamepad_system->gamepad_devices.count)

    for (uint32_t i = 0; i < gamepad_system->gamepad_devices.count; ++i)
    {
        SDL_GameController* device  = gamepad_system->gamepad_devices.devices[i];
        lna_gamepad_t*      gamepad = &gamepad_system->gamepads.states[i];
        if (device)
        {
            SDL_GameControllerClose(device);
        }
        gamepad->device_state = LNA_GAMEPAD_DEVICE_STATE_UNKNOWN;
    }
}
