#ifndef LNA_MATHS_LNA_VEC3_H
#define LNA_MATHS_LNA_VEC3_H

typedef union lna_vec3_u
{
    struct
    {
        float x;
        float y;
        float z;
    };
    struct
    {
        float u;
        float v;
        float w;
    };
    struct
    {
        float r;
        float g;
        float b;
    };
} lna_vec3_t;

extern void         lna_vec3_normalize      (lna_vec3_t* v);
extern lna_vec3_t   lna_vec3_cross_product  (lna_vec3_t a, lna_vec3_t b);
extern float        lna_vec3_dot_product    (lna_vec3_t a, lna_vec3_t b);
extern lna_vec3_t   lna_vec3_mult           (lna_vec3_t v, float r);
extern lna_vec3_t   lna_vec3_div            (lna_vec3_t v, float r);
extern lna_vec3_t   lna_vec3_add            (lna_vec3_t a, lna_vec3_t b);
extern lna_vec3_t   lna_vec3_sub            (lna_vec3_t a, lna_vec3_t b);

#endif
