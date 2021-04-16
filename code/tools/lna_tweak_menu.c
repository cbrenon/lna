#include <string.h>
#include "tools/lna_tweak_menu.h"
#include "core/lna_container.h"
#include "core/lna_memory_pool.h"
#include "core/lna_assert.h"
#include "core/lna_string.h"
#include "backends/lna_input.h"

typedef enum lna_tweak_menu_node_type_e
{
    LNA_TWEAK_MENU_NODE_TYPE_UNKNOWN,
    LNA_TWEAK_MENU_NODE_TYPE_PAGE,
    LNA_TWEAK_MENU_NODE_TYPE_VAR,
    LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_INT,
    LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_UNSIGNED_INT,
    LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_FLOAT,
    LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_DOUBLE,
    LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_BOOL,
} lna_tweak_menu_node_type_t;

#define LNA_TWEAK_MENU_NODE_NAME_MAX_LENGTH         16
#define LNA_TWEAK_MENU_NODE_EDIT_BUFFER_MAX_LENGTH  12

typedef struct lna_tweak_menu_node_s
{
    lna_tweak_menu_node_type_t      type;
    struct lna_tweak_menu_node_s*   parent;
    struct lna_tweak_menu_node_s*   prev_sibling;
    struct lna_tweak_menu_node_s*   next_sibling;
    struct lna_tweak_menu_node_s*   first_child;
    char                            name[LNA_TWEAK_MENU_NODE_NAME_MAX_LENGTH];
    char                            edit_buffer[LNA_TWEAK_MENU_NODE_EDIT_BUFFER_MAX_LENGTH];
    void*                           var_ptr;
} lna_tweak_menu_node_t;

typedef struct lna_tweak_menu_node_pool_s
{
    lna_tweak_menu_node_t*          nodes;
    uint32_t                        max_node_count;
    uint32_t                        cur_node_count;
    lna_tweak_menu_node_t*          last_parent_node;
} lna_tweak_menu_node_pool_t;

typedef enum lna_tweak_menu_action_e
{
    LNA_TWEAK_MENU_ACTION_GO_TO_NEXT,
    LNA_TWEAK_MENU_ACTION_GO_TO_PREV,
    LNA_TWEAK_MENU_ACTION_GO_TO_PARENT,
    LNA_TWEAK_MENU_ACTION_GO_TO_CHILD,
    LNA_TWEAK_MENU_ACTION_EDIT,
    LNA_TWEAK_MENU_ACTION_EDIT_DEL_LAST_CHAR,
    LNA_TWEAK_MENU_ACTION_EDIT_CANCEL,
    LNA_TWEAK_MENU_ACTION_EDIT_VALIDATE,
    LNA_TWEAK_MENU_ACTION_COUNT,
} lna_tweak_menu_action_t;

static const lna_key_t LNA_TWEAK_MENU_ACTION_MAPPING[LNA_TWEAK_MENU_ACTION_COUNT] =
{
    LNA_KEY_DOWN,       // LNA_TWEAK_MENU_ACTION_GO_TO_NEXT
    LNA_KEY_UP,         // LNA_TWEAK_MENU_ACTION_GO_TO_PREV
    LNA_KEY_ESC,        // LNA_TWEAK_MENU_ACTION_GO_TO_PARENT
    LNA_KEY_ENTER,      // LNA_TWEAK_MENU_ACTION_GO_TO_CHILD
    LNA_KEY_ENTER,      // LNA_TWEAK_MENU_ACTION_EDIT
    LNA_KEY_BACKSPACE,  // LNA_TWEAK_MENU_ACTION_EDIT_DEL_LAST_CHAR
    LNA_KEY_ESC,        // LNA_TWEAK_MENU_ACTION_EDIT_CANCEL
    LNA_KEY_ENTER,      // LNA_TWEAK_MENU_ACTION_EDIT_VALIDATE
};

