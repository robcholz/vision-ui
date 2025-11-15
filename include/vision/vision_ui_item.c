//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//
#include "vision_ui_item.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "../vision_ui_config.h"
#include "vision_ui_core.h"

static void* VISION_UI_FONT;
static void* VISION_UI_TITLE_FONT;

typedef void* (*vision_ui_allocator_t)(vision_alloc_op_t op, size_t size, size_t count, void* ptr);

static vision_ui_allocator_t ALLOCATOR = NULL;

void vision_ui_allocator_set(const vision_ui_allocator_t allocator) {
    ALLOCATOR = allocator;
}

static void* vision_ui_malloc(const size_t size) {
    if (ALLOCATOR == NULL) {
        return malloc(size);
    }
    return ALLOCATOR(VisionAllocMalloc, size, 0, NULL);
}

static void* vision_ui_calloc(const size_t count, const size_t size) {
    if (ALLOCATOR == NULL) {
        return calloc(count, size);
    }
    return ALLOCATOR(VisionAllocCalloc, size, count, NULL);
}

static void vision_ui_free(void* ptr) {
    if (ALLOCATOR == NULL) {
        free(ptr);
        return;
    }
    ALLOCATOR(VisionAllocFree, 0, 0, ptr);
}

void vision_ui_font_set(void* font) {
    if (font != VISION_UI_FONT) {
        VISION_UI_FONT = font;
    }
}

void vision_ui_font_set_title(void* font) {
    if (font != VISION_UI_TITLE_FONT) {
        VISION_UI_TITLE_FONT = font;
    }
}

void* vision_ui_font_get() {
    return VISION_UI_FONT;
}

void* vision_ui_font_get_title() {
    return VISION_UI_TITLE_FONT;
}

static vision_ui_notification_t VISION_UI_NOTIFICATION = {
        0,
        1,
        0 - 2 * VISION_UI_NOTIFICATION_HEIGHT,
        0 - 2 * VISION_UI_NOTIFICATION_HEIGHT,
        VISION_UI_NOTIFICATION_WIDTH,
        VISION_UI_NOTIFICATION_WIDTH,
        false,
        0,
        1,
        false,
        0,
        NULL,
        0,
        false
};

extern const vision_ui_notification_t* vision_ui_notification_instance_get() {
    return &VISION_UI_NOTIFICATION;
}

vision_ui_notification_t* vision_ui_notification_mutable_instance_get() {
    return &VISION_UI_NOTIFICATION;
}

void vision_ui_notification_push(const char* content, const uint16_t span) {
    const uint32_t now = vision_ui_driver_ticks_ms_get();

    if (!VISION_UI_NOTIFICATION.is_running) {
        VISION_UI_NOTIFICATION.content = content;
        VISION_UI_NOTIFICATION.span = span;
        VISION_UI_NOTIFICATION.time_start = now;
        VISION_UI_NOTIFICATION.time = now;
        VISION_UI_NOTIFICATION.y_notification_trg = 0;
        VISION_UI_NOTIFICATION.is_running = true;
        VISION_UI_NOTIFICATION.is_dismissing = false;
        VISION_UI_NOTIFICATION.has_pending = false;
        VISION_UI_NOTIFICATION.pending_content = NULL;
        VISION_UI_NOTIFICATION.pending_span = 0;
        VISION_UI_NOTIFICATION.dismiss_start = 0;
        vision_ui_font_set(vision_ui_font_get());
        VISION_UI_NOTIFICATION.w_notification_trg =
                vision_ui_driver_str_utf8_width_get(VISION_UI_NOTIFICATION.content) + VISION_UI_NOTIFICATION_WIDTH;
        return;
    }

    VISION_UI_NOTIFICATION.pending_content = content;
    VISION_UI_NOTIFICATION.pending_span = span;
    VISION_UI_NOTIFICATION.has_pending = true;
    VISION_UI_NOTIFICATION.is_dismissing = true;
    VISION_UI_NOTIFICATION.dismiss_start = now;
    VISION_UI_NOTIFICATION.y_notification_trg = 0 - 2 * VISION_UI_NOTIFICATION_HEIGHT;
}

