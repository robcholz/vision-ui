//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//
#include "vision_ui_item.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "../vision_ui_config.h"
#include "vision_ui_core.h"

static void *VISION_UI_FONT;

void vision_ui_font_set(void *font) {
    if (font != VISION_UI_FONT) {
        vision_ui_driver_font_set(font);
        VISION_UI_FONT = font;
    }
}

void *vision_ui_font_get() {
    return VISION_UI_FONT;
}

static vision_ui_notification_t VISION_UI_NOTIFICATION = {0,
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
                                                          false};

static vision_ui_switch_item_t VISION_UI_SWITCH_ITEM_DUMMY = {0};
static vision_ui_slider_item_t VISION_UI_SLIDER_ITEM_DUMMY = {0};
static vision_ui_user_item_t VISION_UI_USER_ITEM_DUMMY = {0};

extern const vision_ui_notification_t *vision_ui_notification_instance_get() {
    return &VISION_UI_NOTIFICATION;
}

vision_ui_notification_t *vision_ui_notification_mutable_instance_get() {
    return &VISION_UI_NOTIFICATION;
}

void vision_ui_notification_push(const char *content, const uint16_t span) {
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
        0, 1, 0 - 2 * VISION_UI_ALERT_HEIGHT, 0 - 2 * VISION_UI_ALERT_HEIGHT, VISION_UI_ALERT_WIDTH, VISION_UI_ALERT_WIDTH, false, 0, 1};

const vision_ui_alert_t *vision_ui_alert_instance_get() {
    return &VISION_UI_ALERT;
}

vision_ui_alert_t *vision_ui_alert_mutable_instance_get() {
    return &VISION_UI_ALERT;
}

void vision_ui_alert_push(const char *content, const uint16_t span) {
    VISION_UI_ALERT.time = vision_ui_driver_ticks_ms_get();
    VISION_UI_ALERT.content = content;
    VISION_UI_ALERT.span = span;
    VISION_UI_ALERT.is_running = false;

    // 弹出
    if (!VISION_UI_ALERT.is_running) {
        VISION_UI_ALERT.time_start = vision_ui_driver_ticks_ms_get();
        VISION_UI_ALERT.y_alert_trg = 20;
        VISION_UI_ALERT.is_running = true;
    }

    vision_ui_font_set(vision_ui_font_get());
    VISION_UI_ALERT.w_alert_trg = vision_ui_driver_str_utf8_width_get(VISION_UI_ALERT.content) + VISION_UI_ALERT_WIDTH;
}

// vision_ui_list_item_t vision_ui_list_item_root = {};

vision_ui_switch_item_t *vision_ui_to_list_switch_item(vision_ui_list_item_t *list_item) {
    if (list_item != NULL && list_item->type == SWITCH_ITEM) {
        return (vision_ui_switch_item_t *) list_item;
    }

    return &VISION_UI_SWITCH_ITEM_DUMMY;
}

vision_ui_slider_item_t *vision_ui_to_list_slider_item(vision_ui_list_item_t *list_item) {
    if (list_item != NULL && list_item->type == SLIDER_ITEM) {
        return (vision_ui_slider_item_t *) list_item;
    }

    return &VISION_UI_SLIDER_ITEM_DUMMY;
}

vision_ui_user_item_t *vision_ui_to_list_user_item(vision_ui_list_item_t *list_item) {
    if (list_item != NULL && list_item->type == USER_ITEM) {
        return (vision_ui_user_item_t *) list_item;
    }

    return &VISION_UI_USER_ITEM_DUMMY;
}

static vision_ui_page_t *vision_ui_page_create(const size_t capacity, const char *title) {
    vision_ui_page_t *page = malloc(sizeof(vision_ui_page_t));
    memset(page, 0, sizeof(vision_ui_page_t));
    page->title = title;
    page->capacity = capacity;
    page->items = malloc(page->capacity * sizeof(vision_ui_list_item_t *));
    if (title != NULL) {
        vision_ui_page_push_item(page, vision_ui_list_title_item_new(1, title));
    }
    return page;
}

