//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//
#include "vision_ui_core.h"

#include <stdio.h>
#include <tgmath.h>

#include "vision_ui_drawer.h"
#include "vision_ui_draw_driver.h"

bool IS_IN_ASTRA;

static bool IS_BACKGROUND_FROZEN = false;

static bool vision_ui_start() {
    /**自行修改**/
    if (IS_IN_ASTRA) {
        return true;
    }
    static int64_t key_press_span = 0;
    static uint32_t key_start_time = 0;
    static bool key_clicked = false;
    static char msg[100] = {};

    if (true) {
        if (!key_clicked) {
            key_clicked = true;
            key_start_time = get_ticks_ms();
            //变量上限是0xFFFF 65535
        }
        if (get_ticks_ms() - key_start_time > 1000 && key_clicked) {
            key_press_span = get_ticks_ms() - key_start_time;
            if (key_press_span <= 2500) {
                sprintf(msg, "继续长按%.2f秒进入.", (2500 - key_press_span) / 1000.0f);
                astra_push_info_bar(msg, 2000);
            } else if (key_press_span > 2500) {
                astra_push_info_bar("have fun! :p", 2000);
                IS_IN_ASTRA = true;
                astra_init_list();
                key_clicked = false;
                key_start_time = 0;
                key_press_span = 0;
            }
        }
    } else {
        key_clicked = false;
        if (key_press_span != 0) {
            astra_push_info_bar("bye!", 2000);
            key_press_span = 0;
        }
    }
    return false;
}

void vision_ui_render_init() {
    bool init = false;
    while (!init) {
        oled_clear_buffer();
        init = vision_ui_start();
        oled_send_buffer();
    }
}

void astra_animation(float* pos, float pos_trg, float speed) {
    if (*pos != pos_trg) {
        if (fabs(*pos - pos_trg) <= 1.0f) *pos = pos_trg;
        else *pos += (pos_trg - *pos) / (100.0f - speed) / 1.0f;
    }
}

void astra_refresh_info_bar() {
    astra_animation(&ASTRA_INFO_BAR.y_info_bar, ASTRA_INFO_BAR.y_info_bar_trg, 94);
    astra_animation(&ASTRA_INFO_BAR.w_info_bar, ASTRA_INFO_BAR.w_info_bar_trg, 95);
}

void astra_refresh_pop_up() {
    astra_animation(&ASTRA_POP_UP.y_pop_up, ASTRA_POP_UP.y_pop_up_trg, 94);
    astra_animation(&ASTRA_POP_UP.w_pop_up, ASTRA_POP_UP.w_pop_up_trg, 96);
}

void astra_refresh_camera_position() {
    if (ASTRA_CAMERA.selector->y_selector_trg + LIST_SELECTOR_FIXED_HEIGHT + ASTRA_CAMERA.y_camera_trg > SCREEN_HEIGHT) {
        //向下超出屏幕 需要向下移动
        ASTRA_CAMERA.y_camera_trg = SCREEN_HEIGHT - ASTRA_CAMERA.selector->y_selector_trg - LIST_SELECTOR_FIXED_HEIGHT;
    }

    const float top_padding = LIST_TITLE_TO_DISPLAY_TOP_PADDING;
    if (ASTRA_CAMERA.selector->y_selector_trg + ASTRA_CAMERA.y_camera_trg < top_padding) {
        //向上超出屏幕 需要向上移动
        ASTRA_CAMERA.y_camera_trg = top_padding - ASTRA_CAMERA.selector->y_selector_trg;
    }

    ASTRA_CAMERA.x_camera_trg = 0;
    astra_animation(&ASTRA_CAMERA.x_camera, ASTRA_CAMERA.x_camera_trg, 95);
    astra_animation(&ASTRA_CAMERA.y_camera, ASTRA_CAMERA.y_camera_trg, 96);
}

void astra_refresh_widget_core_position() {
    //需要调用所有的widget refresh函数
    astra_refresh_info_bar();
    astra_refresh_pop_up();
}