static vision_ui_alert_t VISION_UI_ALERT = {
        0,
        1,
        0 - 2 * VISION_UI_ALERT_HEIGHT,
        0 - 2 * VISION_UI_ALERT_HEIGHT,
        VISION_UI_ALERT_WIDTH,
        VISION_UI_ALERT_WIDTH,
        false,
        0,
        1
};

const vision_ui_alert_t* vision_ui_alert_instance_get() {
    return &VISION_UI_ALERT;
}

vision_ui_alert_t* vision_ui_alert_mutable_instance_get() {
    return &VISION_UI_ALERT;
}

void vision_ui_alert_push(const char* content, const uint16_t span) {
    VISION_UI_ALERT.time = vision_ui_driver_ticks_ms_get();
    VISION_UI_ALERT.content = content;
    VISION_UI_ALERT.span = span;
    VISION_UI_ALERT.is_running = false;

    // 弹出
    if (!VISION_UI_ALERT.is_running) {
        VISION_UI_ALERT.time_start = vision_ui_driver_ticks_ms_get();
        VISION_UI_ALERT.y_alert_trg = (VISION_UI_SCREEN_HEIGHT - VISION_UI_ALERT_HEIGHT) / 2;
        VISION_UI_ALERT.is_running = true;
    }

    vision_ui_font_set(vision_ui_font_get());
    VISION_UI_ALERT.w_alert_trg = vision_ui_driver_str_utf8_width_get(VISION_UI_ALERT.content) + VISION_UI_ALERT_WIDTH;
}

// vision_ui_list_item_t vision_ui_list_item_root = {};

vision_ui_switch_item_t* vision_ui_to_list_switch_item(vision_ui_list_item_t* list_item) {
    assert(list_item != NULL && list_item->type == SwitchItem);
    return (vision_ui_switch_item_t*) list_item;
}

vision_ui_slider_item_t* vision_ui_to_list_slider_item(vision_ui_list_item_t* list_item) {
    assert(list_item != NULL && list_item->type == SliderItem);
    return (vision_ui_slider_item_t*) list_item;
}

vision_ui_icon_item_t* vision_ui_to_list_icon_item(vision_ui_list_item_t* list_item) {
    assert(list_item != NULL && list_item->type == IconItem);
    return (vision_ui_icon_item_t*) list_item;
}

vision_ui_user_item_t* vision_ui_to_list_user_item(vision_ui_list_item_t* list_item) {
    assert(list_item != NULL && list_item->type == UserItem);
    return (vision_ui_user_item_t*) list_item;
}

vision_ui_list_item_t* vision_ui_list_item_new(const size_t capacity, const bool icon_mode, const char* content) {
    vision_ui_list_item_t* list_item = vision_ui_malloc(sizeof(vision_ui_list_item_t));
    memset(list_item, 0, sizeof(vision_ui_list_item_t));
    list_item->type = ListItem;
    list_item->icon_view_mode = icon_mode;
    list_item->content = content;
    list_item->capacity = capacity;
    list_item->child_list_item = vision_ui_malloc(list_item->capacity * sizeof(vision_ui_list_item_t*));
    return list_item;
}

vision_ui_list_item_t* vision_ui_list_title_item_new(const char* title) {
    vision_ui_list_item_t* list_item = vision_ui_malloc(sizeof(vision_ui_title_item_t));
    memset(list_item, 0, sizeof(vision_ui_title_item_t));
    list_item->type = TitleItem;
    list_item->content = title;
    list_item->capacity = 0;
    list_item->child_list_item = NULL;
    return list_item;
}