vision_ui_page_t *vision_ui_root_page_get() {
    static vision_ui_page_t *vision_ui_root_page = NULL;
    if (vision_ui_root_page == NULL) {
        vision_ui_root_page = vision_ui_page_create(VISION_UI_LIST_ROOT_CAPACITY, "VisionUI");
        vision_ui_root_page->layer = 0;
    }
    return vision_ui_root_page;
}

vision_ui_page_t *vision_ui_list_item_child_page_get(vision_ui_list_item_t *list_item) {
    if (list_item == NULL) {
        return NULL;
    }
    return list_item->child_page;
}

vision_ui_list_item_t *vision_ui_list_item_new(const size_t capacity, const char *content) {
    vision_ui_list_item_t *list_item = malloc(sizeof(vision_ui_list_item_t));
    memset(list_item, 0, sizeof(vision_ui_list_item_t));
    list_item->type = LIST_ITEM;
    list_item->content = content;
    list_item->child_page = vision_ui_page_create(capacity, content);
    if (list_item->child_page != NULL) {
        list_item->child_page->entry_item = list_item;
    }
    return list_item;
}

vision_ui_list_item_t *vision_ui_list_title_item_new(const size_t capacity, const char *title) {
    (void) capacity;
    vision_ui_list_item_t *list_item = malloc(sizeof(vision_ui_list_item_t));
    memset(list_item, 0, sizeof(vision_ui_list_item_t));
    list_item->type = TITLE_ITEM;
    list_item->content = title;
    return list_item;
}

vision_ui_list_item_t *vision_ui_list_switch_item_new(const size_t capacity, const char *content, const bool default_value,
                                                      void (*on_changed)(bool value)) {
    (void) capacity;
    vision_ui_switch_item_t *switch_item = malloc(sizeof(vision_ui_switch_item_t));
    memset(switch_item, 0, sizeof(vision_ui_switch_item_t));
    switch_item->base_item.type = SWITCH_ITEM;
    switch_item->base_item.content = content;
    switch_item->value = default_value;
    switch_item->on_changed = on_changed;
    return (vision_ui_list_item_t *) switch_item;
}

vision_ui_list_item_t *vision_ui_list_slider_item_new(const size_t capacity, const char *content, const int16_t default_value,
                                                      const uint8_t step, const int16_t min, const int16_t max,
                                                      void (*on_changed)(int16_t value)) {
    (void) capacity;
    vision_ui_slider_item_t *slider_item = malloc(sizeof(vision_ui_slider_item_t));
    memset(slider_item, 0, sizeof(vision_ui_slider_item_t));
    slider_item->base_item.type = SLIDER_ITEM;
    slider_item->base_item.content = content;
    slider_item->value = default_value;
    slider_item->value_step = step;
    slider_item->value_min = min;
    slider_item->value_max = max;
    slider_item->on_changed = on_changed;
    return (vision_ui_list_item_t *) slider_item;
}

vision_ui_list_item_t *vision_ui_list_user_item_new(const size_t capacity, const char *content, void (*init_function)(),
                                                    void (*loop_function)(), void (*exit_function)()) {
    (void) capacity;
    vision_ui_user_item_t *user_item = malloc(sizeof(vision_ui_user_item_t));
    memset(user_item, 0, sizeof(vision_ui_user_item_t));
    user_item->base_item.type = USER_ITEM;
    user_item->base_item.content = content;
    user_item->init_function = init_function;
    user_item->loop_function = loop_function;
    user_item->exit_function = exit_function;
    return (vision_ui_list_item_t *) user_item;
}

static vision_ui_selector_t VISION_UI_SELECTOR = {};

const vision_ui_selector_t *vision_ui_selector_instance_get() {
    return &VISION_UI_SELECTOR;
}

extern vision_ui_selector_t *vision_ui_selector_mutable_instance_get() {
    return &VISION_UI_SELECTOR;
}