typedef struct lna_tweak_menu_navigation_s
{
    lna_tweak_menu_node_t*          root;
    lna_tweak_menu_node_t*          cur_page;
    lna_tweak_menu_node_t*          cur_page_item;
    bool                            edit_mode;
    uint32_t                        edit_char_index;
} lna_tweak_menu_navigation_t;

typedef enum lna_tweak_menu_element_color_e
{
    LNA_TWEAK_MENU_ELEMENT_COLOR_OUTLINE,
    LNA_TWEAK_MENU_ELEMENT_COLOR_TITLE_BAR,
    LNA_TWEAK_MENU_ELEMENT_COLOR_TITLE_BAR_TEXT,
    LNA_TWEAK_MENU_ELEMENT_COLOR_BODY,
    LNA_TWEAK_MENU_ELEMENT_COLOR_BODY_TEXT,
    LNA_TWEAK_MENU_ELEMENT_COLOR_BODY_TEXT_FOCUSED,
    LNA_TWEAK_MENU_ELEMENT_COLOR_VALUE,
    LNA_TWEAK_MENU_ELEMENT_COLOR_VALUE_TEXT,
    LNA_TWEAK_MENU_ELEMENT_COLOR_COUNT,
} lna_tweak_menu_element_color_t;

// static const lna_vec3_t lna_tweak_menu_colors[LNA_TWEAK_MENU_ELEMENT_COLOR_COUNT] =
// {
//     { 0.1f, 0.1f, 0.1f }, // LNA_TWEAK_MENU_ELEMENT_COLOR_OUTLINE
//     { 0.2f, 0.2f, 0.2f }, // LNA_TWEAK_MENU_ELEMENT_COLOR_TITLE_BAR
//     { 0.7f, 0.7f, 0.7f }, // LNA_TWEAK_MENU_ELEMENT_COLOR_TITLE_BAR_TEXT
//     { 0.4f, 0.4f, 0.4f }, // LNA_TWEAK_MENU_ELEMENT_COLOR_BODY
//     { 0.1f, 0.1f, 0.1f }, // LNA_TWEAK_MENU_ELEMENT_COLOR_BODY_TEXT
//     { 0.9f, 0.9f, 0.9f }, // LNA_TWEAK_MENU_ELEMENT_COLOR_BODY_TEXT_FOCUSED
//     { 0.1f, 0.1f, 0.1f }, // LNA_TWEAK_MENU_ELEMENT_COLOR_VALUE
//     { 0.8f, 0.8f, 0.8f }, // LNA_TWEAK_MENU_ELEMENT_COLOR_VALUE_TEXT
// };

typedef struct lna_tweak_menu_graphics_s
{
    float opacity;
} lna_tweak_menu_graphics_t;

typedef struct lna_tweak_menu_s
{
    lna_tweak_menu_node_pool_t  node_pool;
    lna_tweak_menu_navigation_t navigation;
    lna_tweak_menu_graphics_t   graphics;
} lna_tweak_menu_t;

static lna_tweak_menu_t* g_tweak_menu = NULL;

//! ============================================================================
//!                             LOCAL FUNCTIONS
//! ============================================================================

static lna_tweak_menu_node_t* lna_tweak_menu_new_node(
    const char* name,
    lna_tweak_menu_node_type_t type,
    lna_tweak_menu_node_t* parent,
    void* var_ptr
    )
{
    lna_assert(g_tweak_menu->node_pool.cur_node_count < g_tweak_menu->node_pool.max_node_count)

    lna_tweak_menu_node_t* node = &g_tweak_menu->node_pool.nodes[g_tweak_menu->node_pool.cur_node_count++];

    lna_string_copy(node->name, name, LNA_TWEAK_MENU_NODE_NAME_MAX_LENGTH);
    node->type      = type;
    node->var_ptr   = var_ptr;

    if (parent)
    {
        node->parent = parent;
        if (!parent->first_child)
        {
            parent->first_child = node;
        }
        else
        {
            lna_tweak_menu_node_t* node_iter = parent->first_child;
            while (node_iter->next_sibling)
            {
                node_iter = node_iter->next_sibling;
            }
            node_iter->next_sibling = node;
            node->prev_sibling      = node_iter;
        }
    }

    return node;
}