vision_ui_list_item_t* vision_ui_list_icon_item_new(
        const size_t capacity,
        const uint8_t* icon,
        const char* title,
        const char* description
) {
    vision_ui_list_item_t* list_item = vision_ui_malloc(sizeof(vision_ui_icon_item_t));
    memset(list_item, 0, sizeof(vision_ui_icon_item_t));
    list_item->type = IconItem;
    list_item->content = title;
    list_item->capacity = capacity;
    list_item->child_list_item = vision_ui_malloc(list_item->capacity * sizeof(vision_ui_list_item_t*));

    ((vision_ui_icon_item_t*) list_item)->icon = icon;
    ((vision_ui_icon_item_t*) list_item)->description = description;
    // start hidden so the first selection animates the slide-in
    ((vision_ui_icon_item_t*) list_item)->title_y = VISION_UI_ICON_VIEW_TITLE_AREA_HEIGHT;
    ((vision_ui_icon_item_t*) list_item)->title_y_velocity = 0;
    ((vision_ui_icon_item_t*) list_item)->title_y_trg = VISION_UI_ICON_VIEW_TITLE_AREA_HEIGHT;
    return list_item;
}

vision_ui_list_item_t* vision_ui_list_switch_item_new(
        const char* content,
        const bool default_value,
        void (*on_changed)(bool value)
) {
    vision_ui_switch_item_t* switch_item = vision_ui_malloc(sizeof(vision_ui_switch_item_t));
    memset(switch_item, 0, sizeof(vision_ui_switch_item_t));
    switch_item->base_item.type = SwitchItem;
    switch_item->base_item.content = content;
    switch_item->value = default_value;
    switch_item->on_changed = on_changed;
    ((vision_ui_list_item_t*) switch_item)->capacity = 0;
    ((vision_ui_list_item_t*) switch_item)->child_list_item = NULL;
    return (vision_ui_list_item_t*) switch_item;
}

vision_ui_list_item_t* vision_ui_list_slider_item_new(
        const char* content,
        const int16_t default_value,
        const uint8_t step,
        const int16_t min,
        const int16_t max,
        void (*on_changed)(int16_t value)
) {
    vision_ui_slider_item_t* slider_item = vision_ui_malloc(sizeof(vision_ui_slider_item_t));
    memset(slider_item, 0, sizeof(vision_ui_slider_item_t));
    slider_item->base_item.type = SliderItem;
    slider_item->base_item.content = content;
    slider_item->value = default_value;
    slider_item->value_step = step;
    slider_item->value_min = min;
    slider_item->value_max = max;
    slider_item->on_changed = on_changed;
    ((vision_ui_list_item_t*) slider_item)->capacity = 0;
    ((vision_ui_list_item_t*) slider_item)->child_list_item = NULL;
    return (vision_ui_list_item_t*) slider_item;
}

vision_ui_list_item_t* vision_ui_list_user_item_new(
        const char* content,
        void (*init_function)(),
        void (*loop_function)(),
        void (*exit_function)()
) {
    vision_ui_user_item_t* user_item = vision_ui_malloc(sizeof(vision_ui_user_item_t));
    memset(user_item, 0, sizeof(vision_ui_user_item_t));
    user_item->base_item.type = UserItem;
    user_item->base_item.content = content;
    user_item->init_function = init_function;
    user_item->loop_function = loop_function;
    user_item->exit_function = exit_function;
    ((vision_ui_list_item_t*) user_item)->capacity = 0;
    ((vision_ui_list_item_t*) user_item)->child_list_item = NULL;
    return (vision_ui_list_item_t*) user_item;
}

static vision_ui_selector_t VISION_UI_SELECTOR = {};

const vision_ui_selector_t* vision_ui_selector_instance_get() {
    return &VISION_UI_SELECTOR;
}

extern vision_ui_selector_t* vision_ui_selector_mutable_instance_get() {
    return &VISION_UI_SELECTOR;
}

bool vision_ui_selector_t_selector_bind_item(vision_ui_list_item_t* item) {
    if (item == NULL) {
        return false;
    }

    // 找item在父节点中的序号
    uint8_t temp_index = 0;
    if (item->parent != NULL) {
        for (uint8_t i = 0; i < item->parent->child_num; i++) {
            if (item->parent->child_list_item[i] == item) {
                temp_index = i;
                break;
            }
        }
    }

    // 坐标在refresh内部更新
    if (VISION_UI_SELECTOR.selected_item == NULL) {
        VISION_UI_SELECTOR.y_selector = 2 * VISION_UI_SCREEN_HEIGHT; // 给个初始坐标做动画
        VISION_UI_SELECTOR.h_selector = 160;
    }
    VISION_UI_SELECTOR.selected_index = temp_index;
    VISION_UI_SELECTOR.selected_item = item;

    return true;
}

