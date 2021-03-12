#ifndef _LNA_GRAPHICS_VERTEX_HPP_
#define _LNA_GRAPHICS_VERTEX_HPP_

#include "maths/vec2.hpp"
#include "maths/vec4.hpp"

namespace lna
{
    struct vertex
    {
        vec2    position;
        vec4    color;
        vec2    uv;
    };
}

#endif // _LNA_GRAPHICS_VERTEX_HPP_
