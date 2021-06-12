#include <math.h>
#include "core/lna_assert.h"
#include "maths/lna_vec3.h"

void lna_vec3_normalize(lna_vec3_t* v)
{
    lna_assert(v)

    float square_length = (v->x * v->x) + (v->y * v->y) + (v->z * v->z);
    if (square_length > 0.0f)
    {
        float length        = sqrtf(square_length);
        float inv_length    = 1.0f / length;
        v->x *= inv_length;
        v->y *= inv_length;
        v->z *= inv_length;
    }
}

lna_vec3_t lna_vec3_cross_product(
    lna_vec3_t a,
    lna_vec3_t b
    )
{
    return (lna_vec3_t)
    {
        (a.y * b.z) - (a.z * b.y),
        (a.z * b.x) - (a.x * b.z),
        (a.x * b.y) - (a.y * b.x),
    };
}

float lna_vec3_dot_product(
    lna_vec3_t a,
    lna_vec3_t b
    )
{
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

lna_vec3_t lna_vec3_mult(lna_vec3_t v, float r)
{
    return (lna_vec3_t)
    {
        v.x * r,
        v.y * r,
        v.z * r,
    };
}

lna_vec3_t lna_vec3_div(lna_vec3_t v, float r)
{
    lna_assert(r != 0.0f)
    return (lna_vec3_t)
    {
        v.x / r,
        v.y / r,
        v.z / r,
    };
}

lna_vec3_t lna_vec3_add(lna_vec3_t a, lna_vec3_t b)
{
    return (lna_vec3_t)
    {
        a.x + b.x,
        a.y + b.y,
        a.z + b.z,
    };
}

lna_vec3_t lna_vec3_sub(lna_vec3_t a, lna_vec3_t b)
{
    return (lna_vec3_t)
    {
        a.x - b.x,
        a.y - b.y,
        a.z - b.z,
    };
}
