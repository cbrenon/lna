#ifndef _LNA_BACKENDS_INPUT_BACKEND_HPP_
#define _LNA_BACKENDS_INPUT_BACKEND_HPP_

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

    input_event input_backend_poll_events(
        input_backend& input
        );

    bool input_backend_is_key_pressed(
        input_backend& input,
        key k
        );
}

#endif // _LNA_BACKENDS_INPUT_BACKEND_HPP_