bool vision_ui_selector_bind_page(vision_ui_page_t *page) {
    if (page == NULL) {
        return false;
    }
    if (page->item_count == 0) {
        return false;
    }

    if (VISION_UI_SELECTOR.selected_item == NULL) {
        VISION_UI_SELECTOR.y_selector = 2 * VISION_UI_SCREEN_HEIGHT; // 给个初始坐标做动画
        VISION_UI_SELECTOR.h_selector = 160;
    }

    VISION_UI_SELECTOR.current_page = page;
    VISION_UI_SELECTOR.selected_index = 0;
    VISION_UI_SELECTOR.selected_item = page->items[0];
    VISION_UI_SELECTOR.scroll_bar_scale_page = NULL;
    VISION_UI_SELECTOR.scroll_bar_scale_part_shared = 0.f;

    return true;
}

void vision_ui_selector_go_next_item() {
    if (VISION_UI_SELECTOR.selected_item->type == SLIDER_ITEM &&
        vision_ui_to_list_slider_item(VISION_UI_SELECTOR.selected_item)->is_confirmed) {
        vision_ui_slider_item_t *selected_slider_item = vision_ui_to_list_slider_item(VISION_UI_SELECTOR.selected_item);
        selected_slider_item->value += selected_slider_item->value_step;
        if (selected_slider_item->value >= selected_slider_item->value_max) {
            selected_slider_item->value = selected_slider_item->value_max;
        }
        selected_slider_item->on_changed(selected_slider_item->value);
        return;
    }

    if (VISION_UI_SELECTOR.selected_item->type == USER_ITEM &&
        vision_ui_to_list_user_item(VISION_UI_SELECTOR.selected_item)->in_user_item) {
        return;
    }

    if (VISION_UI_SELECTOR.current_page == NULL || VISION_UI_SELECTOR.current_page->item_count == 0) {
        return;
    }

    // 到达最末端
    if (VISION_UI_SELECTOR.selected_index == VISION_UI_SELECTOR.current_page->item_count - 1) {
        VISION_UI_SELECTOR.selected_item = VISION_UI_SELECTOR.current_page->items[0];
        VISION_UI_SELECTOR.selected_index = 0;
        return;
    }

    VISION_UI_SELECTOR.selected_item = VISION_UI_SELECTOR.current_page->items[++VISION_UI_SELECTOR.selected_index];
}

void vision_ui_selector_go_prev_item() {
    if (VISION_UI_SELECTOR.selected_item->type == SLIDER_ITEM &&
        vision_ui_to_list_slider_item(VISION_UI_SELECTOR.selected_item)->is_confirmed) {
        vision_ui_slider_item_t *selected_slider_item = vision_ui_to_list_slider_item(VISION_UI_SELECTOR.selected_item);
        selected_slider_item->value -= selected_slider_item->value_step;
        if (selected_slider_item->value <= selected_slider_item->value_min) {
            selected_slider_item->value = selected_slider_item->value_min;
        }
        selected_slider_item->on_changed(selected_slider_item->value);
        return;
    }

    if (VISION_UI_SELECTOR.selected_item->type == USER_ITEM &&
        vision_ui_to_list_user_item(VISION_UI_SELECTOR.selected_item)->in_user_item) {
        return;
    }

    if (VISION_UI_SELECTOR.current_page == NULL || VISION_UI_SELECTOR.current_page->item_count == 0) {
        return;
    }

    // 到达最前端
    if (VISION_UI_SELECTOR.selected_index == 0) {
        VISION_UI_SELECTOR.selected_item =
                VISION_UI_SELECTOR.current_page->items[VISION_UI_SELECTOR.current_page->item_count - 1];
        VISION_UI_SELECTOR.selected_index = VISION_UI_SELECTOR.current_page->item_count - 1;
        return;
    }

    VISION_UI_SELECTOR.selected_item = VISION_UI_SELECTOR.current_page->items[--VISION_UI_SELECTOR.selected_index];
}

