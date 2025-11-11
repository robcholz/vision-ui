//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//

#ifndef VISION_UI_VISION_UI_ITEM_H
#define VISION_UI_VISION_UI_ITEM_H

#include <stdbool.h>

#include "vision_ui_draw_driver.h"

extern void vision_ui_font_set(void* font);

extern void* vision_ui_font_get();

extern bool vision_ui_exit_animation_is_finished();

extern void vision_ui_exit_animation_set_is_finished();

typedef struct vision_ui_info_bar_t {
    char* content;
    uint16_t span;

    float y_info_bar;
    float y_info_bar_trg;

    float w_info_bar;
    float w_info_bar_trg;

    bool is_running;
    uint32_t time_start;
    uint32_t time;
} vision_ui_info_bar_t;

extern vision_ui_info_bar_t VISION_UI_INFO_BAR;

extern void vision_ui_info_bar_push(char* content, uint16_t span);

typedef struct vision_ui_pop_up_t {
    char* content;
    uint16_t span;

    float y_pop_up;
    float y_pop_up_trg;

    float w_pop_up;
    float w_pop_up_trg;

    bool is_running;
    uint32_t time_start;
    uint32_t time;
} vision_ui_pop_up_t;

extern vision_ui_pop_up_t VISION_UI_POP_UP;

extern void vision_ui_pop_up_push(char* content, uint16_t span);

/*** 弹窗 ***/

/*** 列表项 ***/
#define VISION_UI_MAX_LIST_CHILD_NUM 10
#define VISION_UI_MAX_LIST_LAYER 10

/*** 信息栏 ***/
#define VISION_UI_INFO_BAR_HEIGHT 15
#define VISION_UI_INFO_BAR_WIDTH 20

/*** 弹窗 ***/
#define VISION_UI_POP_UP_HEIGHT 20
#define VISION_UI_POP_UP_WIDTH 20

#define VISION_UI_SCREEN_HEIGHT 64
#define VISION_UI_SCREEN_WIDTH 128

// timing
#define VISION_UI_LIST_TEXT_SCROLL_PAUSE_MS 1000
#define VISION_UI_LIST_TEXT_SCROLL_SPEED_PX_S 15
#define VISION_UI_LIST_SLIDER_VALUE_SCROLL_SPEED_PX_S 5
#define VISION_UI_LIST_SLIDER_VALUE_SCROLL_PAUSE_MS 1500

// paddings
#define VISION_UI_LIST_TITLE_TO_DISPLAY_TOP_PADDING 0
#define VISION_UI_LIST_TITLE_TO_FRAME_PADDING 4

#define VISION_UI_LIST_FRAME_BETWEEN_PADDING 2

#define VISION_UI_LIST_FOOTER_CENTER_TO_SCROLL_BAR_PADDING 10
#define VISION_UI_LIST_FOOTER_TO_LEFT_PADDING 10

#define VISION_UI_LIST_HEADER_TO_TEXT_PADDING 2
#define VISION_UI_LIST_HEADER_TO_LEFT_DISPLAY_PADDING 4

#define VISION_UI_LIST_SELECTOR_TO_INNER_WIDGET_PADDING 3

// fixed sizes
#define VISION_UI_LIST_FOOTER_MAX_HEIGHT 11
#define VISION_UI_LIST_FOOTER_MAX_WIDTH 19
#define VISION_UI_LIST_HEADER_MAX_HEIGHT 7
#define VISION_UI_LIST_HEADER_MAX_WIDTH 7
#define VISION_UI_LIST_FRAME_FIXED_HEIGHT 15

#define VISION_UI_LIST_SELECTOR_FIXED_HEIGHT 15

// fixed properties
#define VISION_UI_LIST_SCROLL_BAR_WIDTH 3
#define VISION_UI_LIST_SCROLL_BAR_ANIMATION_SPEED 92

// derived properties
#define VISION_UI_LIST_FOOTER_TO_SCROLL_BAR_PADDING (VISION_UI_LIST_FOOTER_CENTER_TO_SCROLL_BAR_PADDING-VISION_UI_LIST_FOOTER_MAX_WIDTH/2)
#define VISION_UI_LIST_TEXT_MAX_WIDTH (VISION_UI_SCREEN_WIDTH-VISION_UI_LIST_FOOTER_TO_SCROLL_BAR_PADDING-VISION_UI_LIST_FOOTER_MAX_WIDTH-VISION_UI_LIST_FOOTER_TO_LEFT_PADDING-VISION_UI_LIST_HEADER_TO_TEXT_PADDING-VISION_UI_LIST_HEADER_MAX_WIDTH-VISION_UI_LIST_HEADER_TO_LEFT_DISPLAY_PADDING)

