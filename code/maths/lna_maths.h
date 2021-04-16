#ifndef LNA_MATHS_LNA_MATHS_H
#define LNA_MATHS_LNA_MATHS_H

#include <stdint.h>

extern const float LNA_PI;
extern const float LNA_PI_DIV_180;

typedef struct lna_degree_s { float value; } lna_degree_t;
typedef struct lna_radian_s { float value; } lna_radian_t;

extern lna_radian_t lna_degree_to_radian(lna_degree_t degree);
extern uint32_t     lna_clamp_uint32(uint32_t value, uint32_t min, uint32_t max);

#endif
