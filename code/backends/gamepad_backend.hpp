#ifndef _LNA_BACKENDS_GAMEPAD_BACKEND_HPP_
#define _LNA_BACKENDS_GAMEPAD_BACKEND_HPP_

namespace lna
{
    constexpr size_t MAX_GAMEPAD_BUTTON_COUNT = 15;

    struct gamepad_backend;

    struct gamepad_backend_config
    {
        int     device_index;
        float   left_stick_axis_max_value;
        float   left_stick_axis_dead_zone;
    };

    struct gamepad_info
    {
        enum class button
        {
            A,
            B,
            X,
            Y,
            BACK,
            GUIDE,
            START,
            LEFT_STICK,
            RIGHT_STICK,
            LEFT_SHOULDER,
            RIGHT_SHOULDER,
            D_PAD_UP,
            D_PAD_DOWN,
            D_PAD_LEFT,
            D_PAD_RIGHT,
            COUNT,
            NONE,
        };

        enum class state
        {
            UNKNOWN,
            OPENED,
            CLOSED,
            REMOVED,
        };

        float   left_stick_axis_x;
        float   left_stick_axis_y;
        float   left_stick_axis_max_value;
        float   left_stick_axis_dead_zone;
        bool    buttons[MAX_GAMEPAD_BUTTON_COUNT];
        state   device_state;
    };

    int gamepad_count();

    void gamepad_backend_init(
        gamepad_backend& gamepad
        );

    void gamepad_backend_open(
        gamepad_backend& gamepad,
        const gamepad_backend_config& config
        );

    void gamepad_backend_update(
        gamepad_backend& gamepad
        );

    const gamepad_info& gamepad_backend_info(
        gamepad_backend& gamepad
        );
    
    void gamepad_backend_close(
        gamepad_backend& gamepad
        );
}

#endif // _LNA_BACKENDS_GAMEPAD_HPP_
