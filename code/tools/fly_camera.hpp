#ifndef _LNA_TOOLS_FLY_CAMERA_HPP_
#define _LNA_TOOLS_FLY_CAMERA_HPP_

#include <cstdint>
#include <array>
#include "maths/vec3.hpp"
#include "maths/mat4.hpp"
#include "backends/input_backend.hpp"

namespace lna
{
    constexpr uint8_t FLY_CAMERA_DIRECTION_COUNT = 4;

    class fly_camera
    {
        public:

            enum class direction
            {
                FORWARD,
                BACKWARD,
                LEFT,
                RIGHT
            };

            using direction_array = std::array<key, FLY_CAMERA_DIRECTION_COUNT>;

            fly_camera() = default;
            ~fly_camera() = default;

            void init(
                const vec3& position,
                const vec3& up,
                float yaw,
                float pitch,
                float move_speed,
                float mouse_sensitivity,
                float zoom
                );

            void set_key_mapping(
                key direction_key,
                direction direction_value 
                );

            void update(
                input_backend& input,
                double dtime
                );

            inline const vec3&  position()      const { return _position;       }
            inline const mat4&  view_matrix()   const { return _view_matrix;    }
            inline float        zoom()          const { return _zoom;           }

        private:

            void update_camera_vectors();

            float           _yaw;
            float           _pitch;
            float           _move_speed;
            float           _mouse_sensitivity;
            float           _zoom;
            vec3            _front;
            vec3            _up;
            vec3            _right;
            vec3            _world_up;
            vec3            _position;
            mat4            _view_matrix;
            direction_array _direction_key_mapping  { key::KEY_UNKOWN };
    };
}

#endif // _LNA_TOOLS_FLY_CAMERA_HPP_
