#include "tools/fly_camera.hpp"
#include "core/assert.hpp"
#include "maths/maths.hpp"

namespace lna
{
    void fly_camera::init(
        const vec3& position,
        const vec3& up,
        float yaw,
        float pitch,
        float move_speed,
        float mouse_sensitivity,
        float zoom
        )
    {
        _position           = position;
        _world_up           = up;
        _yaw                = yaw;
        _pitch              = pitch;
        _move_speed         = move_speed;
        _mouse_sensitivity  = mouse_sensitivity;
        _zoom               = zoom;

        update_camera_vectors();
    }

    void fly_camera::set_key_mapping(
        key direction_key,
        direction direction_value 
        )
    {
        LNA_ASSERT(static_cast<size_t>(direction_value) < _direction_key_mapping.size());

        _direction_key_mapping[static_cast<size_t>(direction_value)] = direction_key;
    }

    void fly_camera::update(
        input_backend& input,
        double dtime
        )
    {
        //! PROCESS MOUSE MOVE

        const mouse& mouse_info     = input_backend_mouse(input);
        if (mouse_info.middle_button_pressed)
        {
            int32_t mouse_pos_offset_x  = mouse_info.cur_pos_x - mouse_info.old_pos_x;
            int32_t mouse_pos_offset_y  = mouse_info.cur_pos_y - mouse_info.old_pos_y;

            if (
                mouse_pos_offset_x != 0
                || mouse_pos_offset_y != 0
                )
            {
                float mouse_offset_x = static_cast<float>(mouse_pos_offset_x) * _mouse_sensitivity;
                float mouse_offset_y = static_cast<float>(mouse_pos_offset_y) * _mouse_sensitivity;

                _yaw    += mouse_offset_x;
                _pitch  += -mouse_offset_y;
                _pitch  = _pitch > 89.0f ? 89.0f : _pitch;
                _pitch  = _pitch < -89.0f ? -89.0f : _pitch;

                update_camera_vectors();
            }
        }

        //! PROCESS MOUSE SCROLL

        _zoom -= static_cast<float>(mouse_info.wheel_y);
        _zoom = _zoom < 1.0f ? 1.0f : _zoom;
        _zoom = _zoom > 45.0f ? 45.0f : _zoom;

        //! PROCESS KEYBOARD INPUT

        float velocity = _move_speed * static_cast<float>(dtime);

        if (input_backend_is_key_pressed(input, _direction_key_mapping[static_cast<size_t>(direction::FORWARD)]))
        {
            _position.x += _front.x * velocity;
            _position.y += _front.y * velocity;
            _position.z += _front.z * velocity;
        }
        if (input_backend_is_key_pressed(input, _direction_key_mapping[static_cast<size_t>(direction::BACKWARD)]))
        {
            _position.x -= _front.x * velocity;
            _position.y -= _front.y * velocity;
            _position.z -= _front.z * velocity;
        }
        if (input_backend_is_key_pressed(input, _direction_key_mapping[static_cast<size_t>(direction::LEFT)]))
        {
            _position.x -= _right.x * velocity;
            _position.y -= _right.y * velocity;
            _position.z -= _right.z * velocity;
        }
        if (input_backend_is_key_pressed(input, _direction_key_mapping[static_cast<size_t>(direction::RIGHT)]))
        {
            _position.x += _right.x * velocity;
            _position.y += _right.y * velocity;
            _position.z += _right.z * velocity;
        }

        //! UPDATE VIEW MATRIX

        vec3 cam_front =
        {
            _position.x + _front.x,
            _position.y + _front.y,
            _position.z + _front.z
        };

        mat4_loot_at(
            _view_matrix,
            _position,
            cam_front,
            _up
            );
    }

    void fly_camera::update_camera_vectors()
    {
        vec3 cam_front =
        {
            cosf(degree_to_radian(_yaw)) * cosf(degree_to_radian(_pitch)),
            sinf(degree_to_radian(_pitch)),
            sinf(degree_to_radian(_yaw)) * cosf(degree_to_radian(_pitch)),
        };
        vec3_normalize(cam_front);
        _front = cam_front;
        vec3_cross_product(_front, _world_up, _right);
        vec3_cross_product(_right, _front, _up);
    }
}
