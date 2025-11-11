//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//
#include "vision_ui_core.h"


#include <stdio.h>
#include <tgmath.h>

#include "../vision_ui_config.h"
#include "vision_ui_draw_driver.h"
#include "vision_ui_item.h"
#include "vision_ui_renderer.h"

bool IS_IN_VISION_UI;

static bool IS_BACKGROUND_FROZEN = false;
static vision_ui_list_item_t *SCROLL_BAR_SCALE_PARENT = NULL;
static float SCROLL_BAR_SCALE_VALUE = 0.f;

void vision_ui_render_init() {
    IS_IN_VISION_UI = true;
}

void vision_ui_animation_do(float *pos, const float pos_trg, const float speed) {
    if (*pos != pos_trg) {
        if (fabs(*pos - pos_trg) <= 1.0f) {
            *pos = pos_trg;
        } else {
            *pos += (pos_trg - *pos) / (100.0f - speed) / 1.0f;
        }
    }
}

static void vision_ui_notification_update() {
    vision_ui_animation_do(&vision_ui_notification_mutable_instance_get()->y_notification,
                           vision_ui_notification_mutable_instance_get()->y_notification_trg, 94);
    vision_ui_animation_do(&vision_ui_notification_mutable_instance_get()->w_notification,
                           vision_ui_notification_mutable_instance_get()->w_notification_trg, 95);
}

static void vision_ui_alert_update() {
    vision_ui_animation_do(&vision_ui_alert_mutable_instance_get()->y_alert, vision_ui_alert_mutable_instance_get()->y_alert_trg, 94);
    vision_ui_animation_do(&vision_ui_alert_mutable_instance_get()->w_alert, vision_ui_alert_mutable_instance_get()->w_alert_trg, 96);
}

static void vision_ui_camera_position_update() {
    if (vision_ui_camera_instance_get()->selector->y_selector_trg + VISION_UI_LIST_SELECTOR_FIXED_HEIGHT +
                vision_ui_camera_instance_get()->y_camera_trg >
        VISION_UI_SCREEN_HEIGHT) {
        // 向下超出屏幕 需要向下移动
        vision_ui_camera_instance_y_trg_set(VISION_UI_SCREEN_HEIGHT - vision_ui_camera_instance_get()->selector->y_selector_trg -
                                            VISION_UI_LIST_SELECTOR_FIXED_HEIGHT);
    }

    const float top_padding = VISION_UI_LIST_TITLE_TO_DISPLAY_TOP_PADDING;
    if (vision_ui_camera_instance_get()->selector->y_selector_trg + vision_ui_camera_instance_get()->y_camera_trg < top_padding) {
        // 向上超出屏幕 需要向上移动
        vision_ui_camera_instance_y_trg_set(top_padding - vision_ui_camera_instance_get()->selector->y_selector_trg);
    }

    vision_ui_camera_instance_x_trg_set(0);
    vision_ui_animation_do(&vision_ui_camera_mutable_instance_get()->x_camera, vision_ui_camera_instance_get()->x_camera_trg, 95);
    vision_ui_animation_do(&vision_ui_camera_mutable_instance_get()->y_camera, vision_ui_camera_instance_get()->y_camera_trg, 96);
}

static void vision_ui_widget_core_position_update() {
    // 需要调用所有的widget update
    vision_ui_notification_update();
    vision_ui_alert_update();
}

static void vision_ui_list_init() {
    // 做动画
    for (uint8_t i = 0; i < vision_ui_root_list_get()->child_num; i++) {
        vision_ui_list_item_t *list = vision_ui_root_list_get()->child_list_item[i];
        list->y_list_item = 0;
        list->scroll_bar_top = 0;
        list->scroll_bar_top_trg = 0;
        list->scroll_bar_height = 0;
        list->scroll_bar_height_trg = 0;
        list->scroll_bar_scale_part = 0;
        list->scroll_bar_scale_part_trg = 0;
        list->scroll_bar_top_px = 0;
        list->scroll_bar_height_px = VISION_UI_SCREEN_HEIGHT;
    }
    SCROLL_BAR_SCALE_PARENT = NULL;
    SCROLL_BAR_SCALE_VALUE = 0.f;
    vision_ui_selector_mutable_instance_get()->selected_index = 0;
    vision_ui_selector_mutable_instance_get()->selected_item = vision_ui_root_list_get()->child_list_item[0];
    vision_ui_selector_mutable_instance_get()->y_selector = VISION_UI_SCREEN_HEIGHT;
    vision_ui_selector_mutable_instance_get()->h_selector = VISION_UI_SCREEN_HEIGHT;
}