static bool lna_tweak_menu_node_has_child(const lna_tweak_menu_node_t* node)
{
    return node && node->first_child &&
        (
            node->type == LNA_TWEAK_MENU_NODE_TYPE_PAGE
            || node->type == LNA_TWEAK_MENU_NODE_TYPE_VAR
        );
}

static bool lna_tweak_menu_node_is_editable(const lna_tweak_menu_node_t* node)
{
    return node &&
        (
            node->type == LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_INT
            || node->type == LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_UNSIGNED_INT
            || node->type == LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_FLOAT
            || node->type == LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_DOUBLE
            || node->type == LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_BOOL
        );
}

//! ============================================================================
//!                          TWEAK MENU FUNCTIONS
//! ============================================================================

#define LNA_TWEAK_MENU_MAX_NODE_COUNT 1000

void lna_tweak_menu_init(const lna_tweak_menu_config_t* config)
{
    lna_assert(config)
    lna_assert(g_tweak_menu == NULL)

    g_tweak_menu = lna_memory_alloc(config->memory_pool, lna_tweak_menu_t, 1);

    uint32_t max_node_count                 = config->max_node_count == 0 ? LNA_TWEAK_MENU_MAX_NODE_COUNT : config->max_node_count;
    g_tweak_menu->node_pool.nodes           = lna_memory_alloc(config->memory_pool, lna_tweak_menu_node_t, max_node_count);
    g_tweak_menu->node_pool.max_node_count  = max_node_count;
    for (uint32_t i = 0; i < max_node_count; ++i)
    {
        g_tweak_menu->node_pool.nodes[i].type           = LNA_TWEAK_MENU_NODE_TYPE_UNKNOWN;
        g_tweak_menu->node_pool.nodes[i].name[0]        = '\0';
        g_tweak_menu->node_pool.nodes[i].edit_buffer[0] = '\0';
    }
    g_tweak_menu->navigation.edit_mode  = false;
    g_tweak_menu->graphics.opacity      = 1.0f;
}

