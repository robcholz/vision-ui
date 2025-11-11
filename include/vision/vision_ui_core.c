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
    vision_ui_list_item_t *root = vision_ui_root_list_get();
    for (uint8_t i = 0; i < root->child_num; i++) {
        vision_ui_list_item_t *list = root->child_list_item[i];
        list->y_list_item = 0;

        vision_ui_page_t *page = list->page;
        if (page != NULL && page->view_type == VISION_UI_VIEW_LIST) {
            vision_ui_list_scroll_state_t *scroll = &page->view.list.scroll;
            scroll->top = 0.f;
            scroll->top_trg = 0.f;
            scroll->height = 0.f;
            scroll->height_trg = 0.f;
            scroll->scale_part = 0.f;
            scroll->scale_part_trg = 0.f;
            scroll->top_px = 0;
            scroll->height_px = VISION_UI_SCREEN_HEIGHT;
        }
    }
    vision_ui_selector_t *selector = vision_ui_selector_mutable_instance_get();
    selector->scroll_bar_scale_parent = NULL;
    selector->scroll_bar_scale_part_shared = 0.f;
    selector->selected_index = 0;
    selector->selected_item = root->child_list_item[0];
    selector->y_selector = VISION_UI_SCREEN_HEIGHT;
    selector->h_selector = VISION_UI_SCREEN_HEIGHT;
    vision_ui_page_active_set(vision_ui_page_root_get());
}

void vision_ui_core_init() {
    vision_ui_list_init();
    vision_ui_selector_t_selector_bind_item(vision_ui_root_list_get());
    vision_ui_camera_bind_selector(vision_ui_selector_mutable_instance_get());
}

static void vision_ui_list_item_position_update() {
    vision_ui_selector_t *selector_mut = vision_ui_selector_mutable_instance_get();
    const vision_ui_selector_t *selector = vision_ui_selector_instance_get();
    if (selector->selected_item == NULL) {
        return;
    }

    vision_ui_list_item_t *parent_item = selector->selected_item->parent;
    if (parent_item == NULL) {
        return;
    }

    for (uint8_t i = 0; i < parent_item->child_num; i++) {
        vision_ui_animation_do(&parent_item->child_list_item[i]->y_list_item, parent_item->child_list_item[i]->y_list_item_trg, 84);
    }

    vision_ui_page_t *page = parent_item->page;
    if (page == NULL || page->view_type != VISION_UI_VIEW_LIST) {
        return;
    }

    vision_ui_list_scroll_state_t *scroll = &page->view.list.scroll;

    const uint8_t child_cnt = parent_item->child_num > 0 ? parent_item->child_num : 1;
    const float part = (float) VISION_UI_SCREEN_HEIGHT / child_cnt;
    const float slider_top_trg = part * selector->selected_index;
    const float slider_h_trg = fmaxf((float) VISION_UI_LIST_FRAME_FIXED_HEIGHT / child_cnt, part);

    scroll->top_trg = slider_top_trg;
    scroll->height_trg = slider_h_trg;
    scroll->scale_part_trg = part;

    vision_ui_page_t *const prev_page = selector_mut->scroll_bar_scale_parent;
    if (page != prev_page) {
        if (prev_page != NULL && prev_page->view_type == VISION_UI_VIEW_LIST) {
            scroll->top = prev_page->view.list.scroll.top;
            scroll->height = prev_page->view.list.scroll.height;
        }
        scroll->scale_part = selector_mut->scroll_bar_scale_part_shared;
        selector_mut->scroll_bar_scale_parent = page;
    }

    const bool scroll_bar_uninitialized = scroll->height == 0.f && scroll->height_trg == 0.f;
    if (scroll_bar_uninitialized) {
        scroll->top = scroll->top_trg;
        scroll->height = scroll->height_trg;
    } else {
        vision_ui_animation_do(&scroll->top, scroll->top_trg, VISION_UI_LIST_SCROLL_BAR_ANIMATION_SPEED);
        vision_ui_animation_do(&scroll->height, scroll->height_trg, VISION_UI_LIST_SCROLL_BAR_ANIMATION_SPEED);
    }

    const bool scroll_bar_scale_uninitialized = scroll->scale_part == 0.f && scroll->scale_part_trg == 0.f;
    if (scroll_bar_scale_uninitialized) {
        scroll->scale_part = scroll->scale_part_trg;
    } else {
        vision_ui_animation_do(&scroll->scale_part, scroll->scale_part_trg, VISION_UI_LIST_SCROLL_BAR_ANIMATION_SPEED);
    }

    selector_mut->scroll_bar_scale_part_shared = scroll->scale_part;

    int16_t slider_top_px = (int16_t) lrintf(scroll->top);
    int16_t slider_h_px = (int16_t) lrintf(scroll->height);
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

    scroll->top_px = slider_top_px;
    scroll->height_px = slider_h_px;
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

    vision_ui_list_item_t *selected_item = vision_ui_selector_instance_get()->selected_item;
    vision_ui_page_t *selected_page = vision_ui_page_from_item(selected_item);
    vision_ui_custom_view_t *custom_view = NULL;

    if (selected_item != NULL && selected_page != NULL && selected_page->view_type == VISION_UI_VIEW_CUSTOM) {
        custom_view = &selected_page->view.custom;
    }

    if (vision_ui_exit_animation_status_get() == EXIT_MASK_FALL_COMPLETE && custom_view != NULL) {
        if (custom_view->entering) {
            custom_view->entering = false;
            custom_view->active = true;
            vision_ui_page_active_set(selected_page);
        } else if (custom_view->exiting) {
            if (custom_view->is_initialized && custom_view->is_looping && custom_view->exit_function != NULL) {
                custom_view->exit_function();
            }
            custom_view->active = false;
            custom_view->exiting = false;
            custom_view->is_looping = false;
            custom_view->is_initialized = false;
            vision_ui_page_active_set(selected_page->parent);
        }
    }

    if (custom_view != NULL && custom_view->active) {
        if (!custom_view->is_initialized && custom_view->init_function != NULL) {
            custom_view->init_function();
            custom_view->is_initialized = true;
        }

        if (custom_view->loop_function != NULL) {
            custom_view->is_looping = true;
            custom_view->loop_function();
        }
    } else {
        vision_ui_page_t *active_page = vision_ui_page_active_get();
        if (active_page != NULL) {
            switch (active_page->view_type) {
                case VISION_UI_VIEW_LIST:
                    vision_ui_main_core_position_update();
                    vision_ui_selector_position_update();
                    vision_ui_camera_position_update();
                    vision_ui_list_render();
                    break;
                case VISION_UI_VIEW_ICON:
                    vision_ui_icon_render(active_page);
                    break;
                case VISION_UI_VIEW_CUSTOM:
                    break;
            }
        }
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
