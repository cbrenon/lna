#ifndef LNA_MATHS_LNA_VEC2_H
#define LNA_MATHS_LNA_VEC2_H

typedef union lna_vec2_u
{
    struct
    {
        float x;
        float y;
    };
    struct
    {
        float u;
        float v;
    };
    struct
    {
        float width;
        float height;
    };
} lna_vec2_t;

#endif
