#ifndef LNA_MATHS_LNA_MAT4_H
#define LNA_MATHS_LNA_MAT4_H

#include "maths/lna_maths.h"

typedef struct lna_mat4_s
{
    float values[4][4];
} lna_mat4_t;

extern lna_mat4_t   lna_mat4_identity       (void);
extern lna_mat4_t   lna_mat4_scale          (float x, float y, float z);
extern lna_mat4_t   lna_mat4_translation    (float x, float y, float z);
extern lna_mat4_t   lna_mat4_rotation_x     (lna_degree_t angle);
extern lna_mat4_t   lna_mat4_rotation_y     (lna_degree_t angle);
extern lna_mat4_t   lna_mat4_rotation_z     (lna_degree_t angle);
extern lna_mat4_t   lna_mat4_look_at        (float eye_x, float eye_y, float eye_z, float target_x, float target_y, float target_z, float up_x, float up_y, float up_z);
extern lna_mat4_t   lna_mat4_perspective    (lna_degree_t fov, float aspect_ratio, float near_z, float far_z);
extern void         lna_mat4_mult           (const lna_mat4_t* a, const lna_mat4_t* b, lna_mat4_t* result);

#endif