static bool VISION_UI_EXIT_ANIMATION_FINISHED = true;

extern bool vision_ui_exit_animation_is_finished() {
    return VISION_UI_EXIT_ANIMATION_FINISHED;
}

void vision_ui_exit_animation_set_is_finished() {
    VISION_UI_EXIT_ANIMATION_FINISHED = true;
}

/** @brief 确认当前选择的item
 * @note 如果选择了list 就进入选择的list
 * @note 如果选择了特殊item 就翻转/调整对应的值
 */
void vision_ui_selector_jump_to_selected_item() {
    if (!IS_IN_VISION_UI) {
        return;
    }

    if (VISION_UI_SELECTOR.selected_item->type == USER_ITEM) {
        VISION_UI_EXIT_ANIMATION_FINISHED = false;
        // vision_ui_selector.selected_item->in_user_item = true;
        vision_ui_user_item_t *selected_user_item = vision_ui_to_list_user_item(VISION_UI_SELECTOR.selected_item);
        selected_user_item->entering_user_item = true;
        selected_user_item->exiting_user_item = false;
        selected_user_item->user_item_inited = false;
        selected_user_item->user_item_looping = false;
        return;
    }

    if (VISION_UI_SELECTOR.selected_item->type == TITLE_ITEM) {
        vision_ui_selector_exit_current_item();
        return;
    }

    if (VISION_UI_SELECTOR.selected_item->type == SWITCH_ITEM) {
        vision_ui_switch_item_t *selected_switch_item = vision_ui_to_list_switch_item(VISION_UI_SELECTOR.selected_item);
        selected_switch_item->value = !selected_switch_item->value;
        selected_switch_item->on_changed(selected_switch_item->value);
        return;
    }

    if (VISION_UI_SELECTOR.selected_item->type == SLIDER_ITEM) {
        vision_ui_slider_item_t *selected_slider_item = vision_ui_to_list_slider_item(VISION_UI_SELECTOR.selected_item);
        if (!selected_slider_item->is_confirmed) {
            selected_slider_item->is_confirmed = true; // 如果没选中 就选中
            return;
        }
        if (selected_slider_item->is_confirmed) {
            selected_slider_item->is_confirmed = false;
            return;
        }
    }

    if (VISION_UI_SELECTOR.selected_item->child_page == NULL ||
        VISION_UI_SELECTOR.selected_item->child_page->item_count == 0) {
        return;
    }

    for (uint8_t i = 0; i < VISION_UI_SELECTOR.selected_item->child_page->item_count; i++) {
        VISION_UI_SELECTOR.selected_item->child_page->items[i]->y_list_item = 0;
    }

    VISION_UI_SELECTOR.selected_index = 0;
    VISION_UI_SELECTOR.current_page = VISION_UI_SELECTOR.selected_item->child_page;
    VISION_UI_SELECTOR.selected_item = VISION_UI_SELECTOR.current_page->items[0];
}