void lna_tweak_menu_process_input(const lna_input_t* input)
{
    lna_assert(g_tweak_menu)
    lna_assert(input)

    lna_tweak_menu_navigation_t* nav = &g_tweak_menu->navigation;

    if (!nav->cur_page)
    {
        return;
    }
    if (!nav->cur_page_item)
    {
        nav->cur_page_item = nav->cur_page->first_child;
    }

    lna_tweak_menu_node_t* cur_page = nav->cur_page;
    lna_tweak_menu_node_t* cur_item = nav->cur_page_item;

    if (
        lna_input_is_key_has_been_pressed(input, LNA_TWEAK_MENU_ACTION_MAPPING[LNA_TWEAK_MENU_ACTION_GO_TO_PARENT])
        && cur_page->parent
        && !nav->edit_mode
        )
    {
        cur_page = cur_page->parent;
        cur_item = cur_page->first_child;
    }
    else if (
        lna_input_is_key_has_been_pressed(input, LNA_TWEAK_MENU_ACTION_MAPPING[LNA_TWEAK_MENU_ACTION_GO_TO_CHILD])
        && lna_tweak_menu_node_has_child(cur_item)
        && !nav->edit_mode
        )
    {
        cur_page = cur_item;
        cur_item = cur_page->first_child;
    }
    else if (
        lna_input_is_key_has_been_pressed(input, LNA_TWEAK_MENU_ACTION_MAPPING[LNA_TWEAK_MENU_ACTION_GO_TO_NEXT])
        && cur_item->next_sibling
        && !nav->edit_mode
        )
    {
        cur_item = cur_item->next_sibling;
    }
    else if (
        lna_input_is_key_has_been_pressed(input, LNA_TWEAK_MENU_ACTION_MAPPING[LNA_TWEAK_MENU_ACTION_GO_TO_PREV])
        && cur_item->prev_sibling
        && !nav->edit_mode
        )
    {
        cur_item = cur_item->prev_sibling;
    }
    else if (
        lna_input_is_key_has_been_pressed(input, LNA_TWEAK_MENU_ACTION_MAPPING[LNA_TWEAK_MENU_ACTION_EDIT])
        && !nav->edit_mode
        && lna_tweak_menu_node_is_editable(cur_item)
        )
    {
        if (cur_item->type == LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_BOOL)
        {
            lna_assert(cur_item->var_ptr)
            //! for boolean, there is no edit mode, when user press enter it changes automatically the boolean value to the opposite.
            *((bool*)cur_item->var_ptr) = !(*((bool*)cur_item->var_ptr));
        }
        else
        {
            nav->edit_mode = true;
            nav->edit_char_index = 0;
            cur_item->edit_buffer[0] = '\0';
        }
    }
    else if (
        lna_input_is_key_has_been_pressed(input, LNA_TWEAK_MENU_ACTION_MAPPING[LNA_TWEAK_MENU_ACTION_EDIT_CANCEL])
        && nav->edit_mode
        )
    {
        nav->edit_mode = false;
    }
    else if (
        lna_input_is_key_has_been_pressed(input, LNA_TWEAK_MENU_ACTION_MAPPING[LNA_TWEAK_MENU_ACTION_EDIT_VALIDATE])
        && nav->edit_mode
        && lna_tweak_menu_node_is_editable(cur_item)
        )
    {
        lna_assert(cur_item->var_ptr)

        if (nav->edit_char_index > 0)
        {
            switch (cur_item->type)
            {
                case LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_INT:
                    lna_string_to_int((int32_t*)cur_item->var_ptr, cur_item->edit_buffer);
                    break;
                case LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_UNSIGNED_INT:
                    lna_string_to_uint((uint32_t*)cur_item->var_ptr, cur_item->edit_buffer);
                    break;
                case LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_FLOAT:
                    lna_string_to_float((float*)cur_item->var_ptr, cur_item->edit_buffer);
                    break;
                case LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_DOUBLE:
                    lna_string_to_double((double*)cur_item->var_ptr, cur_item->edit_buffer);
                    break;
                case LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_BOOL:
                    break; //! we do nothing here! boolean are automatically modified when we press enter.
                case LNA_TWEAK_MENU_NODE_TYPE_UNKNOWN:
                case LNA_TWEAK_MENU_NODE_TYPE_PAGE:
                case LNA_TWEAK_MENU_NODE_TYPE_VAR:
                    lna_assert(0)
            }
        }
        nav->edit_mode = false;
    }
    else if (
        lna_input_is_key_has_been_pressed(input, LNA_TWEAK_MENU_ACTION_MAPPING[LNA_TWEAK_MENU_ACTION_EDIT_DEL_LAST_CHAR])
        && nav->edit_mode
        )
    {
        if (nav->edit_char_index > 0)
        {
            --nav->edit_char_index;
            cur_item->edit_buffer[nav->edit_char_index] = '\0';
        }
    }
    else if (
        nav->edit_mode
        && nav->edit_char_index < (LNA_TWEAK_MENU_NODE_EDIT_BUFFER_MAX_LENGTH - 1)
        )
    {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-fallthrough"
        switch (cur_item->type)
        {
            case LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_FLOAT:
                {
                    if (lna_input_is_key_has_been_pressed(input, LNA_KEY_PERIOD))
                    {
                        bool period_already_added = false;
                        for (size_t i = 0; i < nav->edit_char_index; ++i)
                        {
                            if (cur_item->edit_buffer[i] == '.')
                            {
                                period_already_added = true;
                            }
                        }
                        if (!period_already_added)
                        {
                            cur_item->edit_buffer[nav->edit_char_index++] = '.';
                            cur_item->edit_buffer[nav->edit_char_index] = '\0';
                        }
                    }
                }
            case LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_DOUBLE:
            case LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_INT:
                {
                    if (
                        lna_input_is_key_has_been_pressed(input, LNA_KEY_MINUS)
                        && nav->edit_char_index == 0
                        )
                    {
                        cur_item->edit_buffer[nav->edit_char_index++] = '-';
                        cur_item->edit_buffer[nav->edit_char_index] = '\0';
                    }
                }
            case LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_UNSIGNED_INT:
                {
                    for (int i = 0; i <= 0; ++i)
                    {
                        if (lna_input_is_key_has_been_pressed(input, LNA_KEY_0 + i))
                        {
                            cur_item->edit_buffer[nav->edit_char_index++] = (char)i + '0';
                            cur_item->edit_buffer[nav->edit_char_index] = '\0';
                        }
                    }
                }
                break;
            case LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_BOOL:
                break; //! we do nothing here! boolean are automatically modified when we press enter.
            case LNA_TWEAK_MENU_NODE_TYPE_UNKNOWN:
            case LNA_TWEAK_MENU_NODE_TYPE_PAGE:
            case LNA_TWEAK_MENU_NODE_TYPE_VAR:
                lna_assert(0)
        }
    }
#pragma clang diagnostic pop

}

