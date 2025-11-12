//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//
#include "vision_ui_renderer.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../vision_ui_config.h"
#include "vision_ui_core.h"
#include "vision_ui_item.h"

typedef struct {
    uint8_t total;
    uint8_t col[3];
} vision_ui_block3x3_t;

static vision_ui_block3x3_t vision_ui_sample_block3x3(const uint8_t* buffer, const uint16_t cx, const uint16_t cy) {
    vision_ui_block3x3_t sample = {0, {0, 0, 0}};
    if (buffer == NULL) {
        return sample;
    }

    const int16_t x_start = cx == 0 ? 0 : (int16_t) cx - 1;
    const int16_t x_end = cx + 1 >= VISION_UI_SCREEN_WIDTH ? VISION_UI_SCREEN_WIDTH - 1 : (int16_t) cx + 1;
    const int16_t y_start = cy == 0 ? 0 : (int16_t) cy - 1;
    const int16_t y_end = cy + 1 >= VISION_UI_SCREEN_HEIGHT ? VISION_UI_SCREEN_HEIGHT - 1 : (int16_t) cy + 1;

    for (int16_t y = y_start; y <= y_end; ++y) {
        const uint16_t page = (uint16_t) y >> 3;
        const uint8_t mask = (uint8_t) (1u << (y & 0x7));
        const uint32_t row_offset = (uint32_t) page * VISION_UI_SCREEN_WIDTH;
        for (int16_t x = x_start; x <= x_end; ++x) {
            if (buffer[row_offset + x] & mask) {
                sample.total++;
                const uint8_t col_idx = (uint8_t) (x - x_start);
                if (col_idx < 3) {
                    sample.col[col_idx]++;
                }
            }
        }
    }
    return sample;
}

static bool vision_ui_block_is_thin_vertical(const vision_ui_block3x3_t* sample) {
    const uint8_t side_sum = (uint8_t) (sample->col[0] + sample->col[2]);
    return (sample->col[1] >= 2) && (side_sum <= 1);
}

static void vision_ui_background_blur_animation_render(const uint16_t x0, const uint16_t y0, uint16_t width, uint16_t height,
                                                       const uint8_t fade_level) {
    if (x0 + width > VISION_UI_SCREEN_WIDTH) {
        width = VISION_UI_SCREEN_WIDTH - x0;
    }
    if (y0 + height > VISION_UI_SCREEN_HEIGHT) {
        height = VISION_UI_SCREEN_HEIGHT - y0;
    }

    if (fade_level < 1 || fade_level > 5) {
        return;
    }

    const uint8_t* buffer_live = vision_ui_driver_buffer_pointer_get();
    if (buffer_live == NULL) {
        return;
    }

    // 定义2x2网格的渐隐模式
    // 每个数组表示一个2x2网格中哪些像素需要熄灭
    // 0表示保持亮，1表示熄灭
    static const uint8_t patterns[5][2][2] = {{{0, 0}, // Level 1: 全亮
                                               {0, 0}},
                                              {{1, 0}, // Level 2: 左上角熄灭
                                               {0, 0}},
                                              {{1, 0}, // Level 3: 左上角和右下角熄灭
                                               {0, 1}},
                                              {{1, 0}, // Level 4: 只保留右上角
                                               {1, 1}},
                                              {{1, 1}, // Level 5: 全暗
                                               {1, 1}}};

    const uint16_t x_end = x0 + width;
    const uint16_t y_end = y0 + height;
    const uint16_t row_bits = width;
    const uint16_t row_mask_bytes = (uint16_t) ((row_bits + 7u) >> 3);
    uint8_t row_mask_prev[(VISION_UI_SCREEN_WIDTH + 7) / 8];
    uint8_t row_mask_curr[(VISION_UI_SCREEN_WIDTH + 7) / 8];
    memset(row_mask_prev, 0, row_mask_bytes);
    memset(row_mask_curr, 0, row_mask_bytes);

    bool has_prev_row = false;
    uint16_t prev_row_y = 0;

    for (uint16_t y = y0; y < y_end; y++) {
        memset(row_mask_curr, 0, row_mask_bytes);

        for (uint16_t x = x0; x < x_end; x++) {
            const uint16_t grid_x = (x - x0) & 0x1u;
            const uint16_t grid_y = (y - y0) & 0x1u;

            if (!patterns[fade_level - 1][grid_y][grid_x]) {
                continue;
            }

            const vision_ui_block3x3_t block = vision_ui_sample_block3x3(buffer_live, x, y);

            if (block.total <= 1) {
                continue;
            }

            if (fade_level >= 2 && fade_level <= 4) {
                if (vision_ui_block_is_thin_vertical(&block)) {
                    if (((y + fade_level) & 1u) == 0) {
                        continue;
                    }
                }
            }

            const uint16_t local_x = (uint16_t) (x - x0);
            row_mask_curr[local_x >> 3] |= (uint8_t) (1u << (local_x & 0x7));
        }

        if (has_prev_row) {
            vision_ui_driver_color_draw(0);
            for (uint16_t local_x = 0; local_x < row_bits; ++local_x) {
                if (row_mask_prev[local_x >> 3] & (uint8_t) (1u << (local_x & 0x7))) {
                    vision_ui_driver_pixel_draw(x0 + local_x, prev_row_y);
                }
            }
        } else {
            has_prev_row = true;
        }

        memcpy(row_mask_prev, row_mask_curr, row_mask_bytes);
        prev_row_y = y;
    }

    if (has_prev_row) {
        vision_ui_driver_color_draw(0);
        for (uint16_t local_x = 0; local_x < row_bits; ++local_x) {
            if (row_mask_prev[local_x >> 3] & (uint8_t) (1u << (local_x & 0x7))) {
                vision_ui_driver_pixel_draw(x0 + local_x, prev_row_y);
            }
        }
    }

    vision_ui_driver_color_draw(1);
}