void vision_ui_selector_exit_current_item() {
    if (VISION_UI_SELECTOR.selected_item->type == SLIDER_ITEM &&
        vision_ui_to_list_slider_item(VISION_UI_SELECTOR.selected_item)->is_confirmed) {
        // 如果已选中又长按退出键
        vision_ui_slider_item_t *selected_slider_item = vision_ui_to_list_slider_item(VISION_UI_SELECTOR.selected_item);

        if (selected_slider_item->is_confirmed) {
            selected_slider_item->is_confirmed = false;
            return;
        }
    }

    if (VISION_UI_SELECTOR.selected_item->type == USER_ITEM &&
        vision_ui_to_list_user_item(VISION_UI_SELECTOR.selected_item)->in_user_item) {
        VISION_UI_EXIT_ANIMATION_FINISHED = false; // 需要重新绘制退场动画
        // vision_ui_selector.selected_item->in_user_item = false;
        vision_ui_user_item_t *selected_user_item = vision_ui_to_list_user_item(VISION_UI_SELECTOR.selected_item);
        selected_user_item->entering_user_item = false;
        selected_user_item->exiting_user_item = true;
        selected_user_item->user_item_inited = false;
        selected_user_item->user_item_looping = false;
        return;
    }

    if (VISION_UI_SELECTOR.current_page != NULL && VISION_UI_SELECTOR.current_page->layer == 0 && IS_IN_VISION_UI) {
        if (VISION_UI_ALLOW_EXIT_BY_USER) {
            IS_IN_VISION_UI = false;
        }
        return;
    }

    if (VISION_UI_SELECTOR.current_page != NULL && VISION_UI_SELECTOR.current_page->parent != NULL &&
        VISION_UI_SELECTOR.current_page->parent->item_count > 0) {
        vision_ui_page_t *parent_page = VISION_UI_SELECTOR.current_page->parent;
        for (uint8_t i = 0; i < parent_page->item_count; i++) {
            parent_page->items[i]->y_list_item = 0;
        }

        uint8_t temp_index = 0;
        for (uint8_t i = 0; i < parent_page->item_count; i++) {
            if (parent_page->items[i] == VISION_UI_SELECTOR.current_page->entry_item) {
                temp_index = i;
                break;
            }
        }

        VISION_UI_SELECTOR.current_page = parent_page;
        VISION_UI_SELECTOR.selected_index = temp_index;
        VISION_UI_SELECTOR.selected_item = parent_page->items[temp_index];
    }
}

bool vision_ui_page_push_item(vision_ui_page_t *page, vision_ui_list_item_t *child) {
    if (page == NULL) {
        return false;
    }
    if (child == NULL) {
        return false;
    }
    if (page->items == NULL) {
        return false;
    }
    if (page->item_count >= page->capacity) {
        return false;
    }
    if (page->layer >= VISION_UI_MAX_LIST_LAYER) {
        return false;
    }

    child->layer = page->layer + 1;
    child->page = page;

    vision_ui_font_set(vision_ui_font_get());
    float next_y = VISION_UI_LIST_TITLE_TO_DISPLAY_TOP_PADDING;
    if (page->item_count > 0) {
        const vision_ui_list_item_t *last_child = page->items[page->item_count - 1];
        const uint8_t gap_after_last =
                (last_child->type == TITLE_ITEM) ? VISION_UI_LIST_TITLE_TO_FRAME_PADDING : VISION_UI_LIST_FRAME_BETWEEN_PADDING;
        next_y = last_child->y_list_item_trg + VISION_UI_LIST_FRAME_FIXED_HEIGHT + gap_after_last;
    } else if (child->type != TITLE_ITEM) {
        // 没有标题时仍旧保留顶端 padding
        next_y = VISION_UI_LIST_TITLE_TO_DISPLAY_TOP_PADDING;
    }
    child->y_list_item_trg = next_y;

    if (child->child_page != NULL) {
        child->child_page->parent = page;
        child->child_page->layer = page->layer + 1;
        child->child_page->entry_item = child;
    }

    page->items[page->item_count++] = child;

    return true;
}

static vision_ui_camera_t VISION_UI_CAMERA = {0, 0, 0, 0}; // 在refresh加上camera的坐标

const vision_ui_camera_t *vision_ui_camera_instance_get() {
    return &VISION_UI_CAMERA;
}

extern vision_ui_camera_t *vision_ui_camera_mutable_instance_get() {
    return &VISION_UI_CAMERA;
}

void vision_ui_camera_instance_x_trg_set(const float x_trg) {
    VISION_UI_CAMERA.x_camera_trg = x_trg;
}

void vision_ui_camera_instance_y_trg_set(const float y_trg) {
    VISION_UI_CAMERA.y_camera_trg = y_trg;
}

void vision_ui_camera_bind_selector(vision_ui_selector_t *selector) {
    if (selector == NULL) {
        return;
    }

    VISION_UI_CAMERA.selector = selector; // 坐标在refresh内部更新
}
