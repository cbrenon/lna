#include <cmath>
#include "maths/vec3.hpp"

void lna::vec3_normalize(
    lna::vec3& v
    )
{
    float square_length = (v.x * v.x) + (v.y * v.y) + (v.z * v.z);
    if (square_length > 0.0f)
    {
        float length        = sqrtf(square_length);
        float inv_length    = 1.0f / length;
        v.x *= inv_length;
        v.y *= inv_length;
        v.z *= inv_length;
    }
}

void lna::vec3_cross_product(
    const lna::vec3& a,
    const lna::vec3& b,
    lna::vec3& result
    )
{
    result =
    {
        (a.y * b.z) - (a.z * b.y),
        (a.z * b.x) - (a.x * b.z),
        (a.x * b.y) - (a.y * b.x),
    };
}

float lna::vec3_dot_product(
    const lna::vec3& a,
    const lna::vec3& b
    )
{
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}
