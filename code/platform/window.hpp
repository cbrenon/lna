#ifndef _NLA_PLATFORM_WINDOW_HPP_
#define _NLA_PLATFORM_WINDOW_HPP_

#include <cstdint>

namespace lna
{
    struct memory_pool;

    struct window_config
    {
        const char*     title;
        uint32_t        width;
        uint32_t        height;
        bool            fullscreen;
        bool            enable_validation_layers;
        memory_pool*    persistent_pool_ptr;
    };

    struct window_extension_infos
    {
        const char**    names;
        uint32_t        count;
    };

    struct window_api;

    void window_configure(
        window_api& window,
        const window_config& config
        );

    void window_resolution_info_update(
        window_api& window
        );

    uint32_t window_width(
        const window_api& window
        );

    uint32_t window_height(
        const window_api& window
        );

    const window_extension_infos& window_extensions(
        const window_api& window
        );

    void window_release(
        window_api& window
        );
}

#endif // _NLA_PLATFORM_WINDOW_HPP_
