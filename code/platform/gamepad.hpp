#ifndef _LNA_PLATFORM_GAMEPAD_HPP_
#define _LNA_PLATFORM_GAMEPAD_HPP_

namespace lna
{
    constexpr size_t MAX_GAMEPAD_BUTTON_COUNT = 15;

    struct gamepad_api;

    struct gamepad_config
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

    void gamepad_init(
        gamepad_api& gamepad
        );

    void gamepad_open(
        gamepad_api& gamepad,
        const gamepad_config& config
        );

    void gamepad_update(
        gamepad_api& gamepad
        );

    const gamepad_info& gamepad_last_update_info(
        gamepad_api& gamepad
        );
    
    void gamepad_close(
        gamepad_api& gamepad
        );
}

#endif // _LNA_PLATFORM_GAMEPAD_HPP_
