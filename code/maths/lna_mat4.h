#ifndef LNA_MATHS_LNA_MAT4_H
#define LNA_MATHS_LNA_MAT4_H

#include "maths/lna_maths.h"

typedef struct lan_mat4_s
{
    float values[4][4];
} lan_mat4_t;

extern lan_mat4_t   lan_mat4_identity       (void);
extern lan_mat4_t   lan_mat4_scale          (float x, float y, float z);
extern lan_mat4_t   lan_mat4_translation    (float x, float y, float z);
extern lan_mat4_t   lan_mat4_rotation_x     (lna_degree_t angle);
extern lan_mat4_t   lan_mat4_rotation_y     (lna_degree_t angle);
extern lan_mat4_t   lan_mat4_rotation_z     (lna_degree_t angle);
extern lan_mat4_t   lan_mat4_look_at        (float eye_x, float eye_y, float eye_z, float target_x, float target_y, float target_z, float up_x, float up_y, float up_z);
extern lan_mat4_t   lan_mat4_perspective    (lna_degree_t fov, float aspect_ratio, float near_z, float far_z);

#endif
