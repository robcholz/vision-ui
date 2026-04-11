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

/**
 * Creates a plain list container that can hold child items.
 *
 * @param ui UI instance whose allocator and ownership list should be used. Must not be `NULL`.
 * @param capacity Maximum number of direct child items this list can hold.
 * @param icon_mode Set to `true` to make this list accept icon-view children only.
 * @param content Borrowed NUL-terminated label pointer stored on the item. The pointed-to string should remain valid
 * while the item is in use.
 * @return A non-`NULL` item on success, or `NULL` when allocating the item or its child array fails.
 */
extern vision_ui_list_item_t* vision_ui_list_item_new(
        const vision_ui_t* ui,
        size_t capacity,
        bool icon_mode,
        const char* content
);

/**
 * Creates a non-interactive title row.
 *
 * @param ui UI instance whose allocator and ownership list should be used. Must not be `NULL`.
 * @param title Borrowed NUL-terminated title string stored on the item. The pointed-to string should remain valid
 * while the item is in use.
 * @return A non-`NULL` item on success, or `NULL` when allocating the item fails.
 */
extern vision_ui_list_item_t* vision_ui_list_title_item_new(const vision_ui_t* ui, const char* title);

/**
 * Creates a selectable icon card, optionally with child items.
 *
 * @param ui UI instance whose allocator and ownership list should be used. Must not be `NULL`.
 * @param capacity Maximum number of direct child items this icon item can hold.
 * @param icon Borrowed monochrome bitmap pointer stored on the item. The pointed-to bitmap should remain valid while
 * the item is in use.
 * @param title Borrowed NUL-terminated title string stored on the item. The pointed-to string should remain valid
 * while the item is in use.
 * @param description Borrowed NUL-terminated description string stored on the item. The pointed-to string should remain
 * valid while the item is in use.
 * @return A non-`NULL` item on success, or `NULL` when allocating the item or its child array fails.
 */
extern vision_ui_list_item_t* vision_ui_list_icon_item_new(
        const vision_ui_t* ui,
        size_t capacity,
        const uint8_t* icon,
        const char* title,
        const char* description
);

/**
 * Creates a switch row whose callback runs when the value changes.
 *
 * @param ui UI instance whose allocator and ownership list should be used. Must not be `NULL`.
 * @param content Borrowed NUL-terminated label string stored on the item. The pointed-to string should remain valid
 * while the item is in use.
 * @param default_value Initial switch value.
 * @param on_changed Optional callback invoked after the switch value changes.
 * @param user_data Borrowed callback context pointer stored on the item. The pointed-to data should remain valid while
 * the item is in use.
 * @return A non-`NULL` item on success, or `NULL` when allocating the item fails.
 */
extern vision_ui_list_item_t* vision_ui_list_switch_item_new(
        const vision_ui_t* ui,
        const char* content,
        bool default_value,
        vision_ui_switch_changed_cb on_changed,
        void* user_data
);

/**
 * Creates a numeric slider row.
 *
 * @param ui UI instance whose allocator and ownership list should be used. Must not be `NULL`.
 * @param content Borrowed NUL-terminated label string stored on the item. The pointed-to string should remain valid
 * while the item is in use.
 * @param default_value Initial slider value.
 * @param step Slider step size used when the value changes.
 * @param min Minimum slider value.
 * @param max Maximum slider value.
 * @param on_changed Optional callback invoked after the slider value changes.
 * @param user_data Borrowed callback context pointer stored on the item. The pointed-to data should remain valid while
 * the item is in use.
 * @return A non-`NULL` item on success, or `NULL` when allocating the item fails.
 */
extern vision_ui_list_item_t* vision_ui_list_slider_item_new(
        const vision_ui_t* ui,
        const char* content,
        int16_t default_value,
        uint8_t step,
        int16_t min,
        int16_t max,
        vision_ui_slider_changed_cb on_changed,
        void* user_data
);

/**
 * Creates a full-screen user scene that takes over drawing while selected.
 *
 * @param ui UI instance whose allocator and ownership list should be used. Must not be `NULL`.
 * @param content Borrowed NUL-terminated label string stored on the item. The pointed-to string should remain valid
 * while the item is in use.
 * @param init_function Optional callback run the first time the user item becomes active.
 * @param loop_function Optional callback run every frame while the user item is active.
 * @param exit_function Optional callback run when leaving the user item.
 * @param user_data Borrowed callback context pointer shared by the scene callbacks. The pointed-to data should remain
 * valid while the item is in use.
 * @return A non-`NULL` item on success, or `NULL` when allocating the item fails.
 */
extern vision_ui_list_item_t* vision_ui_list_user_item_new(
        const vision_ui_t* ui,
        const char* content,
        vision_ui_scene_cb init_function,
        vision_ui_scene_cb loop_function,
        vision_ui_scene_cb exit_function,
        void* user_data
);

/**
 * Sets the root list displayed by the UI.
 *
 * @param ui UI instance to configure. Must not be `NULL`.
 * @param item Root list item to attach by pointer. The item is not copied.
 * @return `true` when `item` was accepted, or `false` when `item` is `NULL`.
 *
 * @note If `item` was created with Vision UI item constructors for the same `ui`, `vision_ui_destroy()` will release
 * it later as part of the UI-owned item tree.
 */
extern bool vision_ui_root_item_set(vision_ui_t* ui, vision_ui_list_item_t* item);

extern vision_ui_list_item_t* vision_ui_root_list_get(const vision_ui_t* ui);

/**
 * Appends a child item to a parent list container.
 *
 * @param ui UI instance whose selector/camera state may be updated. Must not be `NULL`.
 * @param parent Parent list container that will receive the child by pointer. The child is not copied.
 * @param child Child item to append by pointer.
 * @return `true` on success. Returns `false` when `parent` or `child` is `NULL`, when `parent` is already full, when
 * the maximum nesting layer would be exceeded, or when an icon-view parent receives a non-icon child.
 */
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
