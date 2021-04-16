#ifndef LNA_MATHS_LNA_VEC4_H
#define LNA_MATHS_LNA_VEC4_H

#include "maths/lna_vec3.h"

typedef union lna_vec4_u
{
    struct
    {
        float x;
        float y;
        float z;
        float w;
    };
    struct
    {
        float r;
        float g;
        float b;
        float a;
    };
} lna_vec4_t;

#endif