typedef enum {
    LIST_ITEM,
    TITLE_ITEM,
    SWITCH_ITEM,
    SLIDER_ITEM,
    USER_ITEM,
} vision_ui_list_item_type_t;

typedef struct vision_ui_list_item_t {
    vision_ui_list_item_type_t type;

    char* content;
    uint32_t text_scroll_anchor;

    float scroll_bar_top;
    float scroll_bar_top_trg;
    float scroll_bar_height;
    float scroll_bar_height_trg;
    int16_t scroll_bar_top_px;
    int16_t scroll_bar_height_px;

    float y_list_item;
    float y_list_item_trg;

    uint8_t layer;
    uint8_t child_num;
    struct vision_ui_list_item_t* child_list_item[VISION_UI_MAX_LIST_CHILD_NUM];
    struct vision_ui_list_item_t* parent;
} vision_ui_list_item_t;

typedef struct vision_ui_switch_item_t {
    vision_ui_list_item_t base_item;

    bool value;

    void (*on_changed)(bool value);
} vision_ui_switch_item_t;

typedef struct vision_ui_slider_item_t {
    vision_ui_list_item_t base_item;

    int16_t value;
    bool is_confirmed;
    uint8_t value_step;
    int16_t value_max;
    int16_t value_min;

    uint32_t text_scroll_anchor;

    void (*on_changed)(int16_t value);
} vision_ui_slider_item_t;

typedef struct vision_ui_title_item_t {
    vision_ui_list_item_t base_item;
} vision_ui_title_item_t;

typedef struct vision_ui_user_item_t {
    vision_ui_list_item_t base_item;

    bool in_user_item;
    bool entering_user_item;
    bool exiting_user_item;

    void (*init_function)();

    void (*loop_function)(); //user_item的逻辑和item写在一起 方便渲染
    void (*exit_function)();

    bool user_item_inited;
    bool user_item_looping;
} vision_ui_user_item_t;

extern vision_ui_list_item_t* vision_ui_root_list_get();

extern vision_ui_switch_item_t* vision_ui_to_list_switch_item(vision_ui_list_item_t* list_item);

extern vision_ui_slider_item_t* vision_ui_to_list_slider_item(vision_ui_list_item_t* list_item);

extern vision_ui_user_item_t* vision_ui_to_list_user_item(vision_ui_list_item_t* list_item);

extern vision_ui_list_item_t* vision_ui_list_item_new(char* content);

extern vision_ui_list_item_t* vision_ui_list_title_item_new(const char* title);

extern vision_ui_list_item_t* vision_ui_list_switch_item_new(char* content, bool default_value, void (*on_changed)(bool value));

extern vision_ui_list_item_t* vision_ui_list_slider_item_new(char* content, int16_t default_value, uint8_t step, int16_t min, int16_t max,
                                                             void (*on_changed)(int16_t value));

extern vision_ui_list_item_t* vision_ui_list_user_item_new(char* content, void (*init_function)(), void (*loop_function)(),
                                                           void (*exit_function)());

extern bool vision_ui_list_push_item(vision_ui_list_item_t* parent, vision_ui_list_item_t* child);

typedef struct vision_ui_selector_t {
    float y_selector;
    float y_selector_trg;

    float w_selector;
    float w_selector_trg;

    float h_selector;
    float h_selector_trg;

    uint8_t selected_index;
    vision_ui_list_item_t* selected_item;
} vision_ui_selector_t;

extern vision_ui_selector_t VISION_UI_SELECTOR;

extern vision_ui_selector_t* vision_ui_selector_get();

extern bool vision_ui_selector_t_selector_bind_item(vision_ui_list_item_t* item);

extern void vision_ui_selector_go_next_item();

extern void vision_ui_selector_go_prev_item();

extern void vision_ui_selector_jump_to_selected_item();

extern void vision_ui_selector_exit_current_item();

typedef struct vision_ui_camera_t {
    float x_camera;
    float x_camera_trg;

    float y_camera;
    float y_camera_trg;
    vision_ui_selector_t* selector;
} vision_ui_camera_t;

extern vision_ui_camera_t VISION_UI_CAMERA;

extern vision_ui_camera_t* vision_ui_camera_instance_get();

extern void vision_ui_camera_bind_selector(vision_ui_selector_t* selector);

#endif //VISION_UI_VISION_UI_ITEM_H