void vision_ui_selector_go_next_item() {
    if (VISION_UI_SELECTOR.selected_item->type == SliderItem &&
        vision_ui_to_list_slider_item(VISION_UI_SELECTOR.selected_item)->is_confirmed) {
        vision_ui_slider_item_t* selected_slider_item = vision_ui_to_list_slider_item(VISION_UI_SELECTOR.selected_item);
        selected_slider_item->value += selected_slider_item->value_step;
        if (selected_slider_item->value >= selected_slider_item->value_max) {
            selected_slider_item->value = selected_slider_item->value_max;
        }
        selected_slider_item->on_changed(selected_slider_item->value);
        return;
    }

    if (VISION_UI_SELECTOR.selected_item->type == UserItem &&
        vision_ui_to_list_user_item(VISION_UI_SELECTOR.selected_item)->in_user_item) {
        return;
    }

    // 到达最末端
    if (VISION_UI_SELECTOR.selected_index == VISION_UI_SELECTOR.selected_item->parent->child_num - 1) {
        VISION_UI_SELECTOR.selected_item = VISION_UI_SELECTOR.selected_item->parent->child_list_item[0];
        VISION_UI_SELECTOR.selected_index = 0;
        return;
    }

    VISION_UI_SELECTOR.selected_item =
            VISION_UI_SELECTOR.selected_item->parent->child_list_item[++VISION_UI_SELECTOR.selected_index];
}

void vision_ui_selector_go_prev_item() {
    if (VISION_UI_SELECTOR.selected_item->type == SliderItem &&
        vision_ui_to_list_slider_item(VISION_UI_SELECTOR.selected_item)->is_confirmed) {
        vision_ui_slider_item_t* selected_slider_item = vision_ui_to_list_slider_item(VISION_UI_SELECTOR.selected_item);
        selected_slider_item->value -= selected_slider_item->value_step;
        if (selected_slider_item->value <= selected_slider_item->value_min) {
            selected_slider_item->value = selected_slider_item->value_min;
        }
        selected_slider_item->on_changed(selected_slider_item->value);
        return;
    }

    if (VISION_UI_SELECTOR.selected_item->type == UserItem &&
        vision_ui_to_list_user_item(VISION_UI_SELECTOR.selected_item)->in_user_item) {
        return;
    }

    // 到达最前端
    if (VISION_UI_SELECTOR.selected_index == 0) {
        VISION_UI_SELECTOR.selected_item =
                VISION_UI_SELECTOR.selected_item->parent
                        ->child_list_item[VISION_UI_SELECTOR.selected_item->parent->child_num - 1];
        VISION_UI_SELECTOR.selected_index = VISION_UI_SELECTOR.selected_item->parent->child_num - 1;
        return;
    }

    VISION_UI_SELECTOR.selected_item =
            VISION_UI_SELECTOR.selected_item->parent->child_list_item[--VISION_UI_SELECTOR.selected_index];
}

static bool VISION_UI_EXIT_ANIMATION_FINISHED = true;

extern bool vision_ui_exit_animation_is_finished() {
    return VISION_UI_EXIT_ANIMATION_FINISHED;
}

void vision_ui_exit_animation_set_is_finished() {
    VISION_UI_EXIT_ANIMATION_FINISHED = true;
}

void vision_ui_exit_animation_start() {
    VISION_UI_EXIT_ANIMATION_FINISHED = false;
}

static bool VISION_UI_ENTER_ANIMATION_FINISHED = true;

extern bool vision_ui_enter_animation_is_finished() {
    return VISION_UI_ENTER_ANIMATION_FINISHED;
}

extern void vision_ui_enter_animation_set_is_finished() {
    VISION_UI_ENTER_ANIMATION_FINISHED = true;
}

extern void vision_ui_enter_animation_start() {
    VISION_UI_ENTER_ANIMATION_FINISHED = false;
}