void vision_ui_core_init() {
    vision_ui_list_init();
    vision_ui_selector_t_selector_bind_item(vision_ui_root_list_get());
    vision_ui_camera_bind_selector(vision_ui_selector_mutable_instance_get());
}

static void vision_ui_list_item_position_update() {
    vision_ui_list_item_t *parent = vision_ui_selector_instance_get()->selected_item->parent;

    for (uint8_t i = 0; i < parent->child_num; i++) {
        vision_ui_animation_do(&parent->child_list_item[i]->y_list_item, parent->child_list_item[i]->y_list_item_trg, 84);
    }

    const uint8_t child_cnt = parent->child_num > 0 ? parent->child_num : 1;
    const float part = (float) VISION_UI_SCREEN_HEIGHT / child_cnt;
    const float slider_top_trg = part * vision_ui_selector_instance_get()->selected_index;
    const float slider_h_trg = fmaxf((float) VISION_UI_LIST_FRAME_FIXED_HEIGHT / child_cnt, part);

    parent->scroll_bar_top_trg = slider_top_trg;
    parent->scroll_bar_height_trg = slider_h_trg;
    parent->scroll_bar_scale_part_trg = part;

    if (parent != SCROLL_BAR_SCALE_PARENT) {
        parent->scroll_bar_scale_part = SCROLL_BAR_SCALE_VALUE;
        SCROLL_BAR_SCALE_PARENT = parent;
    }

    const bool scroll_bar_uninitialized = parent->scroll_bar_height == 0.f && parent->scroll_bar_height_trg == 0.f;
    if (scroll_bar_uninitialized) {
        parent->scroll_bar_top = parent->scroll_bar_top_trg;
        parent->scroll_bar_height = parent->scroll_bar_height_trg;
    } else {
        vision_ui_animation_do(&parent->scroll_bar_top, parent->scroll_bar_top_trg, VISION_UI_LIST_SCROLL_BAR_ANIMATION_SPEED);
        vision_ui_animation_do(&parent->scroll_bar_height, parent->scroll_bar_height_trg, VISION_UI_LIST_SCROLL_BAR_ANIMATION_SPEED);
    }

    const bool scroll_bar_scale_uninitialized =
            parent->scroll_bar_scale_part == 0.f && parent->scroll_bar_scale_part_trg == 0.f;
    if (scroll_bar_scale_uninitialized) {
        parent->scroll_bar_scale_part = parent->scroll_bar_scale_part_trg;
    } else {
        vision_ui_animation_do(&parent->scroll_bar_scale_part, parent->scroll_bar_scale_part_trg,
                               VISION_UI_LIST_SCROLL_BAR_ANIMATION_SPEED);
    }

    SCROLL_BAR_SCALE_VALUE = parent->scroll_bar_scale_part;

    int16_t slider_top_px = (int16_t) lrintf(parent->scroll_bar_top);
    int16_t slider_h_px = (int16_t) lrintf(parent->scroll_bar_height);
    if (slider_h_px < 1) {
        slider_h_px = 1;
    }
    if (slider_h_px > VISION_UI_SCREEN_HEIGHT) {
        slider_h_px = VISION_UI_SCREEN_HEIGHT;
    }
    if (slider_top_px < 0) {
        slider_top_px = 0;
    }
    if (slider_top_px > VISION_UI_SCREEN_HEIGHT - 1) {
        slider_top_px = VISION_UI_SCREEN_HEIGHT - 1;
    }
    if (slider_top_px + slider_h_px > VISION_UI_SCREEN_HEIGHT) {
        slider_top_px = VISION_UI_SCREEN_HEIGHT - slider_h_px;
    }

    parent->scroll_bar_top_px = slider_top_px;
    parent->scroll_bar_height_px = slider_h_px;
}

