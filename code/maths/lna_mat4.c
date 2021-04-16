#include <math.h>
#include "maths/lna_mat4.h"
#include "maths/lna_vec3.h"

lan_mat4_t  lan_mat4_identity(void)
{
    lan_mat4_t m;
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
    return m;
}

lan_mat4_t lan_mat4_scale(float x, float y, float z)
{
    lan_mat4_t m = lan_mat4_identity();
    m.values[0][0] = x;
    m.values[1][1] = y;
    m.values[2][2] = z;
    return m;
}

lan_mat4_t lan_mat4_translation(float x, float y, float z)
{
    lan_mat4_t m = lan_mat4_identity();
    m.values[3][0] = x;
    m.values[3][1] = y;
    m.values[3][2] = z;
    return m;
}

lan_mat4_t lan_mat4_rotation_x(lna_degree_t angle)
{
    lan_mat4_t  m = lan_mat4_identity();
    float       c = cosf(lna_degree_to_radian(angle).value);
    float       s = sinf(lna_degree_to_radian(angle).value);
    m.values[1][1] = c;
    m.values[1][2] = s;
    m.values[2][1] = -s;
    m.values[2][2] = c;
    return m;
}

lan_mat4_t lan_mat4_rotation_y(lna_degree_t angle)
{
    lan_mat4_t  m = lan_mat4_identity();
    float       c = cosf(lna_degree_to_radian(angle).value);
    float       s = sinf(lna_degree_to_radian(angle).value);
    m.values[0][0] = c;
    m.values[0][2] = -s;
    m.values[2][0] = s;
    m.values[2][2] = c;
    return m;
}

lan_mat4_t lan_mat4_rotation_z(lna_degree_t angle)
{
    lan_mat4_t  m = lan_mat4_identity();
    float       c = cosf(lna_degree_to_radian(angle).value);
    float       s = sinf(lna_degree_to_radian(angle).value);
    m.values[0][0] = c;
    m.values[0][1] = s;
    m.values[1][0] = -s;
    m.values[1][1] = c;
    return m;
}

lan_mat4_t lan_mat4_look_at(float eye_x, float eye_y, float eye_z, float target_x, float target_y, float target_z, float up_x, float up_y, float up_z)
{
    lna_vec3_t z_vec =
    {
        target_x - eye_x,
        target_y - eye_y,
        target_z - eye_z
    };
    lna_vec3_normalize(&z_vec);

    lna_vec3_t x_vec = lna_vec3_cross_product(z_vec, (lna_vec3_t){ up_x, up_y, up_z});
    lna_vec3_normalize(&x_vec);

    lna_vec3_t y_vec = lna_vec3_cross_product(x_vec, z_vec);

    lan_mat4_t  m = lan_mat4_identity();
    m.values[0][0] = x_vec.x;
    m.values[1][0] = x_vec.y;
    m.values[2][0] = x_vec.z;
    m.values[3][0] = -lna_vec3_dot_product(x_vec, (lna_vec3_t){ eye_x, eye_y, eye_z});
    m.values[0][1] = y_vec.x;
    m.values[1][1] = y_vec.y;
    m.values[2][1] = y_vec.z;
    m.values[3][1] = -lna_vec3_dot_product(y_vec, (lna_vec3_t){ eye_x, eye_y, eye_z});
    m.values[0][2] = -z_vec.x;
    m.values[1][2] = -z_vec.y;
    m.values[2][2] = -z_vec.z;
    m.values[3][2] =  lna_vec3_dot_product(z_vec, (lna_vec3_t){ eye_x, eye_y, eye_z});
    return m;
}

lan_mat4_t lan_mat4_perspective(lna_degree_t fov, float aspect_ratio, float near_z, float far_z)
{
    const float f = tanf(lna_degree_to_radian(fov).value * 0.5f);

    lan_mat4_t  m = lan_mat4_identity();
    m.values[0][0] = 1.0f / (f * aspect_ratio);
    m.values[1][1] = -1.0f / (f);
    m.values[2][2] = far_z / (near_z - far_z);
    m.values[2][3] = -1.0f;
    m.values[3][2] = -(far_z * near_z) / (far_z - near_z);
    return m;
}