/** @brief 确认当前选择的item
 * @note 如果选择了list 就进入选择的list
 * @note 如果选择了特殊item 就翻转/调整对应的值
 */
void vision_ui_selector_jump_to_selected_item() {
    if (!IS_IN_VISION_UI) {
        return;
    }

    if (VISION_UI_SELECTOR.selected_item->type == UserItem) {
        vision_ui_exit_animation_start();
        // vision_ui_selector.selected_item->in_user_item = true;
        vision_ui_user_item_t* selected_user_item = vision_ui_to_list_user_item(VISION_UI_SELECTOR.selected_item);
        selected_user_item->entering_user_item = true;
        selected_user_item->exiting_user_item = false;
        selected_user_item->user_item_inited = false;
        selected_user_item->user_item_looping = false;
        return;
    }

    if (VISION_UI_SELECTOR.selected_item->type == TitleItem) {
        vision_ui_selector_exit_current_item();
        return;
    }

    if (VISION_UI_SELECTOR.selected_item->type == SwitchItem) {
        vision_ui_switch_item_t* selected_switch_item = vision_ui_to_list_switch_item(VISION_UI_SELECTOR.selected_item);
        selected_switch_item->value = !selected_switch_item->value;
        selected_switch_item->on_changed(selected_switch_item->value);
        return;
    }

    if (VISION_UI_SELECTOR.selected_item->type == SliderItem) {
        vision_ui_slider_item_t* selected_slider_item = vision_ui_to_list_slider_item(VISION_UI_SELECTOR.selected_item);
        if (!selected_slider_item->is_confirmed) {
            selected_slider_item->is_confirmed = true; // 如果没选中 就选中
            return;
        }
        if (selected_slider_item->is_confirmed) {
            selected_slider_item->is_confirmed = false;
            return;
        }
    }

    if (VISION_UI_SELECTOR.selected_item->child_num == 0) {
        return;
    }

    vision_ui_exit_animation_start();

    // 给选择的item的子item坐标清零 做动画
    for (uint8_t i = 0; i < VISION_UI_SELECTOR.selected_item->child_num; i++) {
        VISION_UI_SELECTOR.selected_item->child_list_item[i]->y_list_item = 0;
    }

    VISION_UI_SELECTOR.selected_index = 0;
    VISION_UI_SELECTOR.selected_item = VISION_UI_SELECTOR.selected_item->child_list_item[0];
}

void vision_ui_selector_exit_current_item() {
    if (VISION_UI_SELECTOR.selected_item->type == SliderItem &&
        vision_ui_to_list_slider_item(VISION_UI_SELECTOR.selected_item)->is_confirmed) {
        // 如果已选中又长按退出键
        vision_ui_slider_item_t* selected_slider_item = vision_ui_to_list_slider_item(VISION_UI_SELECTOR.selected_item);

        if (selected_slider_item->is_confirmed) {
            selected_slider_item->is_confirmed = false;
            return;
        }
    }

    if (VISION_UI_SELECTOR.selected_item->type == UserItem &&
        vision_ui_to_list_user_item(VISION_UI_SELECTOR.selected_item)->in_user_item) {
        vision_ui_exit_animation_start(); // 需要重新绘制退场动画
        // vision_ui_selector.selected_item->in_user_item = false;
        vision_ui_user_item_t* selected_user_item = vision_ui_to_list_user_item(VISION_UI_SELECTOR.selected_item);
        selected_user_item->entering_user_item = false;
        selected_user_item->exiting_user_item = true;
        selected_user_item->user_item_inited = false;
        selected_user_item->user_item_looping = false;
        return;
    }

    if (VISION_UI_SELECTOR.selected_item->parent->layer == 0 && IS_IN_VISION_UI) {
        if (VISION_UI_ALLOW_EXIT_BY_USER) {
            IS_IN_VISION_UI = false;
        }
        return;
    }

    vision_ui_exit_animation_start();

    // 给选择的item的父item的父item的所有子item坐标清零 做动画
    for (uint8_t i = 0; i < VISION_UI_SELECTOR.selected_item->parent->parent->child_num; i++) {
        VISION_UI_SELECTOR.selected_item->parent->parent->child_list_item[i]->y_list_item = 0;
    }

    // 找到当前选择的item的父item在它的父item中的位置
    uint8_t temp_index = 0;
    for (uint8_t i = 0; i < VISION_UI_SELECTOR.selected_item->parent->parent->child_num; i++) {
        if (VISION_UI_SELECTOR.selected_item->parent->parent->child_list_item[i] ==
            VISION_UI_SELECTOR.selected_item->parent) {
            temp_index = i;
            break;
        }
    }
    VISION_UI_SELECTOR.selected_index = temp_index;
    VISION_UI_SELECTOR.selected_item = VISION_UI_SELECTOR.selected_item->parent;
}

