#ifndef _LNA_MATHS_MAT4_HPP_
#define _LNA_MATHS_MAT4_HPP_

#include "maths/vec3.hpp"

namespace lna
{
    struct mat4
    {
        float values[4][4];
    };

    void mat4_identity(
        mat4& m
        );

    void mat4_scale(
        mat4& m,
        float x,
        float y,
        float z
        );

    void mat4_translation(
        mat4& m,
        float x,
        float y,
        float z
        );

    void mat4_rotation_x(
        mat4& m,
        float degree_angle
        );

    void mat4_rotation_y(
        mat4& m,
        float degree_angle
        );

    void mat4_rotation_z(
        mat4& m,
        float degree_angle
        );

    void mat4_loot_at(
        mat4& m,
        const lna::vec3& eye,
        const lna::vec3& target,
        const lna::vec3& up
        );

    void mat4_perspective(
        mat4& m,
        float degree_fov,
        float aspect_ratio,
        float near_plane,
        float far_plane
        );
}

#endif // _LNA_MATHS_MAT4_HPP_