static void vision_ui_selector_position_update() {
    vision_ui_font_set(vision_ui_font_get());
    vision_ui_selector_mutable_instance_get()->h_selector_trg = VISION_UI_LIST_FRAME_FIXED_HEIGHT;
    vision_ui_selector_mutable_instance_get()->y_selector_trg = vision_ui_selector_instance_get()->selected_item->y_list_item_trg;
    const uint16_t selector_max_width = VISION_UI_LIST_HEADER_MAX_WIDTH + VISION_UI_LIST_HEADER_TO_TEXT_PADDING +
                                        VISION_UI_LIST_TEXT_MAX_WIDTH + VISION_UI_LIST_SELECTOR_TO_INNER_WIDGET_PADDING +
                                        VISION_UI_LIST_SELECTOR_TO_INNER_WIDGET_PADDING;
    const uint16_t selector_current_width =
            vision_ui_selector_instance_get()->selected_item->type == TITLE_ITEM
                    ? +vision_ui_driver_str_utf8_width_get(vision_ui_selector_instance_get()->selected_item->content) +
                              VISION_UI_LIST_SELECTOR_TO_INNER_WIDGET_PADDING + VISION_UI_LIST_SELECTOR_TO_INNER_WIDGET_PADDING
                    : +VISION_UI_LIST_HEADER_MAX_WIDTH + VISION_UI_LIST_HEADER_TO_TEXT_PADDING +
                              vision_ui_driver_str_utf8_width_get(vision_ui_selector_instance_get()->selected_item->content) +
                              VISION_UI_LIST_SELECTOR_TO_INNER_WIDGET_PADDING + VISION_UI_LIST_SELECTOR_TO_INNER_WIDGET_PADDING;
    vision_ui_selector_mutable_instance_get()->w_selector_trg =
            selector_current_width > selector_max_width ? selector_max_width : selector_current_width;
    vision_ui_animation_do(&vision_ui_selector_mutable_instance_get()->y_selector, vision_ui_selector_instance_get()->y_selector_trg, 91);
    vision_ui_animation_do(&vision_ui_selector_mutable_instance_get()->w_selector, vision_ui_selector_instance_get()->w_selector_trg, 92);
    vision_ui_animation_do(&vision_ui_selector_mutable_instance_get()->h_selector, vision_ui_selector_instance_get()->h_selector_trg, 93);
}

static void vision_ui_main_core_position_update() {
    vision_ui_list_item_position_update();
}

static void vision_ui_widget_core_step() {
    vision_ui_widget_core_position_update();
    vision_ui_widget_render();
}

static void vision_ui_main_core_step() {
    if (!IS_IN_VISION_UI) {
        return;
    }

    // 切换in user item的逻辑
    if (vision_ui_exit_animation_status_get() == EXIT_MASK_FALL_COMPLETE) {
        if (vision_ui_selector_instance_get()->selected_item->type == USER_ITEM) {
            vision_ui_user_item_t *selected_user_item = vision_ui_to_list_user_item(vision_ui_selector_instance_get()->selected_item);
            if (selected_user_item->entering_user_item) {
                selected_user_item->in_user_item = 1;
            } else if (selected_user_item->exiting_user_item) {
                if (selected_user_item->user_item_inited && selected_user_item->user_item_looping) {
                    selected_user_item->exit_function();
                }
                selected_user_item->in_user_item = 0;
            }
        }
    }

    // 渲染的逻辑
    if (vision_ui_selector_instance_get()->selected_item->type == USER_ITEM &&
        vision_ui_to_list_user_item(vision_ui_selector_instance_get()->selected_item)->in_user_item) {
        vision_ui_user_item_t *selected_user_item = vision_ui_to_list_user_item(vision_ui_selector_instance_get()->selected_item);
        // 初始化
        if (!selected_user_item->user_item_inited) {
            if (selected_user_item->init_function != NULL) {
                selected_user_item->init_function();
            }
            selected_user_item->user_item_inited = true;
        }

        if (selected_user_item->loop_function != NULL) {
            selected_user_item->user_item_looping = true;
            selected_user_item->loop_function();
        }
    } else {
        vision_ui_main_core_position_update();
        vision_ui_selector_position_update();
        vision_ui_camera_position_update();
        vision_ui_list_render();
    }

    IS_BACKGROUND_FROZEN = vision_ui_alert_instance_get()->is_running;

    // 退场动画
    // 上面都是正常应当绘制的内容 退场动画需要绘制时 只需要在上面的基础上绘制遮罩即可
    if (!vision_ui_exit_animation_is_finished()) {
        vision_ui_exit_animation_render();
    }
}

void vision_ui_step_render() {
    switch (vision_ui_driver_action_get()) {
        case UI_ACTION_GO_PREV:
            if (!vision_ui_is_background_frozen()) {
                vision_ui_selector_go_prev_item();
            }
            break;
        case UI_ACTION_GO_NEXT:
            if (!vision_ui_is_background_frozen()) {
                vision_ui_selector_go_next_item();
            }
            break;
        case UI_ACTION_EXIT:
            vision_ui_selector_exit_current_item();
            break;
        case UI_ACTION_ENTER:
            if (!vision_ui_is_background_frozen()) {
                vision_ui_selector_jump_to_selected_item();
            }
            break;
        default:
            break;
    }
    vision_ui_main_core_step(); // 核心处理函数
    vision_ui_widget_core_step(); // 弹窗处理函数
}

extern bool vision_ui_is_exited() {
    return !IS_IN_VISION_UI;
}

extern bool vision_ui_is_background_frozen() {
    return IS_BACKGROUND_FROZEN;
}
