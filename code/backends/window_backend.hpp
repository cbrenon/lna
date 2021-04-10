#ifndef _LNA_BACKENDS_WINDOW_BACKEND_HPP_
#define _LNA_BACKENDS_WINDOW_BACKEND_HPP_

#include <cstdint>
#include "core/heap_array.hpp"

namespace lna
{
    class memory_pool;

    struct window_backend_config
    {
        const char*             title;
        uint32_t                width;
        uint32_t                height;
        bool                    fullscreen;
        bool                    enable_validation_layers;
        memory_pool*            persistent_mem_pool_ptr;
    };

    using window_extension_infos = heap_array<const char*>;

    struct window_backend;

    void window_backend_configure(
        window_backend& window,
        const window_backend_config& config
        );

    void window_backend_resolution_info_update(
        window_backend& window
        );

    uint32_t window_backend_width(
        const window_backend& window
        );

    uint32_t window_backend_height(
        const window_backend& window
        );

    const window_extension_infos& window_backend_extensions(
        const window_backend& window
        );

    void window_backend_release(
        window_backend& window
        );
}

#endif // _LNA_BACKENDS_WINDOW_BACKEND_HPP_
