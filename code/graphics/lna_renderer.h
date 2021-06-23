#ifndef LNA_GRAPHICS_LNA_RENDERER_H
#define LNA_GRAPHICS_LNA_RENDERER_H

#include <stdbool.h>
#include <stdint.h>

typedef struct lna_renderer_s       lna_renderer_t;
typedef struct lna_window_s         lna_window_t;
typedef struct lna_heap_allocator_s lna_heap_allocator_t;

typedef struct lna_renderer_config_s
{
    const lna_window_t*     window;
    bool                    enable_api_diagnostic;
    lna_heap_allocator_t*   allocator;
    uint32_t                max_listener_count;
    size_t                  frame_mem_pool_size;        //! set to 0 to use default value
    size_t                  swap_chain_mem_pool_size;   //! set to 0 to use default value
    size_t                  persistent_mem_pool_size;   //! set to 0 to use default value
} lna_renderer_config_t;

extern bool     lna_renderer_init               (lna_renderer_t* renderer, const lna_renderer_config_t* config);
extern void     lna_renderer_begin_draw_frame   (lna_renderer_t* renderer, uint32_t window_width, uint32_t window_height);
extern void     lna_renderer_end_draw_frame     (lna_renderer_t* renderer, bool window_resized, uint32_t window_width, uint32_t window_height);
extern void     lna_renderer_wait_idle          (lna_renderer_t* renderer);
extern void     lna_renderer_release            (lna_renderer_t* renderer);

#endif
