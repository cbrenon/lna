#include <cmath>
#include "backends/sdl/sdl_gamepad.hpp"
#include "core/assert.hpp"

namespace lna
{
    int gamepad_count()
    {
        int result = 0;
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

    void gamepad_init(
        gamepad_api& gamepad
        )
    {
        gamepad.device                          = nullptr;
        gamepad.info.device_state               = gamepad_info::state::UNKNOWN;
        gamepad.info.left_stick_axis_x          = 0.0f;
        gamepad.info.left_stick_axis_y          = 0.0f;
        gamepad.info.left_stick_axis_dead_zone  = 0.0f;
        gamepad.info.left_stick_axis_max_value  = 0.0f;
        for (size_t i = 0; i < MAX_GAMEPAD_BUTTON_COUNT; ++i)
        {
            gamepad.info.buttons[i] = false;
        }
    }

    void gamepad_open(
        gamepad_api& gamepad,
        const gamepad_config& config
        )
    {
        LNA_ASSERT(gamepad.info.device_state == gamepad_info::state::UNKNOWN);
        LNA_ASSERT(gamepad.device == nullptr);

        int joystick_count = SDL_NumJoysticks();
        int gamepad_count = 0;
        for (int i = 0; i < joystick_count; ++i)
        {
            if (SDL_IsGameController(i))
            {
                if (gamepad_count == config.device_index)
                {
                    gamepad.info.left_stick_axis_dead_zone  = config.left_stick_axis_dead_zone;
                    gamepad.info.left_stick_axis_max_value  = config.left_stick_axis_max_value;
                    gamepad.device                          = SDL_GameControllerOpen(i);   
                    break;
                }
                ++gamepad_count;
            }
        }
    }

    void gamepad_update(
        gamepad_api& gamepad
        )
    {
        SDL_GameControllerUpdate();

        if (gamepad.device == nullptr)
        {
            gamepad.info.device_state = gamepad_info::state::CLOSED;
            return;
        }
        else if (SDL_GameControllerGetAttached(gamepad.device) == SDL_FALSE)
        {
            gamepad_close(gamepad);
            return;
        }

        for (size_t i = 0; i < MAX_GAMEPAD_BUTTON_COUNT; ++i)
        {
            gamepad.info.buttons[i] = SDL_GameControllerGetButton(
                gamepad.device,
                (SDL_GameControllerButton)i
                ) == 1;
        }

        float x = (float)SDL_GameControllerGetAxis(gamepad.device, SDL_CONTROLLER_AXIS_LEFTX);
        float y = (float)SDL_GameControllerGetAxis(gamepad.device, SDL_CONTROLLER_AXIS_LEFTY);
        float nx = x / gamepad.info.left_stick_axis_max_value;
        float ny = y / gamepad.info.left_stick_axis_max_value;
        float tx = 0.0f;
        float ty = 0.0f;

        if (fabs(nx) > gamepad.info.left_stick_axis_dead_zone)
        {
            tx = nx > 1.0f ? 1.0f : nx;
            tx = nx < -1.0f ? -1.0f : nx;
        }
        else
        {
            tx = 0.0f;
        }
        if (fabs(ny) > gamepad.info.left_stick_axis_dead_zone)
        {
            ty = ny > 1.0f ? 1.0f : ny;
            ty = ny < -1.0f ? -1.0f : ny;
        }
        else
        {
            ty = 0.0f;
        }
        gamepad.info.left_stick_axis_x = tx;
        gamepad.info.left_stick_axis_y = ty;

    }

    const gamepad_info& gamepad_last_update_info(
        gamepad_api& gamepad
        )
    {
        return gamepad.info;
    }
    
    void gamepad_close(
        gamepad_api& gamepad
        )
    {
        if (gamepad.device)
        {
            SDL_GameControllerClose(gamepad.device);
            gamepad_init(gamepad);
        }
    }
}
