#include "maths/lna_maths.h"

const float LNA_PI          = 3.14159265358979323846f;
const float LNA_PI_DIV_180  = 3.14159265358979323846f / 180.0f;

lna_radian_t lna_degree_to_radian(lna_degree_t degree)
{
    return (lna_radian_t) { .value = LNA_PI_DIV_180 * degree.value };
}

uint32_t lna_clamp_uint32(uint32_t value, uint32_t min, uint32_t max)
{
    const uint32_t t = value < min ? min : value;
    return t > max ? max : t;
}
