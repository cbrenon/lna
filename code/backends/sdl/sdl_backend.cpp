#include <vulkan/vulkan.h>
#include "backends/sdl/sdl_backend.hpp"
#include "core/assert.hpp"
#include "core/memory_pool.hpp"
#include "imgui_impl_sdl.h"

namespace
{
    const uint16_t SDL_KEYBOARD_MAPPING[] =
    {
        SDL_SCANCODE_ESCAPE, // KEY_ESC
    };
}

namespace lna
{
    void window_backend_init(
        window_backend& window
        )
    {
        window.handle                   = nullptr;
        window.width                    = 0;
        window.height                   = 0;
        window.fullscreen               = false;
        window.display_index            = 0;
        window.extension_infos.names    = nullptr;
        window.extension_infos.count    = 0;
    }

    void window_backend_configure(
        window_backend& window,
        const window_backend_config& config
        )
    {
        LNA_ASSERT(window.handle == nullptr);
        LNA_ASSERT(config.persistent_mem_pool_ptr);

        {
            auto result = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_TIMER);
            LNA_ASSERT(result == 0);
        }

        window.display_index   = 0; // TODO: let player choose display if multiple monitor?
        window.fullscreen      = config.fullscreen;
        SDL_DisplayMode display_mode;
        SDL_GetCurrentDisplayMode(window.display_index, &display_mode);
        window.width    = config.fullscreen ? static_cast<uint32_t>(display_mode.w) : config.width;
        window.height   = config.fullscreen ? static_cast<uint32_t>(display_mode.h) : config.height;
            

        Uint32 window_create_flags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_VULKAN;
        window_create_flags |= config.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;

        window.handle = SDL_CreateWindow(
            config.title ? config.title : "LNA FRAMEWORK",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            window.width,
            window.height,
            window_create_flags
            );
        LNA_ASSERT(window.handle);

        {
            uint32_t extension_count = 0;
            auto result = SDL_Vulkan_GetInstanceExtensions(
                window.handle,
                &extension_count,
                nullptr
                );
            LNA_ASSERT(result == SDL_TRUE);

            window.extension_infos.count    = config.enable_validation_layers ? extension_count + 1 : extension_count;
            window.extension_infos.names    = (const char **)memory_pool_reserve_memory(
                *config.persistent_mem_pool_ptr,
                window.extension_infos.count * sizeof(const char*)
                );
            LNA_ASSERT(window.extension_infos.names);

            result = SDL_Vulkan_GetInstanceExtensions(
                window.handle,
                &extension_count,
                window.extension_infos.names
                );
            LNA_ASSERT(result == SDL_TRUE);
            if (config.enable_validation_layers)
            {
                window.extension_infos.names[extension_count] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
            }
        }
    }

    uint32_t window_backend_width(
        const window_backend& window
        )
    {
        return window.width;
    }

    uint32_t window_backend_height(
        const window_backend& window
        )
    {
        return window.height;
    }

    const window_extension_infos& window_backend_extensions(
        const window_backend& window
        )
    {
        return window.extension_infos;
    }

    void window_backend_resolution_info_update(
        window_backend& window
        )
    {
        SDL_DisplayMode display_mode;
        SDL_GetCurrentDisplayMode(window.display_index, &display_mode);
        window.width  = static_cast<uint32_t>(display_mode.w);
        window.height = static_cast<uint32_t>(display_mode.h);
    }

    void window_backend_release(
        window_backend& window
        )
    {
        if (window.handle)
        {
            SDL_DestroyWindow(window.handle);
            window.handle   = nullptr;
            window.width    = 0;
            window.height   = 0;
        }
        SDL_Quit();            
    }

    input_event input_backend_poll_events(
        input_backend& input
        )
    {
        input_event result = input_event::NONE;
        SDL_Event sdl_event;
        SDL_PumpEvents();
        const Uint8* keystate = SDL_GetKeyboardState(NULL);
        for (uint32_t i = 0; i < SDL_NUM_SCANCODES; ++i)
        {
            input.curr_frame_keyboard_state[i] = keystate[i];
        }
        SDL_JoystickEventState(SDL_DISABLE);
        while (SDL_PollEvent(&sdl_event))
        {
            ImGui_ImplSDL2_ProcessEvent(&sdl_event);
            switch (sdl_event.type)
            {
                case SDL_QUIT:
                {
                    result = input_event::WINDOW_CLOSED;
                    break;
                }
                case SDL_WINDOWEVENT:
                {
                    switch (sdl_event.window.event)
                    {
                        case SDL_WINDOWEVENT_CLOSE:
                        {
                            result = input_event::WINDOW_CLOSED;
                            break;
                        }
                        case SDL_WINDOWEVENT_RESIZED:
                        {
                            result = input_event::WINDOW_RESIZED;
                            break;
                        }
                    }
                }
            }
        }
        return result;
    }

    bool input_backend_is_key_pressed(
        input_backend& input,
        key k
        )
    {
        return input.curr_frame_keyboard_state[SDL_KEYBOARD_MAPPING[static_cast<size_t>(k)]] == 1;
    }

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

    void gamepad_backend_init(
        gamepad_backend& gamepad
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

    void gamepad_backend_open(
        gamepad_backend& gamepad,
        const gamepad_backend_config& config
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

    void gamepad_backend_update(
        gamepad_backend& gamepad
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
            gamepad_backend_close(gamepad);
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

    const gamepad_info& gamepad_backend_info(
        gamepad_backend& gamepad
        )
    {
        return gamepad.info;
    }
    
    void gamepad_backend_close(
        gamepad_backend& gamepad
        )
    {
        if (gamepad.device)
        {
            SDL_GameControllerClose(gamepad.device);
            gamepad_backend_init(gamepad);
        }
    }
}
