//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//

#ifndef VISION_UI_VISION_UI_ITEM_H
#define VISION_UI_VISION_UI_ITEM_H

#include <stdbool.h>
#include <stddef.h>

#include "vision_ui_draw_driver.h"

typedef enum {
    VISION_UI_VIEW_LIST,
    VISION_UI_VIEW_ICON,
    VISION_UI_VIEW_CUSTOM,
} vision_ui_view_type_t;

typedef struct vision_ui_page_t vision_ui_page_t;

typedef struct vision_ui_notification_t {
    const char *content;
    uint16_t span;

    float y_notification;
    float y_notification_trg;

    float w_notification;
    float w_notification_trg;

    bool is_running;
    uint32_t time_start;
    uint32_t time;
    bool is_dismissing;
    uint32_t dismiss_start;
    const char *pending_content;
    uint16_t pending_span;
    bool has_pending;
} vision_ui_notification_t;

typedef struct vision_ui_alert_t {
    const char *content;
    uint16_t span;

    float y_alert;
    float y_alert_trg;

    float w_alert;
    float w_alert_trg;

    bool is_running;
    uint32_t time_start;
    uint32_t time;
} vision_ui_alert_t;

typedef enum {
    LIST_ITEM,
    TITLE_ITEM,
    SWITCH_ITEM,
    SLIDER_ITEM,
} vision_ui_list_item_type_t;

typedef struct vision_ui_list_item_t {
    vision_ui_list_item_type_t type;

    const char *content;
    uint32_t text_scroll_anchor;

    float y_list_item;
    float y_list_item_trg;

    uint8_t layer;
    uint8_t child_num;
    size_t capacity;
    struct vision_ui_list_item_t **child_list_item;
    struct vision_ui_list_item_t *parent;

    vision_ui_page_t *page;
} vision_ui_list_item_t;

typedef struct vision_ui_switch_item_t {
    vision_ui_list_item_t base_item;

    bool value;

    bool is_stateless;

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
} vision_ui_user_item_t;

typedef struct vision_ui_selector_t {
    float y_selector;
    float y_selector_trg;

    float w_selector;
    float w_selector_trg;

    float h_selector;
    float h_selector_trg;

    uint8_t selected_index;
    vision_ui_list_item_t *selected_item;

    vision_ui_page_t *scroll_bar_scale_parent;
    float scroll_bar_scale_part_shared;
} vision_ui_selector_t;

typedef struct vision_ui_camera_t {
    float x_camera;
    float x_camera_trg;

    float y_camera;
    float y_camera_trg;
    vision_ui_selector_t *selector;
} vision_ui_camera_t;

typedef struct {
    float top;
    float top_trg;
    float height;
    float height_trg;
    float scale_part;
    float scale_part_trg;
    int16_t top_px;
    int16_t height_px;
} vision_ui_list_scroll_state_t;

typedef struct {
    vision_ui_list_item_t *root;
    vision_ui_list_scroll_state_t scroll;
} vision_ui_list_view_t;

typedef struct {
    uint8_t icon_count;
    uint8_t columns;
} vision_ui_icon_view_t;

typedef struct {
    void (*init_function)();
    void (*loop_function)();
    void (*exit_function)();
    bool entering;
    bool exiting;
    bool active;
    bool is_initialized;
    bool is_looping;
} vision_ui_custom_view_t;

typedef struct vision_ui_page_t {
    vision_ui_view_type_t view_type;
    vision_ui_page_t *parent;
    vision_ui_list_item_t *owner_item;
    union {
        vision_ui_list_view_t list;
        vision_ui_icon_view_t icon;
        vision_ui_custom_view_t custom;
    } view;
} vision_ui_page_t;

extern vision_ui_page_t *vision_ui_custom_page_create(void (*init_function)(), void (*loop_function)(), void (*exit_function)());

extern void vision_ui_custom_page_bind_to_item(vision_ui_page_t *page, vision_ui_list_item_t *item);

extern void vision_ui_font_set(void *font);

extern void *vision_ui_font_get();

extern bool vision_ui_exit_animation_is_finished();

extern void vision_ui_exit_animation_set_is_finished();

extern const vision_ui_notification_t *vision_ui_notification_instance_get();

extern vision_ui_notification_t *vision_ui_notification_mutable_instance_get();

extern void vision_ui_notification_push(const char *content, uint16_t span);

extern const vision_ui_alert_t *vision_ui_alert_instance_get();

extern vision_ui_alert_t *vision_ui_alert_mutable_instance_get();

extern void vision_ui_alert_push(const char *content, uint16_t span);

extern vision_ui_list_item_t *vision_ui_root_list_get();

extern vision_ui_switch_item_t *vision_ui_to_list_switch_item(vision_ui_list_item_t *list_item);

extern vision_ui_slider_item_t *vision_ui_to_list_slider_item(vision_ui_list_item_t *list_item);

extern vision_ui_user_item_t *vision_ui_to_list_user_item(vision_ui_list_item_t *list_item);

extern vision_ui_list_item_t *vision_ui_list_item_new(size_t capacity, const char *content);

extern vision_ui_list_item_t *vision_ui_list_title_item_new(size_t capacity, const char *title);

extern vision_ui_list_item_t *vision_ui_list_switch_item_new(size_t capacity, const char *content, bool default_value,
                                                             void (*on_changed)(bool value));

extern vision_ui_list_item_t *vision_ui_list_switch_item_stateless_new(size_t capacity, const char *content,
                                                                       void (*on_changed)(bool value));

extern vision_ui_list_item_t *vision_ui_list_slider_item_new(size_t capacity, const char *content, int16_t default_value, uint8_t step,
                                                             int16_t min, int16_t max, void (*on_changed)(int16_t value));

extern bool vision_ui_list_push_item(vision_ui_list_item_t *parent, vision_ui_list_item_t *child);

extern const vision_ui_selector_t *vision_ui_selector_instance_get();

extern vision_ui_selector_t *vision_ui_selector_mutable_instance_get();

extern bool vision_ui_selector_t_selector_bind_item(vision_ui_list_item_t *item);

extern void vision_ui_selector_go_next_item();

extern void vision_ui_selector_go_prev_item();

extern void vision_ui_selector_jump_to_selected_item();

extern void vision_ui_selector_exit_current_item();

extern const vision_ui_camera_t *vision_ui_camera_instance_get();

extern vision_ui_camera_t *vision_ui_camera_mutable_instance_get();

extern void vision_ui_camera_instance_x_trg_set(float x_trg);

extern void vision_ui_camera_instance_y_trg_set(float y_trg);

extern void vision_ui_camera_bind_selector(vision_ui_selector_t *selector);

extern vision_ui_page_t *vision_ui_page_root_get();

extern vision_ui_page_t *vision_ui_page_active_get();

extern void vision_ui_page_active_set(vision_ui_page_t *page);

extern vision_ui_page_t *vision_ui_page_from_item(const vision_ui_list_item_t *item);

extern void vision_ui_page_set_view_type(vision_ui_page_t *page, vision_ui_view_type_t view_type);

extern void vision_ui_page_icon_configure(vision_ui_page_t *page, uint8_t icon_count, uint8_t columns);

#endif // VISION_UI_VISION_UI_ITEM_H