static bool vision_ui_icon_view_is_active() {
    const vision_ui_selector_t* selector = vision_ui_selector_instance_get();
    return selector->selected_item != NULL && selector->selected_item->parent != NULL && selector->selected_item->parent->icon_view_mode;
}

void vision_ui_exit_animation_render() {
    static uint8_t fadeout_sequence;
    static uint8_t state = 0;
    if (state == 0) {
        fadeout_sequence = 1;
        state = 1;
    }
    if (state == 1) {
        vision_ui_background_blur_animation_render(0, 0, VISION_UI_SCREEN_WIDTH, VISION_UI_SCREEN_HEIGHT, fadeout_sequence);
        ++fadeout_sequence;
        if (fadeout_sequence > 5) {
            vision_ui_exit_animation_set_is_finished();
            state = 0;
        }
    }
}

static void vision_ui_notification_render() {
    if (!vision_ui_notification_instance_get()->is_running) {
        return;
    }

    const uint32_t now = vision_ui_driver_ticks_ms_get();

    if (vision_ui_notification_instance_get()->is_dismissing) {
        const bool reached_target =
                vision_ui_notification_instance_get()->y_notification == vision_ui_notification_instance_get()->y_notification_trg;
        const bool removal_time_elapsed =
                now - vision_ui_notification_instance_get()->dismiss_start >= VISION_UI_NOTIFICATION_DISMISS_DURATION_MS;

        if (removal_time_elapsed || reached_target) {
            if (!reached_target) {
                vision_ui_notification_mutable_instance_get()->y_notification = vision_ui_notification_instance_get()->y_notification_trg;
            }

            if (vision_ui_notification_instance_get()->has_pending) {
                vision_ui_notification_mutable_instance_get()->is_dismissing = false;
                vision_ui_notification_mutable_instance_get()->content = vision_ui_notification_instance_get()->pending_content;
                vision_ui_notification_mutable_instance_get()->span = vision_ui_notification_instance_get()->pending_span;
                vision_ui_notification_mutable_instance_get()->pending_content = NULL;
                vision_ui_notification_mutable_instance_get()->pending_span = 0;
                vision_ui_notification_mutable_instance_get()->has_pending = false;
                vision_ui_notification_mutable_instance_get()->time_start = now;
                vision_ui_notification_mutable_instance_get()->time = now;
                vision_ui_notification_mutable_instance_get()->y_notification_trg = 0;
                vision_ui_notification_mutable_instance_get()->dismiss_start = 0;
                vision_ui_font_set(vision_ui_font_get());
                vision_ui_notification_mutable_instance_get()->w_notification_trg =
                        vision_ui_driver_str_utf8_width_get(vision_ui_notification_instance_get()->content) + VISION_UI_NOTIFICATION_WIDTH;
                vision_ui_notification_mutable_instance_get()->is_running = true;
            } else {
                vision_ui_notification_mutable_instance_get()->is_running = false;
                vision_ui_notification_mutable_instance_get()->is_dismissing = false;
                vision_ui_notification_mutable_instance_get()->dismiss_start = 0;
                vision_ui_notification_mutable_instance_get()->has_pending = false;
                vision_ui_notification_mutable_instance_get()->pending_content = NULL;
                vision_ui_notification_mutable_instance_get()->pending_span = 0;
                return;
            }
        }
    } else {
        if (vision_ui_notification_instance_get()->y_notification == vision_ui_notification_instance_get()->y_notification_trg) {
            vision_ui_notification_mutable_instance_get()->time = now;
        }

        if (vision_ui_notification_instance_get()->time - vision_ui_notification_instance_get()->time_start >=
            vision_ui_notification_instance_get()->span) {
            vision_ui_notification_mutable_instance_get()->y_notification_trg = 0 - 2 * VISION_UI_NOTIFICATION_HEIGHT;
            vision_ui_notification_mutable_instance_get()->is_dismissing = true;
            vision_ui_notification_mutable_instance_get()->dismiss_start = now;
        }
    }

    if (!vision_ui_notification_instance_get()->is_running) {
        return;
    }

    const int16_t x_notification = VISION_UI_SCREEN_WIDTH / 2 - vision_ui_notification_instance_get()->w_notification / 2;
    const int16_t y_notification_1 = vision_ui_notification_instance_get()->y_notification - 4;
    const int16_t y_notification_2 = vision_ui_notification_instance_get()->y_notification + VISION_UI_NOTIFICATION_HEIGHT;

    vision_ui_font_set(vision_ui_font_get());

    vision_ui_driver_color_draw(0); // 黑遮罩打底
    vision_ui_driver_box_r_draw((int16_t) (VISION_UI_SCREEN_WIDTH / 2 - (vision_ui_notification_instance_get()->w_notification + 4) / 2),
                                y_notification_1, (int16_t) (vision_ui_notification_instance_get()->w_notification + 4),
                                VISION_UI_NOTIFICATION_HEIGHT + 6, 4);

    vision_ui_driver_color_draw(1);
    vision_ui_driver_box_r_draw(x_notification, y_notification_1, (int16_t) vision_ui_notification_instance_get()->w_notification,
                                VISION_UI_NOTIFICATION_HEIGHT + 4, 3);
    // 向上移动四个像素 同时向下多画四个像素 只用下半部分圆角

    vision_ui_driver_color_draw(2);
    vision_ui_driver_line_h_draw(x_notification + 2, y_notification_2 - 2,
                                 (int16_t) (vision_ui_notification_instance_get()->w_notification - 4));
    vision_ui_driver_pixel_draw(x_notification + 1, y_notification_2 - 3);
    vision_ui_driver_pixel_draw(x_notification + 1, y_notification_2 - 3);

    const int16_t text_w = vision_ui_driver_str_width_get(vision_ui_notification_instance_get()->content);
    const int16_t text_h = vision_ui_driver_str_height_get();
    const int16_t text_x = x_notification + (int16_t) ((vision_ui_notification_instance_get()->w_notification - text_w) / 2);
    const int16_t text_y = (int16_t) (vision_ui_notification_instance_get()->y_notification + vision_ui_driver_str_height_get() - 2);

    vision_ui_driver_color_draw(0);
    vision_ui_driver_box_draw(text_x, text_y - text_h, text_w, text_h);

    vision_ui_driver_color_draw(1);
    vision_ui_driver_str_utf8_draw(text_x, text_y, vision_ui_notification_instance_get()->content);

    vision_ui_driver_color_draw(2);
    vision_ui_driver_box_draw(text_x, text_y - text_h, text_w, text_h);
}

