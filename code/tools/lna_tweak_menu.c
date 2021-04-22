#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include "tools/lna_tweak_menu.h"
#include "core/lna_container.h"
#include "core/lna_memory_pool.h"
#include "core/lna_assert.h"
#include "core/lna_string.h"
#include "backends/lna_input.h"
#include "backends/lna_ui.h"
#include "backends/lna_texture.h"

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
    lna_tweak_menu_node_t*      root;
    lna_tweak_menu_node_t*      cur_page;
    lna_tweak_menu_node_t*      cur_page_item;
    bool                        edit_mode;
    uint32_t                    edit_char_index;
} lna_tweak_menu_navigation_t;

typedef struct lna_tweak_menu_graphics_s
{
    lna_ui_buffer_t*            buffer;
    float                       font_size;
    float                       leading;
    float                       spacing;
    lna_vec2_t                  viewport_pos;
    lna_vec2_t                  viewport_size;
    lna_vec2_t                  uv_char_size;
    uint32_t                    font_texture_col_count;
    uint32_t                    font_texture_row_count;
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

static const uint32_t   LNA_TWEAK_MENU_MAX_NODE_COUNT           = 1000;
static const uint32_t   LNA_TWEAK_MENU_MAX_BUFFER_VERTEX_COUNT  = 1280;
static const uint32_t   LNA_TWEAK_MENU_MAX_BUFFER_INDEX_COUNT   = 1280;

void lna_tweak_menu_init(const lna_tweak_menu_config_t* config)
{
    lna_assert(config)
    lna_assert(g_tweak_menu == NULL)

    g_tweak_menu = lna_memory_alloc(
        config->memory_pool,
        lna_tweak_menu_t,
        1
        );

    g_tweak_menu->node_pool.max_node_count  = config->max_node_count == 0 ? LNA_TWEAK_MENU_MAX_NODE_COUNT : config->max_node_count;
    g_tweak_menu->node_pool.nodes           = lna_memory_alloc(
        config->memory_pool,
        lna_tweak_menu_node_t,
        g_tweak_menu->node_pool.max_node_count
        );
    for (uint32_t i = 0; i < g_tweak_menu->node_pool.max_node_count; ++i)
    {
        g_tweak_menu->node_pool.nodes[i].type           = LNA_TWEAK_MENU_NODE_TYPE_UNKNOWN;
        g_tweak_menu->node_pool.nodes[i].name[0]        = '\0';
        g_tweak_menu->node_pool.nodes[i].edit_buffer[0] = '\0';
    }

    g_tweak_menu->navigation.edit_mode  = false;

    //g_tweak_menu->graphics.buffer                   = config->ui_buffer;
    g_tweak_menu->graphics.font_size                = config->font_size;
    g_tweak_menu->graphics.leading                  = config->leading;
    g_tweak_menu->graphics.spacing                  = config->spacing;
    g_tweak_menu->graphics.font_texture_col_count   = lna_texture_atlas_col_count(config->font_texture);
    g_tweak_menu->graphics.font_texture_row_count   = lna_texture_atlas_row_count(config->font_texture);
    g_tweak_menu->graphics.uv_char_size             = (lna_vec2_t)
    {
        //! we divide by texture width and height again to have normalized uv data
        ((float)(g_tweak_menu->graphics.font_texture_col_count) / (float)(lna_texture_width(config->font_texture))),
        ((float)(g_tweak_menu->graphics.font_texture_row_count) / (float)(lna_texture_height(config->font_texture))),
    };

    g_tweak_menu->graphics.buffer = lna_ui_system_new_buffer(
        config->ui_system,
        &(lna_ui_buffer_config_t)
        {
            .memory_pool = config->memory_pool,
            .max_vertex_count = config->max_buffer_vertex_count == 0 ? LNA_TWEAK_MENU_MAX_BUFFER_VERTEX_COUNT : config->max_buffer_vertex_count,
            .max_index_count = config->max_buffer_index_count == 0 ? LNA_TWEAK_MENU_MAX_BUFFER_INDEX_COUNT : config->max_buffer_index_count,
            .texture = config->font_texture,
        }
        );
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

    if (
        lna_input_is_key_has_been_pressed(input, LNA_TWEAK_MENU_ACTION_MAPPING[LNA_TWEAK_MENU_ACTION_GO_TO_PARENT])
        && nav->cur_page->parent
        && !nav->edit_mode
        )
    {
        nav->cur_page = nav->cur_page->parent;
        nav->cur_page_item = nav->cur_page->first_child;
    }
    else if (
        lna_input_is_key_has_been_pressed(input, LNA_TWEAK_MENU_ACTION_MAPPING[LNA_TWEAK_MENU_ACTION_GO_TO_CHILD])
        && lna_tweak_menu_node_has_child(nav->cur_page_item)
        && !nav->edit_mode
        )
    {
        nav->cur_page = nav->cur_page_item;
        nav->cur_page_item = nav->cur_page->first_child;
    }
    else if (
        lna_input_is_key_has_been_pressed(input, LNA_TWEAK_MENU_ACTION_MAPPING[LNA_TWEAK_MENU_ACTION_GO_TO_NEXT])
        && nav->cur_page_item->next_sibling
        && !nav->edit_mode
        )
    {
        nav->cur_page_item = nav->cur_page_item->next_sibling;
    }
    else if (
        lna_input_is_key_has_been_pressed(input, LNA_TWEAK_MENU_ACTION_MAPPING[LNA_TWEAK_MENU_ACTION_GO_TO_PREV])
        && nav->cur_page_item->prev_sibling
        && !nav->edit_mode
        )
    {
        nav->cur_page_item = nav->cur_page_item->prev_sibling;
    }
    else if (
        lna_input_is_key_has_been_pressed(input, LNA_TWEAK_MENU_ACTION_MAPPING[LNA_TWEAK_MENU_ACTION_EDIT])
        && !nav->edit_mode
        && lna_tweak_menu_node_is_editable(nav->cur_page_item)
        )
    {
        if (nav->cur_page_item->type == LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_BOOL)
        {
            lna_assert(nav->cur_page_item->var_ptr)
            //! for boolean, there is no edit mode, when user press enter it changes automatically the boolean value to the opposite.
            *((bool*)nav->cur_page_item->var_ptr) = !(*((bool*)nav->cur_page_item->var_ptr));
        }
        else
        {
            nav->edit_mode = true;
            nav->edit_char_index = 0;
            nav->cur_page_item->edit_buffer[0] = '\0';
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
        && lna_tweak_menu_node_is_editable(nav->cur_page_item)
        )
    {
        lna_assert(nav->cur_page_item->var_ptr)

        if (nav->edit_char_index > 0)
        {
            switch (nav->cur_page_item->type)
            {
                case LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_INT:
                    lna_string_to_int((int32_t*)nav->cur_page_item->var_ptr, nav->cur_page_item->edit_buffer);
                    break;
                case LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_UNSIGNED_INT:
                    lna_string_to_uint((uint32_t*)nav->cur_page_item->var_ptr, nav->cur_page_item->edit_buffer);
                    break;
                case LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_FLOAT:
                    lna_string_to_float((float*)nav->cur_page_item->var_ptr, nav->cur_page_item->edit_buffer);
                    break;
                case LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_DOUBLE:
                    lna_string_to_double((double*)nav->cur_page_item->var_ptr, nav->cur_page_item->edit_buffer);
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
            nav->cur_page_item->edit_buffer[nav->edit_char_index] = '\0';
        }
    }
    else if (
        nav->edit_mode
        && nav->edit_char_index < (LNA_TWEAK_MENU_NODE_EDIT_BUFFER_MAX_LENGTH - 1)
        )
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-fallthrough"
        switch (nav->cur_page_item->type)
        {
            case LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_FLOAT:
                {
                    if (lna_input_is_key_has_been_pressed(input, LNA_KEY_PERIOD))
                    {
                        bool period_already_added = false;
                        for (size_t i = 0; i < nav->edit_char_index; ++i)
                        {
                            if (nav->cur_page_item->edit_buffer[i] == '.')
                            {
                                period_already_added = true;
                            }
                        }
                        if (!period_already_added)
                        {
                            nav->cur_page_item->edit_buffer[nav->edit_char_index++] = '.';
                            nav->cur_page_item->edit_buffer[nav->edit_char_index] = '\0';
                        }
                    }
                }
                //! no break point here => need code below
            case LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_DOUBLE:
                //! no break point here => need code below
            case LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_INT:
                {
                    if (
                        lna_input_is_key_has_been_pressed(input, LNA_KEY_MINUS)
                        && nav->edit_char_index == 0
                        )
                    {
                        nav->cur_page_item->edit_buffer[nav->edit_char_index++] = '-';
                        nav->cur_page_item->edit_buffer[nav->edit_char_index] = '\0';
                    }
                }
                //! no break point here => need code below
            case LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_UNSIGNED_INT:
                {
                    for (int i = 0; i <= 0; ++i)
                    {
                        if (lna_input_is_key_has_been_pressed(input, LNA_KEY_0 + i))
                        {
                            nav->cur_page_item->edit_buffer[nav->edit_char_index++] = (char)i + '0';
                            nav->cur_page_item->edit_buffer[nav->edit_char_index] = '\0';
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
#pragma clang diagnostic pop
    }

    //! not the most efficient way to update current item buffers with their corresponding var values but it works.
    //! I will take the time later to find a better way if I have performance issue here.
    lna_tweak_menu_node_t* node = nav->cur_page->first_child;
    while (node)
    {
        if (
            (!nav->edit_mode || node != nav->cur_page_item)
            && lna_tweak_menu_node_is_editable(node)
            )
        {
            switch (node->type)
            {
                case LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_INT:
                    snprintf(node->edit_buffer, sizeof node->edit_buffer, "%" PRIi32, *((int32_t*)node->var_ptr));
                    break;
                case LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_UNSIGNED_INT:
                    snprintf(node->edit_buffer, sizeof node->edit_buffer, "%" PRIu32, *((uint32_t*)node->var_ptr));
                    break;
                case LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_FLOAT:
                    // TODO: find a better solution that this ugly cast sequence
                    //! it is fucking ugly I know. It seems that dealing with float and *printf family functions
                    //! is hard. I will see later to find a more elegant solution (later == never ?)
                    snprintf(node->edit_buffer, sizeof node->edit_buffer, "%g", (double)(*((float*)node->var_ptr)));
                    break;
                case LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_DOUBLE:
                    snprintf(node->edit_buffer, sizeof node->edit_buffer, "%g", *((double*)node->var_ptr));
                    break;
                case LNA_TWEAK_MENU_NODE_TYPE_VAR_VALUE_BOOL:
                    snprintf(node->edit_buffer, sizeof node->edit_buffer, "%s", *((bool*)node->var_ptr) ? "true" : "false");
                    break;
                case LNA_TWEAK_MENU_NODE_TYPE_UNKNOWN:
                case LNA_TWEAK_MENU_NODE_TYPE_PAGE:
                case LNA_TWEAK_MENU_NODE_TYPE_VAR:
                    lna_assert(0)
            }
        }
        node = node->next_sibling;
    }
}

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

static const lna_vec4_t LNA_TWEAK_MENU_COLORS[LNA_TWEAK_MENU_ELEMENT_COLOR_COUNT] =
{
    { 0.1f, 1.0f, 0.1f, 1.0f }, // LNA_TWEAK_MENU_ELEMENT_COLOR_OUTLINE
    { 0.0f, 0.0f, 0.0f, 1.0f }, // LNA_TWEAK_MENU_ELEMENT_COLOR_TITLE_BAR
    { 0.2f, 1.0f, 0.2f, 1.0f }, // LNA_TWEAK_MENU_ELEMENT_COLOR_TITLE_BAR_TEXT
    { 0.2f, 0.7f, 0.2f, 1.0f }, // LNA_TWEAK_MENU_ELEMENT_COLOR_BODY
    { 0.1f, 1.0f, 0.1f, 1.0f }, // LNA_TWEAK_MENU_ELEMENT_COLOR_BODY_TEXT
    { 0.9f, 1.0f, 0.9f, 1.0f }, // LNA_TWEAK_MENU_ELEMENT_COLOR_BODY_TEXT_FOCUSED
    { 0.1f, 0.1f, 0.1f, 1.0f }, // LNA_TWEAK_MENU_ELEMENT_COLOR_VALUE
    { 0.8f, 1.0f, 0.8f, 1.0f }, // LNA_TWEAK_MENU_ELEMENT_COLOR_VALUE_TEXT
};

static const float LNA_TWEAK_MENU_OUTLINE_SIZE  = 1.0f;
static const float LNA_TWEAK_MENU_PADDING       = 2.0f;

void lna_tweak_menu_update(void)
{
    //? Drawing here is just rebuild the buffer from scratch using the current page node information.
    //? It is done in N part:
    //? 1. empty the buffer to begin from scratch
    //? 2. calculate all window elements sizes and positions
    //? 3. create new rect for the outline of the window
    //? 4. create new rect for the title bar
    //? 5. create new text for the title bar
    //? 6. create new rect for the body
    //? 7. for all children nodes in page:
    //?     7.1. create new text for the node name
    //?     7.2. if editable create new rect for node edit buffer
    //?     7.3. if editable create new text for node edit buffer

    //?  |========================================|     -|-            -|-
    //?  |                                        |      | => TITLE     |
    //?  |========================================|     -|-             |
    //?  |                                        |      |              |
    //?  |                                        |      |              | => WINDOW
    //?  |                                        |      | => BODY      |
    //?  |                                        |      |              |
    //?  |                                        |      |              |
    //?  |========================================|     -|-            -|-

    lna_assert(g_tweak_menu)

    lna_ui_buffer_empty(g_tweak_menu->graphics.buffer);

    lna_tweak_menu_node_t*      page        = g_tweak_menu->navigation.cur_page;
    if (!page) return;

    lna_tweak_menu_graphics_t*  graphics    = &g_tweak_menu->graphics;
    lna_ui_buffer_t*            buffer      = graphics->buffer;

    float min_horizontal_empty_space_size = 0.0f;
    min_horizontal_empty_space_size += LNA_TWEAK_MENU_OUTLINE_SIZE * 2.0f;  //? we add border left and right border size
    min_horizontal_empty_space_size += LNA_TWEAK_MENU_PADDING * 2.0f;       //? we add border left and right padding
    float width = min_horizontal_empty_space_size;
    width += (float)((int32_t)strlen(page->name)) * graphics->font_size; // TODO: find a better way than this double cast

    lna_tweak_menu_node_t*  child_node              = page->first_child;
    uint32_t                child_count             = 0;
    float                   max_value_name_length   = 0.0f;
    while (child_node)
    {
        float child_node_width = min_horizontal_empty_space_size + (float)((int32_t)strlen(child_node->name)) * graphics->font_size;  // TODO: find a better way than this double cast
        max_value_name_length = (child_node_width > max_value_name_length) ? child_node_width : max_value_name_length;
        
        child_node_width += lna_tweak_menu_node_is_editable(child_node) ? LNA_TWEAK_MENU_NODE_EDIT_BUFFER_MAX_LENGTH * graphics->font_size + LNA_TWEAK_MENU_PADDING * 4.0f : 0.0f;
        width = (child_node_width > width) ? child_node_width : width;

        child_node = child_node->next_sibling;
        ++child_count;
    }

    float height = 0.0f;
    height += LNA_TWEAK_MENU_OUTLINE_SIZE;                                                                          //? top outline
    height += graphics->font_size + 2.0f * LNA_TWEAK_MENU_PADDING;                                                  //? title bar height
    height += (float)child_count * (graphics->font_size + LNA_TWEAK_MENU_PADDING * 3.0f) + LNA_TWEAK_MENU_PADDING;  //? window body height
    height += LNA_TWEAK_MENU_OUTLINE_SIZE;                                                                          //? bottom outline

    lna_vec2_t window_pos =
    {
        (graphics->viewport_size.width - width) * 0.5f + graphics->viewport_pos.x,
        (graphics->viewport_size.height - height) * 0.5f + graphics->viewport_pos.y,
    };
    lna_vec2_t window_size =
    {
        width,
        height
    };
    lna_vec2_t window_title_bar_pos =
    {
        window_pos.x + LNA_TWEAK_MENU_OUTLINE_SIZE,
        window_pos.y + LNA_TWEAK_MENU_OUTLINE_SIZE,
    };
    lna_vec2_t window_title_bar_size =
    {
        width - LNA_TWEAK_MENU_OUTLINE_SIZE * 2.0f,
        graphics->font_size + LNA_TWEAK_MENU_PADDING * 2.0f,
    };
    lna_vec2_t window_title_bar_text_pos =
    {
        window_title_bar_pos.x + LNA_TWEAK_MENU_PADDING,
        window_title_bar_pos.y + LNA_TWEAK_MENU_PADDING,
    };
    lna_vec2_t window_body_pos =
    {
        window_title_bar_pos.x,
        window_title_bar_pos.y + window_title_bar_size.height,
    };
    lna_vec2_t window_body_size =
    {
        window_title_bar_size.x,
        window_size.height - window_title_bar_size.height -LNA_TWEAK_MENU_OUTLINE_SIZE * 2.0f,
    };

    lna_ui_buffer_push_rect(
        buffer,
        &(lna_ui_buffer_rect_config_t)
        {
            .position = &window_pos,
            .size = &window_size,
            .color = &LNA_TWEAK_MENU_COLORS[LNA_TWEAK_MENU_ELEMENT_COLOR_OUTLINE],
        }
        );

    lna_ui_buffer_push_rect(
        buffer,
        &(lna_ui_buffer_rect_config_t)
        {
            .position = &window_title_bar_pos,
            .size = &window_title_bar_size,
            .color = &LNA_TWEAK_MENU_COLORS[LNA_TWEAK_MENU_ELEMENT_COLOR_TITLE_BAR],
        }
        );

    lna_ui_buffer_push_text(
        buffer,
        &(lna_ui_buffer_text_config_t)
        {
            .text = page->name,
            .position = &window_title_bar_text_pos,
            .size = graphics->font_size,
            .color = &LNA_TWEAK_MENU_COLORS[LNA_TWEAK_MENU_ELEMENT_COLOR_TITLE_BAR_TEXT],
            .leading = graphics->leading,
            .spacing = graphics->spacing,
            .texture_col_char_count = graphics->font_texture_col_count,
            .texture_row_char_count = graphics->font_texture_row_count,
            .uv_char_size = &graphics->uv_char_size,
        }
        );

    lna_ui_buffer_push_rect(
        buffer,
        &(lna_ui_buffer_rect_config_t)
        {
            .position = &window_body_pos,
            .size = &window_body_size,
            .color = &LNA_TWEAK_MENU_COLORS[LNA_TWEAK_MENU_ELEMENT_COLOR_BODY],
        }
        );

    lna_vec2_t node_text_pos=
    {
        window_body_pos.x + LNA_TWEAK_MENU_PADDING,
        window_body_pos.y + LNA_TWEAK_MENU_PADDING * 2.0f,
    };

    lna_tweak_menu_navigation_t* nav = &g_tweak_menu->navigation;

    child_node = page->first_child;
    while (child_node)
    {
        const lna_vec4_t* text_color = (child_node == nav->cur_page_item) ? 
            &LNA_TWEAK_MENU_COLORS[LNA_TWEAK_MENU_ELEMENT_COLOR_BODY_TEXT_FOCUSED] : &LNA_TWEAK_MENU_COLORS[LNA_TWEAK_MENU_ELEMENT_COLOR_BODY_TEXT];

        lna_ui_buffer_push_text(
            buffer,
            &(lna_ui_buffer_text_config_t)
            {
                .text = child_node->name,
                .position = &node_text_pos,
                .size = graphics->font_size,
                .color = text_color,
                .leading = graphics->leading,
                .spacing = graphics->spacing,
                .texture_col_char_count = graphics->font_texture_col_count,
                .texture_row_char_count = graphics->font_texture_row_count,
                .uv_char_size = &graphics->uv_char_size,
            }
            );

        if (lna_tweak_menu_node_is_editable(child_node))
        {
            lna_vec2_t node_value_pos =
            {
                window_body_pos.x + max_value_name_length,
                node_text_pos.y - LNA_TWEAK_MENU_PADDING,
            };
            lna_vec2_t node_value_size =
            {
                LNA_TWEAK_MENU_NODE_EDIT_BUFFER_MAX_LENGTH * graphics->font_size + 2.0f * LNA_TWEAK_MENU_PADDING,
                graphics->font_size + 2.0f * LNA_TWEAK_MENU_PADDING
            };

            lna_ui_buffer_push_rect(
                buffer,
                &(lna_ui_buffer_rect_config_t)
                {
                    .position = &node_value_pos,
                    .size = &node_value_size,
                    .color = &LNA_TWEAK_MENU_COLORS[LNA_TWEAK_MENU_ELEMENT_COLOR_VALUE],
                }
                );

            node_value_pos.x += LNA_TWEAK_MENU_PADDING;
            node_value_pos.y += LNA_TWEAK_MENU_PADDING;

            lna_ui_buffer_push_text(
                buffer,
                &(lna_ui_buffer_text_config_t)
                {
                    .text = nav->edit_mode && nav->edit_char_index == 0 ? "|" : child_node->edit_buffer,
                    .position = &node_value_pos,
                    .size = graphics->font_size,
                    .color = &LNA_TWEAK_MENU_COLORS[LNA_TWEAK_MENU_ELEMENT_COLOR_VALUE_TEXT],
                    .leading = graphics->leading,
                    .spacing = graphics->spacing,
                    .texture_col_char_count = graphics->font_texture_col_count,
                    .texture_row_char_count = graphics->font_texture_row_count,
                    .uv_char_size = &graphics->uv_char_size,
                }
                );
        }

        node_text_pos.y += graphics->font_size + LNA_TWEAK_MENU_PADDING * 2.0f;
        child_node = child_node->next_sibling;
        node_text_pos.y += (child_node && lna_tweak_menu_node_is_editable(child_node)) ? LNA_TWEAK_MENU_PADDING : 0.0f;
    }
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
