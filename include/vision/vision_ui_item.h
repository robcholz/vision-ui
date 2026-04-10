//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//

#ifndef VISION_UI_VISION_UI_ITEM_H
#define VISION_UI_VISION_UI_ITEM_H

#include "vision_ui_types.h"

extern vision_ui_switch_item_t* vision_ui_to_list_switch_item(vision_ui_list_item_t* list_item);

extern vision_ui_slider_item_t* vision_ui_to_list_slider_item(vision_ui_list_item_t* list_item);

extern vision_ui_icon_item_t* vision_ui_to_list_icon_item(vision_ui_list_item_t* list_item);

extern vision_ui_user_item_t* vision_ui_to_list_user_item(vision_ui_list_item_t* list_item);

/// Creates a plain list container that can hold child items.
extern vision_ui_list_item_t* vision_ui_list_item_new(
        const vision_ui_t* ui,
        size_t capacity,
        bool icon_mode,
        const char* content
);

/// Creates a non-interactive title row.
extern vision_ui_list_item_t* vision_ui_list_title_item_new(const vision_ui_t* ui, const char* title);

/// Creates a selectable icon card, optionally with child items.
extern vision_ui_list_item_t* vision_ui_list_icon_item_new(
        const vision_ui_t* ui,
        size_t capacity,
        const uint8_t* icon,
        const char* title,
        const char* description
);

/// Creates a switch row whose callback runs when the value changes.
extern vision_ui_list_item_t* vision_ui_list_switch_item_new(
        const vision_ui_t* ui,
        const char* content,
        bool default_value,
        void (*on_changed)(vision_ui_t* ui, bool value)
);

/// Creates a numeric slider row.
extern vision_ui_list_item_t* vision_ui_list_slider_item_new(
        const vision_ui_t* ui,
        const char* content,
        int16_t default_value,
        uint8_t step,
        int16_t min,
        int16_t max,
        void (*on_changed)(vision_ui_t* ui, int16_t value)
);

/// Creates a full-screen user scene that takes over drawing while selected.
extern vision_ui_list_item_t* vision_ui_list_user_item_new(
        const vision_ui_t* ui,
        const char* content,
        void (*init_function)(vision_ui_t* ui),
        void (*loop_function)(vision_ui_t* ui),
        void (*exit_function)(vision_ui_t* ui)
);

/// Sets the root list displayed by the UI.
extern bool vision_ui_root_item_set(vision_ui_t* ui, vision_ui_list_item_t* item);

extern vision_ui_list_item_t* vision_ui_root_list_get(const vision_ui_t* ui);
/// Appends a child item to a parent list container.
extern bool vision_ui_list_push_item(vision_ui_t* ui, vision_ui_list_item_t* parent, vision_ui_list_item_t* child);

extern const vision_ui_selector_t* vision_ui_selector_instance_get(const vision_ui_t* ui);

extern vision_ui_selector_t* vision_ui_selector_mutable_instance_get(vision_ui_t* ui);

extern bool vision_ui_selector_t_selector_bind_item(vision_ui_t* ui, vision_ui_list_item_t* item);

extern void vision_ui_selector_go_next_item(vision_ui_t* ui);

extern void vision_ui_selector_go_prev_item(vision_ui_t* ui);

extern void vision_ui_selector_jump_to_selected_item(vision_ui_t* ui);

extern void vision_ui_selector_exit_current_item(vision_ui_t* ui);

extern const vision_ui_camera_t* vision_ui_camera_instance_get(const vision_ui_t* ui);

extern vision_ui_camera_t* vision_ui_camera_mutable_instance_get(vision_ui_t* ui);

extern void vision_ui_camera_instance_x_trg_set(vision_ui_t* ui, float x_trg);

extern void vision_ui_camera_instance_y_trg_set(vision_ui_t* ui, float y_trg);

extern void vision_ui_camera_bind_selector(vision_ui_t* ui, vision_ui_selector_t* selector);

#endif // VISION_UI_VISION_UI_ITEM_H