static void vision_ui_alert_render() {
    if (!vision_ui_alert_instance_get()->is_running) {
        return;
    }

    // 弹窗到位后才开始计算时间
    if (vision_ui_alert_instance_get()->y_alert == vision_ui_alert_instance_get()->y_alert_trg) {
        vision_ui_alert_mutable_instance_get()->time = vision_ui_driver_ticks_ms_get();
    }

    // 时间到了就收回
    if (vision_ui_alert_instance_get()->time - vision_ui_alert_instance_get()->time_start >= vision_ui_alert_instance_get()->span) {
        vision_ui_alert_mutable_instance_get()->y_alert_trg = 0 - 2 * VISION_UI_ALERT_HEIGHT; // 收回
        if (vision_ui_alert_instance_get()->y_alert == vision_ui_alert_instance_get()->y_alert_trg) {
            vision_ui_alert_mutable_instance_get()->is_running = false; // 等归位后结束生命周期
        }
    }

    const int16_t x_alert = VISION_UI_SCREEN_WIDTH / 2 - vision_ui_alert_instance_get()->w_alert / 2;
    const int16_t y_alert = vision_ui_alert_instance_get()->y_alert + VISION_UI_ALERT_HEIGHT;

    vision_ui_font_set(vision_ui_font_get());

    vision_ui_driver_color_draw(0); // 黑遮罩
    vision_ui_driver_box_r_draw((int16_t) (VISION_UI_SCREEN_WIDTH / 2 - (vision_ui_alert_instance_get()->w_alert + 4) / 2 - 2),
                                (int16_t) (vision_ui_alert_instance_get()->y_alert - 2),
                                (int16_t) (vision_ui_alert_instance_get()->w_alert + 8), VISION_UI_ALERT_HEIGHT + 4, 5);

    vision_ui_driver_color_draw(1);
    vision_ui_driver_box_r_draw(x_alert - 2, (int16_t) vision_ui_alert_instance_get()->y_alert,
                                (int16_t) (vision_ui_alert_instance_get()->w_alert + 4), VISION_UI_ALERT_HEIGHT, 3);

    vision_ui_driver_color_draw(2);
    vision_ui_driver_line_h_draw(x_alert, y_alert - 2, (int16_t) vision_ui_alert_instance_get()->w_alert);
    vision_ui_driver_pixel_draw(x_alert - 1, y_alert - 3);
    vision_ui_driver_pixel_draw((int16_t) (VISION_UI_SCREEN_WIDTH / 2 + vision_ui_alert_instance_get()->w_alert / 2), y_alert - 3);

    const int16_t text_w = vision_ui_driver_str_width_get(vision_ui_alert_instance_get()->content);
    const int16_t text_h = vision_ui_driver_str_height_get();
    const int16_t text_x = x_alert + (int16_t) ((vision_ui_alert_instance_get()->w_alert - text_w) / 2);
    const int16_t text_y = (int16_t) (vision_ui_alert_instance_get()->y_alert + vision_ui_driver_str_height_get() + 1);

    vision_ui_driver_color_draw(0);
    vision_ui_driver_box_draw(text_x, text_y - text_h, text_w, text_h);

    vision_ui_driver_color_draw(1);
    vision_ui_driver_str_utf8_draw(text_x, text_y, vision_ui_alert_instance_get()->content);

    vision_ui_driver_color_draw(2);
    vision_ui_driver_box_draw(text_x, text_y - text_h, text_w, text_h);
}