void lna_tweak_menu_draw(void)
{

}

void lna_tweak_menu_release(void)
{
    g_tweak_menu = NULL; //TODO: take the time to see if it is the best way to "release"
}

//! ============================================================================
//!                           BUILD MENU FUNCTIONS
//! ============================================================================

void lna_tweak_menu_push_page(const char* name)
{
    lna_assert(g_tweak_menu)

    lna_tweak_menu_node_t* node = lna_tweak_menu_new_node(
        name,
        LNA_TWEAK_MENU_NODE_TYPE_PAGE,
        g_tweak_menu->node_pool.last_parent_node,
        NULL
        );

    g_tweak_menu->node_pool.last_parent_node    = node;
    g_tweak_menu->navigation.root               = !g_tweak_menu->navigation.root ? node : g_tweak_menu->navigation.root;
    g_tweak_menu->navigation.cur_page           = !g_tweak_menu->navigation.cur_page ? node : g_tweak_menu->navigation.cur_page;
}

void lna_tweak_menu_pop_page(void)
{
    lna_assert(g_tweak_menu)
    lna_assert(g_tweak_menu->node_pool.last_parent_node)

    g_tweak_menu->node_pool.last_parent_node = g_tweak_menu->node_pool.last_parent_node->parent;
}

void lna_tweak_menu_push_int_var(const char* var_name, int32_t* var_ptr)
{
    lna_assert(g_tweak_menu)
    lna_assert(var_ptr)

    lna_tweak_menu_node_t* node = lna_tweak_menu_new_node(
        var_name,
        LNA_TWEAK_MENU_NODE_TYPE_VAR,
        g_tweak_menu->node_pool.last_parent_node,
        (void*)var_ptr
        );
    lna_tweak_menu_new_node(
        "(int) value",
        LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_INT,
        node,
        (void*)var_ptr
        );
}

void lna_tweak_menu_push_unsigned_int_var(const char* var_name, uint32_t* var_ptr)
{
    lna_assert(g_tweak_menu)
    lna_assert(var_ptr)

    lna_tweak_menu_node_t* node = lna_tweak_menu_new_node(
        var_name,
        LNA_TWEAK_MENU_NODE_TYPE_VAR,
        g_tweak_menu->node_pool.last_parent_node,
        (void*)var_ptr
        );
    lna_tweak_menu_new_node(
        "(unsigned int) value",
        LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_UNSIGNED_INT,
        node,
        (void*)var_ptr
        );
}

void lna_tweak_menu_push_bool(const char* var_name, bool* var_ptr)
{
    lna_assert(g_tweak_menu)
    lna_assert(var_ptr)

    lna_tweak_menu_node_t* node = lna_tweak_menu_new_node(
        var_name,
        LNA_TWEAK_MENU_NODE_TYPE_VAR,
        g_tweak_menu->node_pool.last_parent_node,
        (void*)var_ptr
        );
    lna_tweak_menu_new_node(
        "(bool) value",
        LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_BOOL,
        node,
        (void*)var_ptr
        );
}

