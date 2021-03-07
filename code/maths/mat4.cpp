#include <cmath>
#include "maths/mat4.hpp"
#include "maths/maths.hpp"

void lna::mat4_identity(
    lna::mat4& m
    )
{
    m.values[0][0] = 1.0f;
    m.values[0][1] = 0.0f;
    m.values[0][2] = 0.0f;
    m.values[0][3] = 0.0f;
    m.values[1][0] = 0.0f;
    m.values[1][1] = 1.0f;
    m.values[1][2] = 0.0f;
    m.values[1][3] = 0.0f;
    m.values[2][0] = 0.0f;
    m.values[2][1] = 0.0f;
    m.values[2][2] = 1.0f;
    m.values[2][3] = 0.0f;
    m.values[3][0] = 0.0f;
    m.values[3][1] = 0.0f;
    m.values[3][2] = 0.0f;
    m.values[3][3] = 1.0f;
}

void lna::mat4_scale(
    lna::mat4& m,
    float x,
    float y,
    float z
    )
{
    lna::mat4_identity(m);
    m.values[0][0] = x;
    m.values[1][1] = y;
    m.values[2][2] = z;
}

void lna::mat4_translation(
    lna::mat4& m,
    float x,
    float y,
    float z
    )
{
    lna::mat4_identity(m);
    m.values[3][0] = x;
    m.values[3][1] = y;
    m.values[3][2] = z;
}

void lna::mat4_rotation_x(
    lna::mat4& m,
    float degree_angle
    )
{
    float c = cosf(lna::degree_to_radian(degree_angle));
    float s = sinf(lna::degree_to_radian(degree_angle));
    lna::mat4_identity(m);
    m.values[1][1] = c;
    m.values[1][2] = s;
    m.values[2][1] = -s;
    m.values[2][2] = c;
}

void lna::mat4_rotation_y(
    lna::mat4& m,
    float degree_angle
    )
{
    float c = cosf(lna::degree_to_radian(degree_angle));
    float s = sinf(lna::degree_to_radian(degree_angle));
    lna::mat4_identity(m);
    m.values[0][0] = c;
    m.values[0][2] = -s;
    m.values[2][0] = s;
    m.values[2][2] = c;

}

void lna::mat4_rotation_z(
    lna::mat4& m,
    float degree_angle
    )
{
    float c = cosf(lna::degree_to_radian(degree_angle));
    float s = sinf(lna::degree_to_radian(degree_angle));
    lna::mat4_identity(m);
    m.values[0][0] = c;
    m.values[0][1] = s;
    m.values[1][0] = -s;
    m.values[1][1] = c;

}

void lna::mat4_loot_at(
    lna::mat4& m,
    const lna::vec3& eye,
    const lna::vec3& target,
    const lna::vec3& up
    )
{
    lna::mat4_identity(m);

    lna::vec3 z_vec =
    {
        target.x - eye.x,
        target.y - eye.y,
        target.z - eye.z,
    };
    lna::vec3_normalize(z_vec);

    lna::vec3 x_vec;
    lna::vec3_cross_product(z_vec, up, x_vec);
    lna::vec3_normalize(x_vec);

    lna::vec3 y_vec;
    lna::vec3_cross_product(x_vec, z_vec, y_vec);

    m.values[0][0] = x_vec.x;
    m.values[1][0] = x_vec.y;
    m.values[2][0] = x_vec.z;
    m.values[3][0] = -lna::vec3_dot_product(x_vec, eye);
    m.values[0][1] = y_vec.x;
    m.values[1][1] = y_vec.y;
    m.values[2][1] = y_vec.z;
    m.values[3][1] = -lna::vec3_dot_product(y_vec, eye);
    m.values[0][2] = -z_vec.x;
    m.values[1][2] = -z_vec.y;
    m.values[2][2] = -z_vec.z;
    m.values[3][2] =  lna::vec3_dot_product(z_vec, eye);
}

void lna::mat4_perspective(
    lna::mat4& m,
    float degree_fov,
    float aspect_ratio,
    float near_plane,
    float far_plane
    )
{
    lna::mat4_identity(m);
    float f = tanf(lna::degree_to_radian(degree_fov) / 2.0f);
    m.values[0][0] = 1.0f / (f * aspect_ratio);
    m.values[1][1] = -1.0f / (f); //! we use Vulkan by default where Y is inverted
    m.values[2][2] = -(far_plane + near_plane) / (far_plane - near_plane);
    m.values[2][3] = -1.0f;
    m.values[3][2] = -(2.0f * far_plane * near_plane) / (far_plane - near_plane);
}