static vision_ui_list_item_t* ROOT_ITEM = NULL;

bool vision_ui_root_item_set(vision_ui_list_item_t* item) {
    if (item == NULL) {
        return false;
    }
    ROOT_ITEM = item;
    return true;
}

vision_ui_list_item_t* vision_ui_root_list_get() {
    assert(ROOT_ITEM != NULL);
    return ROOT_ITEM;
}

bool vision_ui_list_push_item(vision_ui_list_item_t* parent, vision_ui_list_item_t* child) {
    if (parent == NULL) {
        return false;
    }
    if (child == NULL) {
        return false;
    }
    if (parent->child_num >= parent->capacity) {
        return false;
    }
    if (parent->layer >= VISION_UI_MAX_LIST_LAYER) {
        return false;
    }
    if (parent->icon_view_mode && child->type != IconItem) {
        return false;
    }

    child->layer = parent->layer + 1;

    float next_y = VISION_UI_LIST_TITLE_TO_DISPLAY_TOP_PADDING;
    if (parent->child_num > 0) {
        const vision_ui_list_item_t* last_child = parent->child_list_item[parent->child_num - 1];
        const uint8_t gap_after_last = (last_child->type == TitleItem) ? VISION_UI_LIST_TITLE_TO_FRAME_PADDING
                                                                       : VISION_UI_LIST_FRAME_BETWEEN_PADDING;
        next_y = last_child->y_list_item_trg + VISION_UI_LIST_FRAME_FIXED_HEIGHT + gap_after_last;
    } else if (child->type != TitleItem) {
        // 没有标题时仍旧保留顶端 padding
        next_y = VISION_UI_LIST_TITLE_TO_DISPLAY_TOP_PADDING;
    }
    child->y_list_item_trg = next_y;

    const vision_ui_list_item_t* selected_item = vision_ui_selector_instance_get()->selected_item;
    const bool selector_needs_root_binding = selected_item == NULL || selected_item->parent == NULL;
    if (parent == vision_ui_root_list_get() && selector_needs_root_binding) {
        vision_ui_list_item_t* first_root_item = parent->child_num > 0 ? parent->child_list_item[0] : child;
        vision_ui_selector_t_selector_bind_item(first_root_item); // 初始化并绑定selector
        vision_ui_camera_bind_selector(&VISION_UI_SELECTOR); // 初始化并绑定camera
    }

    parent->child_list_item[parent->child_num++] = child;
    child->parent = parent;

    return true;
}

static vision_ui_camera_t VISION_UI_CAMERA = {0, 0, 0, 0}; // 在refresh加上camera的坐标

const vision_ui_camera_t* vision_ui_camera_instance_get() {
    return &VISION_UI_CAMERA;
}

extern vision_ui_camera_t* vision_ui_camera_mutable_instance_get() {
    return &VISION_UI_CAMERA;
}

void vision_ui_camera_instance_x_trg_set(const float x_trg) {
    VISION_UI_CAMERA.x_camera_trg = x_trg;
}

void vision_ui_camera_instance_y_trg_set(const float y_trg) {
    VISION_UI_CAMERA.y_camera_trg = y_trg;
}

void vision_ui_camera_bind_selector(vision_ui_selector_t* selector) {
    if (selector == NULL) {
        return;
    }

    VISION_UI_CAMERA.selector = selector; // 坐标在refresh内部更新
}