static void vision_ui_list_appearance_render() {
    vision_ui_driver_color_draw(1);

    vision_ui_driver_line_v_draw(VISION_UI_SCREEN_WIDTH - 2, 0, VISION_UI_SCREEN_HEIGHT);

    const vision_ui_list_item_t* parent = vision_ui_selector_mutable_instance_get()->selected_item->parent;

    const int16_t slider_top_px = parent ? parent->scroll_bar_top_px : 0;
    const int16_t slider_h_px = parent ? parent->scroll_bar_height_px : VISION_UI_SCREEN_HEIGHT;

    vision_ui_driver_box_draw(VISION_UI_SCREEN_WIDTH - VISION_UI_LIST_SCROLL_BAR_WIDTH, slider_top_px, VISION_UI_LIST_SCROLL_BAR_WIDTH,
                              slider_h_px);

    const uint8_t child_cnt = parent ? parent->child_num : 1;
    const float scale_part = parent ? parent->scroll_bar_scale_part : (float) VISION_UI_SCREEN_HEIGHT;
    const uint8_t mark_cnt = child_cnt > 0 ? child_cnt : 1;

    const int16_t track_x = VISION_UI_SCREEN_WIDTH - VISION_UI_LIST_SCROLL_BAR_WIDTH;
    for (uint8_t i = 0; i <= mark_cnt; ++i) {
        const int16_t y = lrintf(scale_part * i);
        if (y >= 0 && y < VISION_UI_SCREEN_HEIGHT) {
            if (i % 2 == 0) {
                vision_ui_driver_line_h_draw(track_x, y, VISION_UI_LIST_SCROLL_BAR_WIDTH);
            } else {
                vision_ui_driver_line_h_draw(track_x, y, VISION_UI_LIST_SCROLL_BAR_WIDTH - 1);
            }
        }
    }
}

