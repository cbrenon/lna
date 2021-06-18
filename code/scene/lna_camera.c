#include "scene/lna_camera.h"
#include "core/lna_assert.h"

void lna_camera_init(lna_camera_t* camera, const lna_camera_config_t* config)
{
    lna_assert(camera)
    lna_assert(config)
    lna_assert(config->position)
    lna_assert(config->target_position)
    lna_assert(config->up_vector)

    camera->position        = *config->position;
    camera->target_position = *config->target_position;
    camera->up_vector       = *config->up_vector;
    lna_camera_update(camera);
}

void lna_camera_update(lna_camera_t* camera)
{
    lna_assert(camera)

    camera->view_matrix = lna_mat4_look_at(
        camera->position.x, camera->position.y, camera->position.z,                         //! EYE
        camera->target_position.x, camera->target_position.y, camera->target_position.z,    //! TARGET
        camera->up_vector.x, camera->up_vector.y, camera->up_vector.z                       //! UP
        );
}