void lna_tweak_menu_push_float_var(const char* var_name, float* var_ptr)
{
    lna_assert(g_tweak_menu)
    lna_assert(var_ptr)

    lna_tweak_menu_node_t* node = lna_tweak_menu_new_node(
        var_name,
        LNA_TWEAK_MENU_NODE_TYPE_VAR,
        g_tweak_menu->node_pool.last_parent_node,
        (void*)var_ptr
        );
    lna_tweak_menu_new_node(
        "(float) value",
        LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_FLOAT,
        node,
        (void*)var_ptr
        );
}

void lna_tweak_menu_push_double_var(const char* var_name, double* var_ptr)
{
    lna_assert(g_tweak_menu)
    lna_assert(var_ptr)

    lna_tweak_menu_node_t* node = lna_tweak_menu_new_node(
        var_name,
        LNA_TWEAK_MENU_NODE_TYPE_VAR,
        g_tweak_menu->node_pool.last_parent_node,
        (void*)var_ptr
        );
    lna_tweak_menu_new_node(
        "(double) value",
        LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_DOUBLE,
        node,
        (void*)var_ptr
        );
}

void lna_tweak_menu_push_vec2_var(const char* var_name, lna_vec2_t* var_ptr)
{
    lna_assert(g_tweak_menu)
    lna_assert(var_ptr)

    lna_tweak_menu_node_t* node = lna_tweak_menu_new_node(
        var_name,
        LNA_TWEAK_MENU_NODE_TYPE_VAR,
        g_tweak_menu->node_pool.last_parent_node,
        (void*)var_ptr
        );
    lna_tweak_menu_new_node(
        "(float) x",
        LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_FLOAT,
        node,
        (void*)(&var_ptr->x)
        );
    lna_tweak_menu_new_node(
        "(float) y",
        LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_FLOAT,
        node,
        (void*)(&var_ptr->y)
        );
}

void lna_tweak_menu_push_vec3_var(const char* var_name, lna_vec3_t* var_ptr)
{
    lna_assert(g_tweak_menu)
    lna_assert(var_ptr)

    lna_tweak_menu_node_t* node = lna_tweak_menu_new_node(
        var_name,
        LNA_TWEAK_MENU_NODE_TYPE_VAR,
        g_tweak_menu->node_pool.last_parent_node,
        (void*)var_ptr
        );
    lna_tweak_menu_new_node(
        "(float) x",
        LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_FLOAT,
        node,
        (void*)(&var_ptr->x)
        );
    lna_tweak_menu_new_node(
        "(float) y",
        LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_FLOAT,
        node,
        (void*)(&var_ptr->y)
        );
    lna_tweak_menu_new_node(
        "(float) z",
        LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_FLOAT,
        node,
        (void*)(&var_ptr->z)
        );
}

void lna_tweak_menu_push_vec4_var(const char* var_name, lna_vec4_t* var_ptr)
{
    lna_assert(g_tweak_menu)
    lna_assert(var_ptr)

    lna_tweak_menu_node_t* node = lna_tweak_menu_new_node(
        var_name,
        LNA_TWEAK_MENU_NODE_TYPE_VAR,
        g_tweak_menu->node_pool.last_parent_node,
        (void*)var_ptr
        );
    lna_tweak_menu_new_node(
        "(float) x",
        LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_FLOAT,
        node,
        (void*)(&var_ptr->x)
        );
    lna_tweak_menu_new_node(
        "(float) y",
        LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_FLOAT,
        node,
        (void*)(&var_ptr->y)
        );
    lna_tweak_menu_new_node(
        "(float) z",
        LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_FLOAT,
        node,
        (void*)(&var_ptr->z)
        );
    lna_tweak_menu_new_node(
        "(float) w",
        LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_FLOAT,
        node,
        (void*)(&var_ptr->w)
        );
}