static void vision_ui_text_draw(const char* text, uint32_t* text_scroll_anchor, const int16_t x0, const int16_t y0, const int16_t x1,
                                const int16_t y1, const uint16_t scroll_speed_s, const uint16_t scroll_pause_ms) {
    // 当前字体行高
    const int16_t line_h = vision_ui_driver_str_height_get();
    const int16_t text_x = x0;
    const int16_t text_y = y0 + (y1 - y0) / 2 + line_h / 2; // 垂直居中到rect内
    const int16_t visible_width = x1 - x0;

    // 文本总宽度
    const int16_t text_width = vision_ui_driver_str_utf8_width_get(text);

    const int16_t clip_x0 = x0 < 0 ? 0 : x0;
    const int16_t clip_y0 = y0 < 0 ? 0 : y0;
    const int16_t clip_x1 = x1 > VISION_UI_SCREEN_WIDTH ? VISION_UI_SCREEN_WIDTH : x1;
    const int16_t clip_y1 = y1 > VISION_UI_SCREEN_HEIGHT ? VISION_UI_SCREEN_HEIGHT : y1;
    const bool has_visible_area = clip_x0 < clip_x1 && clip_y0 < clip_y1;

    // 如果文本太长，需要滚动
    if (text_width > visible_width && visible_width > 0 && !vision_ui_is_background_frozen()) {
        int16_t scroll_offset = 0;
        const int16_t overflow = text_width - visible_width;
        const float speed_px_s = scroll_speed_s; // px/s
        const uint32_t forward_ms = (uint32_t) (1000.f * overflow / speed_px_s + 0.5f);

        const uint32_t now = vision_ui_driver_ticks_ms_get();
        if (*text_scroll_anchor == 0) {
            *text_scroll_anchor = now;
        }

        const uint32_t elapsed = now - *text_scroll_anchor;

        if (elapsed > scroll_pause_ms && forward_ms > 0) {
            // 生成往返三角波（0..overflow..0）
            const uint32_t t = (elapsed - scroll_pause_ms) % (forward_ms * 2u);
            const float p = (float) t / (float) forward_ms; // 0..2
            const float tri = (p <= 1.f) ? p : (2.f - p); // 0..1..0
            scroll_offset = (int16_t) lrintf((float) overflow * tri);
        } else {
            scroll_offset = 0;
        }

        if (has_visible_area) {
            vision_ui_driver_clip_window_set(clip_x0, clip_y0, clip_x1, clip_y1);
            vision_ui_driver_str_utf8_draw(text_x - scroll_offset, text_y, text);
            vision_ui_driver_clip_window_reset();
        }
    } else {
        // 不滚动，直接居中画
        *text_scroll_anchor = 0;
        if (has_visible_area) {
            vision_ui_driver_clip_window_set(clip_x0, clip_y0, clip_x1, clip_y1);
            vision_ui_driver_str_utf8_draw(text_x, text_y, text);
            vision_ui_driver_clip_window_reset();
        }
    }
}
static void vision_ui_text_list_item_draw(vision_ui_list_item_t* list, const int16_t x0, const int16_t y0, const int16_t x1,
                                          const int16_t y1) {
    vision_ui_text_draw(list->content, &list->text_scroll_anchor, x0, y0, x1, y1, VISION_UI_LIST_TEXT_SCROLL_SPEED_PX_S,
                        VISION_UI_LIST_TEXT_SCROLL_PAUSE_MS);
}

static void vision_ui_draw_list_header() {
    static const uint8_t header_list_item[VISION_UI_LIST_HEADER_MAX_HEIGHT] = {
            0b0000000, 0b0011110, 0b0000000, 0b0111110, 0b0000000, 0b0001110, 0b0000000,
    };

    static const uint8_t header_switch_item[VISION_UI_LIST_HEADER_MAX_HEIGHT] = {
            0b0011100, 0b0100010, 0b1001001, 0b1001001, 0b1001001, 0b0100010, 0b0011100,
    };

    static const uint8_t header_slider_item[VISION_UI_LIST_HEADER_MAX_HEIGHT] = {
            0b0001110, 0b0101110, 0b0101110, 0b0100100, 0b1110100, 0b1110100, 0b1110000,
    };

    static const uint8_t header_other_item[VISION_UI_LIST_HEADER_MAX_HEIGHT] = {
            0b0000000, 0b0000000, 0b0000000, 0b0011100, 0b0000000, 0b0000000, 0b0000000,
    };

    for (uint8_t i = 0; i < vision_ui_selector_mutable_instance_get()->selected_item->parent->child_num; i++) {
        const vision_ui_list_item_t* current_list_item = vision_ui_selector_instance_get()->selected_item->parent->child_list_item[i];
        const int16_t x_list_item = vision_ui_camera_instance_get()->x_camera + VISION_UI_LIST_HEADER_TO_LEFT_DISPLAY_PADDING;
        const int16_t y_list_item = current_list_item->y_list_item + vision_ui_camera_instance_get()->y_camera +
                                    (VISION_UI_LIST_FRAME_FIXED_HEIGHT - VISION_UI_LIST_HEADER_MAX_HEIGHT) / 2;
        // draw header
        vision_ui_driver_color_draw(1);
        const uint16_t header_base_x = x_list_item;
        if (current_list_item->type == LIST_ITEM) {
            vision_ui_driver_bmp_draw(header_base_x, y_list_item, VISION_UI_LIST_HEADER_MAX_WIDTH, VISION_UI_LIST_HEADER_MAX_HEIGHT,
                                      header_list_item);
        } else if (current_list_item->type == SWITCH_ITEM) {
            vision_ui_driver_bmp_draw(header_base_x, y_list_item, VISION_UI_LIST_HEADER_MAX_WIDTH, VISION_UI_LIST_HEADER_MAX_HEIGHT,
                                      header_switch_item);
        } else if (current_list_item->type == SLIDER_ITEM) {
            vision_ui_driver_bmp_draw(header_base_x, y_list_item, VISION_UI_LIST_HEADER_MAX_WIDTH, VISION_UI_LIST_HEADER_MAX_HEIGHT,
                                      header_slider_item);
        } else if (current_list_item->type == TITLE_ITEM) {
            // do nothing
        } else {
            vision_ui_driver_bmp_draw(header_base_x, y_list_item, VISION_UI_LIST_HEADER_MAX_WIDTH, VISION_UI_LIST_HEADER_MAX_HEIGHT,
                                      header_other_item);
        }
    }
}