void astra_init_list() {
    //做动画
    for (uint8_t i = 0; i < astra_get_root_list()->child_num; i++) {
        astra_list_item_t* list = astra_get_root_list()->child_list_item[i];
        list->y_list_item = 0;
        list->scroll_bar_top = 0;
        list->scroll_bar_top_trg = 0;
        list->scroll_bar_height = 0;
        list->scroll_bar_height_trg = 0;
        list->scroll_bar_top_px = 0;
        list->scroll_bar_height_px = SCREEN_HEIGHT;
    }
    ASTRA_SELECTOR.selected_index = 0;
    ASTRA_SELECTOR.selected_item = astra_get_root_list()->child_list_item[0];
    ASTRA_SELECTOR.y_selector = SCREEN_HEIGHT;
    ASTRA_SELECTOR.h_selector = SCREEN_HEIGHT;
}

void astra_init_core() {
    astra_init_list();
    astra_bind_item_to_selector(astra_get_root_list());
    astra_bind_selector_to_camera(astra_get_selector());
}

void astra_refresh_list_item_position() {
    astra_list_item_t* parent = ASTRA_SELECTOR.selected_item->parent;

    for (uint8_t i = 0; i < parent->child_num; i++)
        astra_animation(&parent->child_list_item[i]->y_list_item,
                        parent->child_list_item[i]->y_list_item_trg, 84);

    const uint8_t child_cnt = parent->child_num > 0 ? parent->child_num : 1;
    const float part = (float) SCREEN_HEIGHT / child_cnt;
    const float slider_top_trg = part * ASTRA_SELECTOR.selected_index;
    const float slider_h_trg = fmaxf((float) LIST_FRAME_FIXED_HEIGHT / child_cnt, part);

    parent->scroll_bar_top_trg = slider_top_trg;
    parent->scroll_bar_height_trg = slider_h_trg;

    const bool scroll_bar_uninitialized = parent->scroll_bar_height == 0.f && parent->scroll_bar_height_trg == 0.f;
    if (scroll_bar_uninitialized) {
        parent->scroll_bar_top = parent->scroll_bar_top_trg;
        parent->scroll_bar_height = parent->scroll_bar_height_trg;
    } else {
        astra_animation(&parent->scroll_bar_top, parent->scroll_bar_top_trg, LIST_SCROLL_BAR_ANIMATION_SPEED);
        astra_animation(&parent->scroll_bar_height, parent->scroll_bar_height_trg, LIST_SCROLL_BAR_ANIMATION_SPEED);
    }

    int16_t slider_top_px = (int16_t) lrintf(parent->scroll_bar_top);
    int16_t slider_h_px = (int16_t) lrintf(parent->scroll_bar_height);
    if (slider_h_px < 1) slider_h_px = 1;
    if (slider_h_px > SCREEN_HEIGHT) slider_h_px = SCREEN_HEIGHT;
    if (slider_top_px < 0) slider_top_px = 0;
    if (slider_top_px > SCREEN_HEIGHT - 1) slider_top_px = SCREEN_HEIGHT - 1;
    if (slider_top_px + slider_h_px > SCREEN_HEIGHT)
        slider_top_px = SCREEN_HEIGHT - slider_h_px;

    parent->scroll_bar_top_px = slider_top_px;
    parent->scroll_bar_height_px = slider_h_px;
}

