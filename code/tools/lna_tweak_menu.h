#ifndef LNA_TOOLS_TWEAK_MENU_H
#define LNA_TOOLS_TWEAK_MENU_H

#include <stdint.h>
#include <stdbool.h>
#include "maths/lna_vec2.h"
#include "maths/lna_vec3.h"
#include "maths/lna_vec4.h"

typedef struct lna_input_s              lna_input_t;
typedef struct lna_memory_pool_s        lna_memory_pool_t;
typedef struct lna_texture_atlas_info_s lna_texture_atlas_info_t;
typedef struct lna_ui_system_s          lna_ui_system_t;
typedef struct lna_texture_s            lna_texture_t;

typedef struct lna_tweak_menu_config_s
{
    uint32_t                        max_buffer_vertex_count;    //! set to 0 to use default vertex capacity
    uint32_t                        max_buffer_index_count;     //! set to 0 to use default index capacity
    uint32_t                        max_node_count;             //! set to 0 to use default node capacity
    lna_memory_pool_t*              memory_pool;
    float                           font_size;
    float                           leading;
    float                           spacing;
    lna_ui_system_t*                ui_system;
    lna_texture_t*                  font_texture;
    lna_vec2_t*                     viewport_position;
    lna_vec2_t*                     viewport_size;
} lna_tweak_menu_config_t;

//! TWEAK MENU FUNCTIONS
extern void                     lna_tweak_menu_init                     (const lna_tweak_menu_config_t* config);
extern void                     lna_tweak_menu_process_input            (const lna_input_t* input);
extern void                     lna_tweak_menu_update                   (void);

//! BUILD MENU FUNCTIONS
extern void                     lna_tweak_menu_push_page                (const char* name);
extern void                     lna_tweak_menu_pop_page                 (void);
extern void                     lna_tweak_menu_push_int_var             (const char* var_name, int32_t* var_ptr);
extern void                     lna_tweak_menu_push_unsigned_int_var    (const char* var_name, uint32_t* var_ptr);
extern void                     lna_tweak_menu_push_bool                (const char* var_name, bool* var_ptr);
extern void                     lna_tweak_menu_push_float_var           (const char* var_name, float* var_ptr);
extern void                     lna_tweak_menu_push_double_var          (const char* var_name, double* var_ptr);
extern void                     lna_tweak_menu_push_vec2_var            (const char* var_name, lna_vec2_t* var_ptr);
extern void                     lna_tweak_menu_push_vec3_var            (const char* var_name, lna_vec3_t* var_ptr);
extern void                     lna_tweak_menu_push_vec4_var            (const char* var_name, lna_vec4_t* var_ptr);

#endif
