#include <math.h>
#include "tools/lna_free_camera.h"
#include "system/lna_input.h"
#include "core/lna_assert.h"
#include "maths/lna_maths.h"

static void lna_free_camera_update_vectors(lna_free_camera_t* free_camera)
{
    lna_assert(free_camera)

    float yaw_rad   = lna_degree_to_radian(free_camera->yaw).value;
    float pitch_rad = lna_degree_to_radian(free_camera->pitch).value;
    float cos_yaw   = cosf(yaw_rad);
    float sin_yaw   = sinf(yaw_rad);
    float cos_pitch = cosf(pitch_rad);
    float sin_pitch = sinf(pitch_rad);

    free_camera->front = (lna_vec3_t)
    {
        .x  = cos_yaw * cos_pitch,
        .y  = sin_pitch,
        .z  = sin_yaw * cos_pitch,
    };
    lna_vec3_normalize(&free_camera->front);

    free_camera->right  = lna_vec3_cross_product(free_camera->front, free_camera->world_up);
    free_camera->up     = lna_vec3_cross_product(free_camera->right, free_camera->front);
}

void lna_free_camera_init(lna_free_camera_t* free_camera, const lna_free_camera_config_t* config)
{
    lna_assert(free_camera)
    lna_assert(config)
    lna_assert(config->position)
    lna_assert(config->up)

    free_camera->position       = *config->position;
    free_camera->world_up       = *config->up;
    free_camera->yaw            = config->yaw;
    free_camera->pitch          = config->pitch;
    free_camera->speed          = config->speed;
    free_camera->sensitivity    = config->sensitivity;
    
    lna_free_camera_update_vectors(free_camera);

    free_camera->view_matrix    = lna_mat4_look_at(
        free_camera->position.x,
        free_camera->position.y,
        free_camera->position.z,
        free_camera->position.x + free_camera->front.x,
        free_camera->position.y + free_camera->front.y,
        free_camera->position.z + free_camera->front.z,
        free_camera->up.x,
        free_camera->up.y,
        free_camera->up.z
        );
}

void lna_free_camera_update(lna_free_camera_t* free_camera, const lna_input_t* input, double dtime_in_s)
{
    lna_assert(free_camera)
    lna_assert(input)

    float velocity = free_camera->speed * (float)dtime_in_s;
    if (lna_input_is_key_pressed(input, LNA_KEY_UP))    free_camera->position = lna_vec3_add(free_camera->position, lna_vec3_mult(free_camera->front, velocity));
    if (lna_input_is_key_pressed(input, LNA_KEY_DOWN))  free_camera->position = lna_vec3_add(free_camera->position, lna_vec3_mult(free_camera->front, -velocity));
    if (lna_input_is_key_pressed(input, LNA_KEY_LEFT))  free_camera->position = lna_vec3_add(free_camera->position, lna_vec3_mult(free_camera->right, -velocity));
    if (lna_input_is_key_pressed(input, LNA_KEY_RIGHT)) free_camera->position = lna_vec3_add(free_camera->position, lna_vec3_mult(free_camera->right, velocity));

    const lna_mouse_state_t* mouse_state        = lna_input_mouse_state(input);
    if (mouse_state->buttons[LNA_MOUSE_BUTTON_RIGHT])
    {
        const lna_mouse_state_t* prev_mouse_state   = lna_input_prev_mouse_state(input);
        float mouse_offset_x = (float)(mouse_state->x - prev_mouse_state->x) * free_camera->sensitivity;
        float mouse_offset_y = (float)(mouse_state->y - prev_mouse_state->y) * free_camera->sensitivity;
        free_camera->yaw.value      += mouse_offset_x;
        free_camera->pitch.value    -= mouse_offset_y;
        free_camera->pitch.value    = free_camera->pitch.value > 89.0f  ? 89.0f     : free_camera->pitch.value;
        free_camera->pitch.value    = free_camera->pitch.value < -89.0f ? -89.0f    : free_camera->pitch.value;
        lna_free_camera_update_vectors(free_camera);
    }

    // TODO: only do these things when needed
    free_camera->view_matrix    = lna_mat4_look_at(
        free_camera->position.x,
        free_camera->position.y,
        free_camera->position.z,
        free_camera->position.x + free_camera->front.x,
        free_camera->position.y + free_camera->front.y,
        free_camera->position.z + free_camera->front.z,
        free_camera->up.x,
        free_camera->up.y,
        free_camera->up.z
        );
}
