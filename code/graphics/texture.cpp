#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-compare"
#pragma warning(push, 0)
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#pragma warning(pop)
#pragma clang diagnostic pop
#include "graphics/texture.hpp"
#include "core/assert.hpp"

void lna::texture_init(
    lna::texture& tex
    )
{
    tex.width       = 0;
    tex.height      = 0;
    tex.channels    = 0;
    tex.pixels      = nullptr;
}

void lna::texture_load_from_file(
    lna::texture& tex,
    const char* filename
    )
{
    LNA_ASSERT(tex.pixels == nullptr);

    int         texture_width;
    int         texture_height;
    int         texture_channels;
    tex.pixels = stbi_load(
        filename,
        &texture_width,
        &texture_height,
        &texture_channels,
        STBI_rgb_alpha
        );
    LNA_ASSERT(tex.pixels);
    tex.width       = static_cast<uint32_t>(texture_width);
    tex.height      = static_cast<uint32_t>(texture_height);
    tex.channels    = static_cast<uint32_t>(texture_channels);
}

void lna::texture_free_pixels(
    lna::texture& tex
    )
{
    if (tex.pixels)
    {
        stbi_image_free(tex.pixels);
        tex.pixels = nullptr;
    }
}