#ifndef _LNA_MATHS_MATHS_HPP_
#define _LNA_MATHS_MATHS_HPP_

namespace lna
{
    constexpr float PI          = 3.14159265358979323846f;
    constexpr float PI_DIV_180  = PI / 180.0f;

    inline float degree_to_radian(float degree)
    {
        return degree * PI_DIV_180;
    }
}

#endif // _LNA_MATHS_MATHS_HPP_
