#ifndef VISION_UI_VISION_UI_TYPES_H
#define VISION_UI_VISION_UI_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/// Opaque UI instance that owns runtime state for one Vision UI session.
typedef struct vision_ui_t vision_ui_t;

/// Public tree node type used to build list screens and child items.
typedef struct vision_ui_list_item_t vision_ui_list_item_t;
typedef struct vision_ui_selector_t vision_ui_selector_t;

typedef void (*vision_ui_switch_changed_cb)(vision_ui_t* ui, bool value, void* user_data);
typedef void (*vision_ui_slider_changed_cb)(vision_ui_t* ui, int16_t value, void* user_data);
typedef void (*vision_ui_scene_cb)(vision_ui_t* ui, void* user_data);

/// Font configuration passed to the public font setter and driver APIs.
typedef struct vision_ui_font_t {
    /// Backend font object, such as a u8g2 font pointer.
    const void* font;
    /// Adjusts text metrics upward when the font baseline sits too low.
    int8_t top_compensation;
    /// Adjusts text metrics downward when the font clips or sits too high.
    int8_t bottom_compensation;
} vision_ui_font_t;

/// Shared bitmap pack used by the list icon renderer.
typedef struct vision_ui_list_icon_t {
    /// Header bitmap for a plain list container.
    const uint8_t* list_header;
    /// Header bitmap for a switch item.
    const uint8_t* switch_header;
    /// Header bitmap for a slider item.
    const uint8_t* slider_header;
    /// Header bitmap for other list item styles.
    const uint8_t* default_header;

    /// Footer bitmap for an enabled switch item.
    const uint8_t* switch_on_footer;
    /// Footer bitmap for a disabled switch item.
    const uint8_t* switch_off_footer;
    /// Footer bitmap for a slider item.
    const uint8_t* slider_footer;

    /// Shared width of header bitmaps.
    size_t header_width;
    /// Shared height of header bitmaps.
    size_t header_height;

    /// Shared width of footer bitmaps.
    size_t footer_width;
    /// Shared height of footer bitmaps.
    size_t footer_height;
} vision_ui_icon_t;

/// High-level input actions consumed by Vision UI.
typedef enum vision_ui_action_t {
    UiActionNone,
    UiActionGoPrev,
    UiActionGoNext,
    UiActionEnter,
    UiActionExit,
} vision_ui_action_t;

typedef struct vision_ui_notification_t {
    const char* content;
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
    const char* pending_content;
    uint16_t pending_span;
    bool has_pending;
} vision_ui_notification_t;

typedef struct vision_ui_alert_t {
    const char* content;
    uint16_t span;

    float y_alert;
    float y_alert_trg;

    float w_alert;
    float w_alert_trg;

    bool is_running;
    uint32_t time_start;
    uint32_t time;
} vision_ui_alert_t;

typedef enum vision_ui_list_item_type_t {
    ListItem,
    TitleItem,
    IconItem,
    SwitchItem,
    SliderItem,
    UserItem,
} vision_ui_list_item_type_t;

struct vision_ui_list_item_t {
    vision_ui_list_item_type_t type;
    bool icon_view_mode;
    bool owns_child_list;

    const char* content;
    uint32_t text_scroll_anchor;

    float scroll_bar_top;
    float scroll_bar_top_velocity;
    float scroll_bar_top_trg;
    float scroll_bar_height;
    float scroll_bar_height_velocity;
    float scroll_bar_height_trg;
    float scroll_bar_scale_part;
    float scroll_bar_scale_part_trg;
    int16_t scroll_bar_top_px;
    int16_t scroll_bar_height_px;

    float y_list_item;
    float y_list_item_trg;

    float icon_scroll_offset;
    float icon_scroll_offset_velocity;
    float icon_scroll_offset_trg;

    uint8_t layer;
    uint8_t child_num;
    size_t capacity;
    vision_ui_list_item_t** child_list_item;
    vision_ui_list_item_t* parent;
    vision_ui_list_item_t* owned_next;
};

typedef struct vision_ui_switch_item_t {
    vision_ui_list_item_t base_item;

    bool value;

    vision_ui_switch_changed_cb on_changed;
    void* on_changed_user_data;
} vision_ui_switch_item_t;

typedef struct vision_ui_slider_item_t {
    vision_ui_list_item_t base_item;

    int16_t value;
    bool is_confirmed;
    uint8_t value_step;
    int16_t value_max;
    int16_t value_min;

    uint32_t text_scroll_anchor;

    vision_ui_slider_changed_cb on_changed;
    void* on_changed_user_data;
} vision_ui_slider_item_t;

typedef struct vision_ui_title_item_t {
    vision_ui_list_item_t base_item;
} vision_ui_title_item_t;

typedef struct vision_ui_icon_item_t {
    vision_ui_list_item_t base_item;

    float title_y;
    float title_y_velocity;
    float title_y_trg;

    const uint8_t* icon;
    const char* description;

    uint32_t description_scroll_anchor;
} vision_ui_icon_item_t;

typedef struct vision_ui_user_item_t {
    vision_ui_list_item_t base_item;

    bool in_user_item;
    bool entering_user_item;
    bool exiting_user_item;

    vision_ui_scene_cb init_function;
    vision_ui_scene_cb loop_function;
    vision_ui_scene_cb exit_function;
    void* user_data;

    bool user_item_inited;
    bool user_item_looping;
} vision_ui_user_item_t;

struct vision_ui_selector_t {
    float y_selector;
    float y_selector_velocity;
    float y_selector_trg;

    float w_selector;
    float w_selector_velocity;
    float w_selector_trg;

    float h_selector;
    float h_selector_velocity;
    float h_selector_trg;

    uint8_t selected_index;
    vision_ui_list_item_t* selected_item;

    vision_ui_list_item_t* scroll_bar_scale_parent;
    float scroll_bar_scale_part_shared;

    bool has_pending_selection;
    uint8_t pending_selected_index;
    vision_ui_list_item_t* pending_selected_item;
};

typedef struct vision_ui_camera_t {
    float x_camera;
    float x_camera_velocity;
    float x_camera_trg;

    float y_camera;
    float y_camera_velocity;
    float y_camera_trg;
    vision_ui_selector_t* selector;
} vision_ui_camera_t;

typedef enum vision_alloc_op_t {
    VisionAllocMalloc,
    VisionAllocCalloc,
    VisionAllocFree,
} vision_alloc_op_t;

typedef void* (*vision_ui_allocator_t)(vision_alloc_op_t op, size_t size, size_t count, void* ptr);

struct vision_ui_t {
    bool is_in_vision_ui;
    bool is_background_frozen;

    const uint8_t* logo;
    uint32_t logo_span;
    uint32_t last_tick;
    bool logo_finished;
    bool logo_started;
    uint32_t logo_start_time;

    bool exit_animation_finished;
    bool enter_animation_finished;
    float exit_elapsed_ms;
    bool running_exit;
    float enter_elapsed_ms;
    bool running_enter;

    vision_ui_font_t minifont;
    vision_ui_font_t font;
    vision_ui_font_t title_font;
    vision_ui_font_t subtitle_font;

    vision_ui_allocator_t allocator;
    vision_ui_notification_t notification;
    vision_ui_alert_t alert;
    vision_ui_selector_t selector;
    vision_ui_camera_t camera;
    vision_ui_list_item_t* root_item;
    vision_ui_list_item_t* owned_item_head;
    vision_ui_icon_t list_icon;

    void* driver;
    vision_ui_font_t driver_current_font;
    int8_t driver_str_top;
    int8_t driver_str_bottom;
};

#endif // VISION_UI_VISION_UI_TYPES_H
