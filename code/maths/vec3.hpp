#ifndef _LNA_MATHS_VEC3_HPP_
#define _LNA_MATHS_VEC3_HPP_

namespace lna
{
    struct vec3
    {
        float x { 0.0f };
        float y { 0.0f };
        float z { 0.0f };
    };

    void vec3_normalize(
        vec3& v
        );

    void vec3_cross_product(
        const vec3& a,
        const vec3& b,
        vec3& result
        );

    float vec3_dot_product(
        const vec3& a,
        const vec3& b
        );
}

#endif // _LNA_MATHS_VEC3_HPP_
