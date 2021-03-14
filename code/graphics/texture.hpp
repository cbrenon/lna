#ifndef _LNA_GRAPHICS_TEXTURE_HPP_
#define _LNA_GRAPHICS_TEXTURE_HPP_

#include <cstdint>

namespace lna
{
    struct texture
    {
        uint32_t        width;
        uint32_t        height;
        uint32_t        channels;
        unsigned char*  pixels;
    };

    void texture_init(
        texture& tex
        );

    void texture_load_from_file(
        texture& tex,
        const char* filename
        );

    void texture_free_pixels(
        texture& tex
        );
}

#endif // _LNA_GRAPHICS_TEXTURE_HPP_
