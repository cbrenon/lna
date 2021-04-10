#ifndef _LNA_BACKENDS_TEXTURE_BACKEND_HPP_
#define _LNA_BACKENDS_TEXTURE_BACKEND_HPP_

#include <cstdint>

namespace lna
{
    class memory_pool;
    struct texture_backend;
    struct renderer_backend;
    struct texture;

    struct texture_backend_config
    {
        uint32_t            max_texture_count;
        renderer_backend*   renderer_backend_ptr;
        memory_pool*        persistent_memory_pool_ptr;
    };

    struct texture_config
    {
        enum class format
        {
            R8G8B8A8_SRGB,
            R8G8B8A8_UNORM,
        };

        enum class filter
        {
            LINEAR,
            NEAREST,
        };

        enum class mipmap_mode
        {
            LINEAR,
            NEAREST,
        };

        enum class sampler_address_mode
        {
            REPEAT,
            CLAMP_TO_EDGE,
            CLAMP_TO_BORDER,
        };

        format                  fmt;
        filter                  mag;
        filter                  min;
        mipmap_mode             mipmap;
        sampler_address_mode    u;
        sampler_address_mode    v;
        sampler_address_mode    w;

        //? CREATE FROM FILE:
        const char* filename;

        //? CREATE FROM BUFFER:
        unsigned char* pixels;
        uint32_t width;
        uint32_t height;
    };

    void texture_backend_configure(
        texture_backend& backend,
        texture_backend_config& config
        );

    texture* texture_backend_new_texture(
        texture_backend& backend,
        texture_config& config
        );
        
    void texture_backend_release(
        texture_backend& backend
        );
}

#endif // _LNA_BACKENDS_TEXTURE_BACKEND_HPP_