void astra_refresh_selector_position() {
    astra_set_font(ASTRA_FONT);
    ASTRA_SELECTOR.h_selector_trg = LIST_FRAME_FIXED_HEIGHT;
    ASTRA_SELECTOR.y_selector_trg = ASTRA_SELECTOR.selected_item->y_list_item_trg;
    const uint16_t selector_max_width = LIST_HEADER_MAX_WIDTH + LIST_HEADER_TO_TEXT_PADDING +
                                        LIST_TEXT_MAX_WIDTH + LIST_SELECTOR_TO_INNER_WIDGET_PADDING + LIST_SELECTOR_TO_INNER_WIDGET_PADDING;
    const uint16_t selector_current_width = ASTRA_SELECTOR.selected_item->type == TITLE_ITEM
                                                ? +oled_get_UTF8_width(
                                                      ASTRA_SELECTOR.selected_item->content) + LIST_SELECTOR_TO_INNER_WIDGET_PADDING
                                                  + LIST_SELECTOR_TO_INNER_WIDGET_PADDING
                                                : +LIST_HEADER_MAX_WIDTH + LIST_HEADER_TO_TEXT_PADDING
                                                  +
                                                  oled_get_UTF8_width(
                                                      ASTRA_SELECTOR.selected_item->content) + LIST_SELECTOR_TO_INNER_WIDGET_PADDING
                                                  + LIST_SELECTOR_TO_INNER_WIDGET_PADDING;
    ASTRA_SELECTOR.w_selector_trg = selector_current_width > selector_max_width ? selector_max_width : selector_current_width;
    astra_animation(&ASTRA_SELECTOR.y_selector, ASTRA_SELECTOR.y_selector_trg, 91);
    astra_animation(&ASTRA_SELECTOR.w_selector, ASTRA_SELECTOR.w_selector_trg, 92);
    astra_animation(&ASTRA_SELECTOR.h_selector, ASTRA_SELECTOR.h_selector_trg, 93);
}

void astra_refresh_main_core_position() {
    astra_refresh_list_item_position();
}

void vision_ui_render_loop() {
    switch (vision_ui_get_ui_action()) {
        case UI_ACTION_GO_PREV:
            if (!vision_ui_is_background_frozon()) {
                astra_selector_go_prev_item();
            }
            break;
        case UI_ACTION_GO_NEXT:
            if (!vision_ui_is_background_frozon()) {
                astra_selector_go_next_item();
            }
            break;
        case UI_ACTION_EXIT:
            astra_selector_exit_current_item();
            break;
        case UI_ACTION_ENTER:
            if (!vision_ui_is_background_frozon()) {
                astra_selector_jump_to_selected_item();
            }
            break;
        default:
            break;
    }
    astra_ui_main_core(); // 核心处理函数
    astra_ui_widget_core(); // 弹窗处理函数
}

void astra_ui_widget_core() {
    astra_refresh_widget_core_position();
    astra_draw_widget();
}

void astra_ui_main_core() {
    if (!IS_IN_ASTRA) return;

    //切换in user item的逻辑
    if (ASTRA_EXIT_ANIMATION_STATUS == 1) {
        if (ASTRA_SELECTOR.selected_item->type == USER_ITEM) {
            astra_user_item_t* selected_user_item = astra_to_user_item(ASTRA_SELECTOR.selected_item);
            if (selected_user_item->entering_user_item)
                selected_user_item->in_user_item = 1;
            else if (selected_user_item->exiting_user_item) {
                if (selected_user_item->user_item_inited && selected_user_item->user_item_looping)
                    selected_user_item->exit_function();
                selected_user_item->in_user_item = 0;
            }
        }
    }

    //渲染的逻辑
    if (ASTRA_SELECTOR.selected_item->type == USER_ITEM && astra_to_user_item(ASTRA_SELECTOR.selected_item)->in_user_item) {
        astra_user_item_t* selected_user_item = astra_to_user_item(ASTRA_SELECTOR.selected_item);
        //初始化
        if (!selected_user_item->user_item_inited) {
            if (selected_user_item->init_function != NULL)
                selected_user_item->init_function();
            selected_user_item->user_item_inited = true;
        }

        if (selected_user_item->loop_function != NULL) {
            selected_user_item->user_item_looping = true;
            selected_user_item->loop_function();
        }
    } else {
        astra_refresh_main_core_position();
        astra_refresh_selector_position();
        astra_refresh_camera_position();
        astra_draw_list();
    }

    IS_BACKGROUND_FROZEN = ASTRA_POP_UP.is_running;

    //退场动画
    //上面都是正常应当绘制的内容 退场动画需要绘制时 只需要在上面的基础上绘制遮罩即可
    if (!ASTRA_EXIT_ANIMATION_FINISHED)
        astra_draw_exit_animation();
}

extern bool vision_ui_is_exited() {
    return !IS_IN_ASTRA;
}

extern bool vision_ui_is_background_frozon() {
    return IS_BACKGROUND_FROZEN;
}
