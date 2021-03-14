#include <cstdint>
#include "platform/sdl/sdl_input.hpp"

namespace
{
    const uint16_t SDL_KEYBOARD_MAPPING[] =
    {
        SDL_SCANCODE_ESCAPE, // KEY_ESC
    };
}

namespace lna
{
    template<>
    input_event input_poll_events<sdl_input>(
        sdl_input& input
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

    template<>
    bool input_is_key_pressed<sdl_input>(
        sdl_input& input,
        key k
        )
    {
        return input.curr_frame_keyboard_state[SDL_KEYBOARD_MAPPING[static_cast<size_t>(k)]] == 1;
    }
}