#ifndef _LNA_PLATFORM_INPUT_HPP_
#define _LNA_PLATFORM_INPUT_HPP_

namespace lna
{
    enum class key
    {
        KEY_ESC = 0,
        KEY_COUNT,
    };

    enum class input_event
    {
        NONE,
        WINDOW_CLOSED,
        WINDOW_RESIZED,
    };

    struct input_api;

    input_event input_poll_events(
        input_api& input
        );

    bool input_is_key_pressed(
        input_api& input,
        key k
        );
}

#endif // _LNA_PLATFORM_INPUT_HPP_