static void vision_ui_draw_list_footer() {
    static const uint8_t footer_switch_off[VISION_UI_LIST_FOOTER_MAX_HEIGHT][3] = {
            {0b00000000, 0b00000000, 0b00000000}, {0b11100000, 0b00111111, 0b00000000}, {0b00100000, 0b00100000, 0b00000000},
            {0b00100000, 0b00100000, 0b00000000}, {0b00100000, 0b00100000, 0b00000000}, {0b00100000, 0b00100000, 0b00000000},
            {0b00100000, 0b00100000, 0b00000000}, {0b00100000, 0b00100000, 0b00000000}, {0b00100000, 0b00100000, 0b00000000},
            {0b11100000, 0b00111111, 0b00000000}, {0b00000000, 0b00000000, 0b00000000},
    };

    static const uint8_t footer_switch_on[VISION_UI_LIST_FOOTER_MAX_HEIGHT][3] = {
            {0b00000000, 0b00000000, 0b00000000}, {0b11100000, 0b00111111, 0b00000000}, {0b00100000, 0b00100000, 0b00000000},
            {0b10100000, 0b00101111, 0b00000000}, {0b10100000, 0b00101111, 0b00000000}, {0b10100000, 0b00101111, 0b00000000},
            {0b10100000, 0b00101111, 0b00000000}, {0b10100000, 0b00101111, 0b00000000}, {0b00100000, 0b00100000, 0b00000000},
            {0b11100000, 0b00111111, 0b00000000}, {0b00000000, 0b00000000, 0b00000000},
    };

    static const uint8_t footer_slider[VISION_UI_LIST_FOOTER_MAX_HEIGHT][3] = {
            {0b00000000, 0b00000000, 0b00000000}, {0b11111110, 0b11111111, 0b00000011}, {0b11111111, 0b11111111, 0b00000111},
            {0b11111111, 0b11111111, 0b00000111}, {0b11111111, 0b11111111, 0b00000111}, {0b11111111, 0b11111111, 0b00000111},
            {0b11111111, 0b11111111, 0b00000111}, {0b11111111, 0b11111111, 0b00000111}, {0b11111111, 0b11111111, 0b00000111},
            {0b11111111, 0b11111111, 0b00000111}, {0b11111110, 0b11111111, 0b00000011},
    };

    for (uint8_t i = 0; i < vision_ui_selector_instance_get()->selected_item->parent->child_num; i++) {
        vision_ui_list_item_t* current_list_item = vision_ui_selector_instance_get()->selected_item->parent->child_list_item[i];
        const int16_t y_list_item = current_list_item->y_list_item + vision_ui_camera_instance_get()->y_camera;

        // draw header
        const int16_t frame_x = VISION_UI_SCREEN_WIDTH - VISION_UI_LIST_SCROLL_BAR_WIDTH - VISION_UI_LIST_FOOTER_TO_SCROLL_BAR_PADDING -
                                VISION_UI_LIST_FOOTER_MAX_WIDTH;
        const int16_t frame_y = y_list_item + (VISION_UI_LIST_FRAME_FIXED_HEIGHT - VISION_UI_LIST_FOOTER_MAX_HEIGHT) / 2;
        if (current_list_item->type == LIST_ITEM) {
        } else if (current_list_item->type == SWITCH_ITEM) {
            if (vision_ui_to_list_switch_item(current_list_item)->value == true) {
                vision_ui_driver_color_draw(1);
                vision_ui_driver_bmp_draw(frame_x, frame_y, VISION_UI_LIST_FOOTER_MAX_WIDTH, VISION_UI_LIST_FOOTER_MAX_HEIGHT,
                                          (uint8_t*) footer_switch_on);
            } else {
                vision_ui_driver_color_draw(1);
                vision_ui_driver_bmp_draw(frame_x, frame_y, VISION_UI_LIST_FOOTER_MAX_WIDTH, VISION_UI_LIST_FOOTER_MAX_HEIGHT,
                                          (uint8_t*) footer_switch_off);
            }
        } else if (current_list_item->type == SLIDER_ITEM) {
            const uint16_t shrink_width = VISION_UI_LIST_FOOTER_MAX_WIDTH - 4;
            const int16_t footer_x0 = frame_x + (VISION_UI_LIST_FOOTER_MAX_WIDTH - shrink_width) / 2;
            const int16_t footer_y0 = frame_y;
            const int16_t footer_x1 = footer_x0 + shrink_width;
            const int16_t footer_y1 = footer_y0 + VISION_UI_LIST_FOOTER_MAX_HEIGHT;

            char value_str[10] = {};
            sprintf(value_str, "%d", vision_ui_to_list_slider_item(current_list_item)->value);

            vision_ui_driver_color_draw(1);
            vision_ui_text_draw(value_str, &vision_ui_to_list_slider_item(current_list_item)->text_scroll_anchor, footer_x0, footer_y0,
                                footer_x1, footer_y1, VISION_UI_LIST_SLIDER_VALUE_SCROLL_SPEED_PX_S,
                                VISION_UI_LIST_SLIDER_VALUE_SCROLL_PAUSE_MS);
            if (vision_ui_to_list_slider_item(current_list_item)->is_confirmed) {
                vision_ui_driver_color_draw(2);
                vision_ui_driver_bmp_draw(frame_x, frame_y, VISION_UI_LIST_FOOTER_MAX_WIDTH, VISION_UI_LIST_FOOTER_MAX_HEIGHT,
                                          (uint8_t*) footer_slider);
            }
        }
    }
}

