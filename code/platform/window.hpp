#ifndef _NLA_PLATFORM_WINDOW_HPP_
#define _NLA_PLATFORM_WINDOW_HPP_

#include <cstdint>
#include <vector>

namespace lna
{
    struct window_config
    {
        const char* title                       { nullptr   };
        uint32_t    width                       { 1920      };
        uint32_t    height                      { 1080      };
        bool        fullscreen                  { false     };
        bool        enable_validation_layers    { true      };
    };

    template<typename window_api>
    void window_init(
        window_api& window,
        const window_config& config
        );

    template<typename window_api>
    void window_resolution_info_update(
        window_api& window
        );

    template<typename window_api>
    uint32_t window_width(
        const window_api& window
        );

    template<typename window_api>
    uint32_t window_height(
        const window_api& window
        );

    template<typename window_api>
    const std::vector<const char*>& window_extensions(
        const window_api& window
        );

    template<typename window_api, typename window_api_handle>
    window_api_handle window_handle(
        window_api& window
        );

    template<typename window_api>
    void window_release(
        window_api& window
        );
}

#endif // _NLA_PLATFORM_WINDOW_HPP_
