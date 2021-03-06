#include <string.h>
#include "backends/sdl/lna_input_sdl.h"
#include "core/lna_assert.h"

static const uint16_t LNA_SDL_KEYBOARD_MAPPING[LNA_KEY_COUNT] =
{
    SDL_SCANCODE_ESCAPE,
    SDL_SCANCODE_RETURN,
    SDL_SCANCODE_BACKSPACE,
    SDL_SCANCODE_PERIOD,
    SDL_SCANCODE_MINUS,
    SDL_SCANCODE_LEFT,
    SDL_SCANCODE_RIGHT,
    SDL_SCANCODE_UP,
    SDL_SCANCODE_DOWN,
    SDL_SCANCODE_0,
    SDL_SCANCODE_1,
    SDL_SCANCODE_2,
    SDL_SCANCODE_3,
    SDL_SCANCODE_4,
    SDL_SCANCODE_5,
    SDL_SCANCODE_6,
    SDL_SCANCODE_7,
    SDL_SCANCODE_8,
    SDL_SCANCODE_9,
    SDL_SCANCODE_F1,
    SDL_SCANCODE_F2,
};

void lna_input_init(lna_input_t* input)
{
    lna_assert(input)

    input->keyboard_state = SDL_GetKeyboardState(NULL);
}

lna_input_event_t lna_input_poll_events(lna_input_t* input)
{
    lna_assert(input)

    memcpy(
        input->prev_keyboard_state,
        input->keyboard_state,
        sizeof(input->prev_keyboard_state)
        );

    input->prev_mouse_state = input->mouse_state;

    lna_input_event_t   input_event = LNA_INPUT_EVENT_NONE;
    SDL_Event           sdl_event;

    SDL_JoystickEventState(SDL_DISABLE);
    while (SDL_PollEvent(&sdl_event))
    {
        switch (sdl_event.type)
        {
            case SDL_QUIT:
            {
                input_event = LNA_INPUT_EVENT_WINDOW_CLOSED;
                break;
            }
            case SDL_WINDOWEVENT:
            {
                switch (sdl_event.window.event)
                {
                    case SDL_WINDOWEVENT_CLOSE:
                    {
                        input_event = LNA_INPUT_EVENT_WINDOW_CLOSED;
                        break;
                    }
                    case SDL_WINDOWEVENT_RESIZED:
                    {
                        input_event = LNA_INPUT_EVENT_WINDOW_RESIZED;
                        break;
                    }
                }
                break;
            }
            case SDL_MOUSEMOTION:
            {
                input->mouse_state.x = sdl_event.motion.x;
                input->mouse_state.y = sdl_event.motion.y;
                break;
            }
            case SDL_MOUSEBUTTONUP:
            {
                switch(sdl_event.button.button)
                {
                    case SDL_BUTTON_LEFT:
                        input->mouse_state.buttons[LNA_MOUSE_BUTTON_LEFT] = false;
                        break;
                    case SDL_BUTTON_MIDDLE:
                        input->mouse_state.buttons[LNA_MOUSE_BUTTON_MIDDLE] = false;
                        break;
                    case SDL_BUTTON_RIGHT:
                        input->mouse_state.buttons[LNA_MOUSE_BUTTON_RIGHT] = false;
                        break;
                }
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
            {
                switch(sdl_event.button.button)
                {
                    case SDL_BUTTON_LEFT:
                        input->mouse_state.buttons[LNA_MOUSE_BUTTON_LEFT] = true;
                        break;
                    case SDL_BUTTON_MIDDLE:
                        input->mouse_state.buttons[LNA_MOUSE_BUTTON_MIDDLE] = true;
                        break;
                    case SDL_BUTTON_RIGHT:
                        input->mouse_state.buttons[LNA_MOUSE_BUTTON_RIGHT] = true;
                        break;
                }
                break;
            }
        }
    }

    return input_event;
}

bool lna_input_is_key_pressed(const lna_input_t* input, lna_key_t key)
{
    lna_assert(input)
    lna_assert(input->keyboard_state)
    lna_assert((size_t)key >= 0 && (size_t)key < sizeof(LNA_SDL_KEYBOARD_MAPPING) / sizeof(LNA_SDL_KEYBOARD_MAPPING[0]))

    return input->keyboard_state[LNA_SDL_KEYBOARD_MAPPING[key]] == 1;
}

bool lna_input_is_key_has_been_pressed(const lna_input_t* input, lna_key_t key)
{
    lna_assert(input)
    lna_assert(input->keyboard_state)
    lna_assert((size_t)key >= 0 && (size_t)key < sizeof(LNA_SDL_KEYBOARD_MAPPING) / sizeof(LNA_SDL_KEYBOARD_MAPPING[0]))

    return input->keyboard_state[LNA_SDL_KEYBOARD_MAPPING[key]] == 1 && input->prev_keyboard_state[LNA_SDL_KEYBOARD_MAPPING[key]] == 0;
}

const lna_mouse_state_t* lna_input_mouse_state(const lna_input_t* input)
{
    lna_assert(input)
    return &input->mouse_state;
}

const lna_mouse_state_t* lna_input_prev_mouse_state(const lna_input_t* input)
{
    lna_assert(input)
    return &input->prev_mouse_state;
}
