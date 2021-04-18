#ifndef LNA_BACKENDS_LNA_TEXTURE_H
#define LNA_BACKENDS_LNA_TEXTURE_H

#include <stdint.h>

typedef struct lna_renderer_s       lna_renderer_t;
typedef struct lna_texture_s        lna_texture_t;
typedef struct lna_texture_system_s lna_texture_system_t;
typedef struct lna_memory_pool_s    lna_memory_pool_t;

typedef enum lna_texture_format_e
{
    LNA_TEXTURE_FORMAT_R8G8B8A8_SRGB,
    LNA_TEXTURE_FORMAT_R8G8B8A8_UNORM,
} lna_texture_format_t;

typedef enum lna_texture_filter_e
{
    LNA_TEXTURE_FILTER_LINEAR,
    LNA_TEXTURE_FILTER_NEAREST,
} lna_texture_filter_t;

typedef enum lna_texture_mipmap_mode_e
{
    LNA_TEXTURE_MIPMAP_MODE_LINEAR,
    LNA_TEXTURE_MIPMAP_MODE_NEAREST,
} lna_texture_mipmap_mode_t;

typedef enum lna_texture_sampler_address_mode_e
{
    LNA_TEXTURE_SAMPLER_ADDRESS_MODE_REPEAT,
    LNA_TEXTURE_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
    LNA_TEXTURE_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
} lna_texture_sampler_address_mode_t;

typedef struct lna_texture_system_config_s
{
    uint32_t                        max_texture_count;
    lna_renderer_t*                 renderer;
    lna_memory_pool_t*              memory_pool;
} lna_texture_system_config_t;

typedef struct lna_texture_config_s
{
    lna_texture_format_t                format;
    lna_texture_filter_t                filter;
    lna_texture_mipmap_mode_t           mimap_mode;
    lna_texture_sampler_address_mode_t  u;
    lna_texture_sampler_address_mode_t  v;
    lna_texture_sampler_address_mode_t  w;
    const char*                         filename;
} lna_texture_config_t;

extern void             lna_texture_system_init         (lna_texture_system_t* texture_system, const lna_texture_system_config_t* config);
extern lna_texture_t*   lna_texture_system_new_texture  (lna_texture_system_t* texture_system);
extern void             lna_texture_system_release      (lna_texture_system_t* texture_system);

extern void             lna_texture_init                (lna_texture_t* texture, const lna_texture_config_t* config);
extern void             lna_texture_release             (lna_texture_t* texture);

#endif
