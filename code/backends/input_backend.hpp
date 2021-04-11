#ifndef _LNA_BACKENDS_INPUT_BACKEND_HPP_
#define _LNA_BACKENDS_INPUT_BACKEND_HPP_

#include <cstdint>

namespace lna
{
    enum class key
    {
        KEY_ESC,
        KEY_W,
        KEY_S,
        KEY_A,
        KEY_D,
        KEY_COUNT,
        KEY_UNKOWN,
    };

    enum class input_event
    {
        NONE,
        WINDOW_CLOSED,
        WINDOW_RESIZED,
    };

    struct input_backend;

    struct mouse
    {
        int32_t old_pos_x;
        int32_t old_pos_y;
        int32_t cur_pos_x;
        int32_t cur_pos_y;
        int32_t wheel_y;
        bool    left_button_pressed;
        bool    middle_button_pressed;
        bool    right_button_pressed;
    };

    void input_backend_init(
        input_backend& input
        );

    input_event input_backend_poll_events(
        input_backend& input
        );

    const mouse& input_backend_mouse(
        input_backend& input
        );

    bool input_backend_is_key_pressed(
        input_backend& input,
        key k
        );
}

#endif // _LNA_BACKENDS_INPUT_BACKEND_HPP_