static void vision_ui_list_item_render() {
    vision_ui_draw_list_header();
    vision_ui_draw_list_footer();

    for (uint8_t i = 0; i < vision_ui_selector_instance_get()->selected_item->parent->child_num; i++) {
        vision_ui_list_item_t* current_list_item = vision_ui_selector_instance_get()->selected_item->parent->child_list_item[i];
        const int16_t x_list_item = vision_ui_camera_instance_get()->x_camera + VISION_UI_LIST_HEADER_TO_LEFT_DISPLAY_PADDING;
        const int16_t y_list_item = current_list_item->y_list_item + vision_ui_camera_instance_get()->y_camera;

        const int16_t frame_x = current_list_item->type == TITLE_ITEM
                                        ? x_list_item
                                        : x_list_item + VISION_UI_LIST_HEADER_MAX_WIDTH + VISION_UI_LIST_HEADER_TO_TEXT_PADDING;

        const int16_t frame_y = y_list_item;

        vision_ui_font_set(vision_ui_font_get());
        vision_ui_driver_color_draw(1);
        vision_ui_text_list_item_draw(current_list_item, frame_x, frame_y, frame_x + VISION_UI_LIST_TEXT_MAX_WIDTH,
                                      y_list_item + VISION_UI_LIST_FRAME_FIXED_HEIGHT);
    }
}

