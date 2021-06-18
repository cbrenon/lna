#ifndef LNA_SCENE_LNA_CAMERA_H
#define LNA_SCENE_LNA_CAMERA_H

#include "maths/lna_vec3.h"
#include "maths/lna_mat4.h"

typedef struct lna_camera_s
{
    lna_vec3_t          position;
    lna_vec3_t          up_vector;
    lna_vec3_t          target_position;
    lna_mat4_t          view_matrix;
} lna_camera_t;

typedef struct lna_camera_config_s
{
    const lna_vec3_t*   position;
    const lna_vec3_t*   up_vector;
    const lna_vec3_t*   target_position;
} lna_camera_config_t;

extern void lna_camera_init     (lna_camera_t* camera, const lna_camera_config_t* config);
extern void lna_camera_update   (lna_camera_t* camera);

#endif