static void vision_ui_icon_view_render() {
    const vision_ui_selector_t* selector = vision_ui_selector_instance_get();
    if (selector->selected_item == NULL || selector->selected_item->parent == NULL) {
        return;
    }

    vision_ui_list_item_t* parent = selector->selected_item->parent;
    const float scroll_offset = parent->icon_scroll_offset;
    const float item_span = (float) (VISION_UI_ICON_VIEW_ICON_SIZE + VISION_UI_ICON_VIEW_ITEM_SPACING);
    const int16_t center_x = VISION_UI_SCREEN_WIDTH / 2;

    for (uint8_t i = 0; i < parent->child_num; ++i) {
        vision_ui_list_item_t* child = parent->child_list_item[i];
        if (child->type != ICON_ITEM) {
            continue;
        }

        const float icon_center = (float) center_x + scroll_offset + (float) i * item_span;
        const int16_t icon_x = (int16_t) lrintf(icon_center) - VISION_UI_ICON_VIEW_ICON_SIZE / 2;
        if (icon_x + VISION_UI_ICON_VIEW_ICON_SIZE < 0 || icon_x > VISION_UI_SCREEN_WIDTH) {
            continue;
        }

        const vision_ui_icon_item_t* icon_item = vision_ui_to_list_icon_item(child);
        vision_ui_driver_color_draw(1);
        if (icon_item->icon != NULL) {
            vision_ui_driver_bmp_draw(icon_x, VISION_UI_ICON_VIEW_ICON_TO_TOP_DISPLAY_PADDING, VISION_UI_ICON_VIEW_ICON_SIZE,
                                      VISION_UI_ICON_VIEW_ICON_SIZE, icon_item->icon);
        } else {
            vision_ui_driver_frame_draw(icon_x, VISION_UI_ICON_VIEW_ICON_TO_TOP_DISPLAY_PADDING, VISION_UI_ICON_VIEW_ICON_SIZE,
                                        VISION_UI_ICON_VIEW_ICON_SIZE);
            vision_ui_driver_line_h_dotted_draw(icon_x, VISION_UI_ICON_VIEW_ICON_TO_TOP_DISPLAY_PADDING + VISION_UI_ICON_VIEW_ICON_SIZE / 2,
                                                VISION_UI_ICON_VIEW_ICON_SIZE);
        }
    }

    if (selector->selected_item->type == ICON_ITEM) {
        const int16_t title_area_y0 = VISION_UI_ICON_VIEW_ICON_TO_TOP_DISPLAY_PADDING + VISION_UI_ICON_VIEW_ICON_SIZE +
                                      VISION_UI_ICON_VIEW_ICON_TO_TITLE_AREA_PADDING;
        const int16_t title_bar_x0 = VISION_UI_ICON_VIEW_TITLE_BAR_TO_LEFT_DISPLAY_PADDING;
        const int16_t title_x0 = title_bar_x0 + VISION_UI_ICON_VIEW_TITLE_BAR_WIDTH + VISION_UI_ICON_VIEW_TITLE_BAR_TO_TITLE_PADDING;
        const int16_t title_x1 = VISION_UI_SCREEN_WIDTH - VISION_UI_ICON_VIEW_TITLE_TO_RIGHT_DISPLAY_MIN_PADDING;
        const int16_t title_y0 = title_area_y0;
        const int16_t title_y1 = title_y0 + VISION_UI_ICON_VIEW_TITLE_AREA_HEIGHT;

        vision_ui_driver_color_draw(1);
        vision_ui_driver_box_draw(title_bar_x0, title_area_y0, VISION_UI_ICON_VIEW_TITLE_BAR_WIDTH, VISION_UI_ICON_VIEW_TITLE_AREA_HEIGHT);

        const vision_ui_icon_item_t* selected_icon = vision_ui_to_list_icon_item(selector->selected_item);
        const float title_offset = selected_icon->title_y;
        const int16_t title_offset_px = (int16_t) lrintf(title_offset);

        vision_ui_text_draw(selector->selected_item->content, &selector->selected_item->text_scroll_anchor, title_x0,
                            title_y0 + title_offset_px, title_x1, title_y1, VISION_UI_LIST_TEXT_SCROLL_SPEED_PX_S,
                            VISION_UI_LIST_TEXT_SCROLL_PAUSE_MS);

        if (selected_icon->description != NULL) {
            uint16_t spacing;
            if (vision_ui_driver_str_width_get(selected_icon->description) >
                VISION_UI_SCREEN_WIDTH - 2 * VISION_UI_ICON_VIEW_DESCRIPTION_TO_DISPLAY_MIN_SPACING) {
                spacing = VISION_UI_ICON_VIEW_DESCRIPTION_TO_DISPLAY_MIN_SPACING;
            } else {
                spacing = (VISION_UI_SCREEN_WIDTH - vision_ui_driver_str_width_get(selected_icon->description)) / 2;
            }
            const uint16_t description_x0 = spacing;
            const uint16_t description_x1 = VISION_UI_SCREEN_WIDTH - spacing;
            const uint16_t description_y0 =
                    title_area_y0 + VISION_UI_ICON_VIEW_TITLE_AREA_HEIGHT + VISION_UI_ICON_VIEW_TITLE_AREA_TO_DESCRIPTION_PADDING;
            const uint16_t description_y1 = description_y0 + vision_ui_driver_str_height_get();
            vision_ui_driver_color_draw(1);
            vision_ui_text_draw(selected_icon->description,
                                &vision_ui_to_list_icon_item(selector->selected_item)->description_scroll_anchor, description_x0,
                                description_y0, description_x1, description_y1, VISION_UI_LIST_TEXT_SCROLL_SPEED_PX_S,
                                VISION_UI_LIST_TEXT_SCROLL_PAUSE_MS);
        }
    }
}

static void vision_ui_selector_render() {
    const int16_t x_selector = (int16_t) (lrintf(vision_ui_camera_instance_get()->x_camera) +
                                          VISION_UI_LIST_HEADER_TO_LEFT_DISPLAY_PADDING - VISION_UI_LIST_SELECTOR_TO_INNER_WIDGET_PADDING);
    const int16_t y_selector = (int16_t) lrintf(vision_ui_selector_instance_get()->y_selector + vision_ui_camera_instance_get()->y_camera);

    vision_ui_driver_color_draw(1);
    const int16_t selector_w = (int16_t) lrintf(vision_ui_selector_instance_get()->w_selector);
    const int16_t selector_h = (int16_t) lrintf(vision_ui_selector_instance_get()->h_selector);

    vision_ui_driver_frame_r_draw(x_selector, y_selector, selector_w, selector_h, 3);
    vision_ui_driver_color_draw(2);
    vision_ui_driver_box_draw(x_selector + 1, y_selector + 2, selector_w - 2, selector_h - 4);
    vision_ui_driver_line_h_draw(x_selector + 2, y_selector + 1, selector_w - 4);
    vision_ui_driver_line_h_draw(x_selector + 2, y_selector + selector_h - 2, selector_w - 4);
}

void vision_ui_widget_render() {
    vision_ui_notification_render();

    // everything else should be blured except the alert object
    if (vision_ui_is_background_frozen()) {
        vision_ui_background_blur_animation_render(0, 0, VISION_UI_SCREEN_WIDTH, VISION_UI_SCREEN_HEIGHT, 4);
    }
    vision_ui_alert_render();
}

void vision_ui_list_render() {
    if (vision_ui_icon_view_is_active()) {
        vision_ui_icon_view_render();
        return;
    }

    // 调用所有的列表相关draw函数
    vision_ui_list_appearance_render();
    vision_ui_list_item_render();
    vision_ui_selector_render();
}
