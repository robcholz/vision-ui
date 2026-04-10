//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//
#include "vision_ui_renderer.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vision_ui_animation.h"
#include "vision_ui_config.h"
#include "vision_ui_core.h"
#include "vision_ui_draw_driver.h"
#include "vision_ui_item.h"

static const uint8_t HEADER_LIST_ITEM[7] = {
        0b0000000,
        0b0011110,
        0b0000000,
        0b0111110,
        0b0000000,
        0b0001110,
        0b0000000,
};

static const uint8_t HEADER_SWITCH_ITEM[7] = {
        0b0011100,
        0b0100010,
        0b1001001,
        0b1001001,
        0b1001001,
        0b0100010,
        0b0011100,
};

static const uint8_t HEADER_SLIDER_ITEM[7] = {
        0b0001110,
        0b0101110,
        0b0101110,
        0b0100100,
        0b1110100,
        0b1110100,
        0b1110000,
};

static const uint8_t HEADER_OTHER_ITEM[7] = {
        0b0000000,
        0b0000000,
        0b0000000,
        0b0011100,
        0b0000000,
        0b0000000,
        0b0000000,
};

static const uint8_t FOOTER_SWITCH_OFF[19][3] = {
        {0b00000000, 0b00000000, 0b00000000},
        {0b11100000, 0b00111111, 0b00000000},
        {0b00100000, 0b00100000, 0b00000000},
        {0b00100000, 0b00100000, 0b00000000},
        {0b00100000, 0b00100000, 0b00000000},
        {0b00100000, 0b00100000, 0b00000000},
        {0b00100000, 0b00100000, 0b00000000},
        {0b00100000, 0b00100000, 0b00000000},
        {0b00100000, 0b00100000, 0b00000000},
        {0b11100000, 0b00111111, 0b00000000},
        {0b00000000, 0b00000000, 0b00000000},
};

static const uint8_t FOOTER_SWITCH_ON[19][3] = {
        {0b00000000, 0b00000000, 0b00000000},
        {0b11100000, 0b00111111, 0b00000000},
        {0b00100000, 0b00100000, 0b00000000},
        {0b10100000, 0b00101111, 0b00000000},
        {0b10100000, 0b00101111, 0b00000000},
        {0b10100000, 0b00101111, 0b00000000},
        {0b10100000, 0b00101111, 0b00000000},
        {0b10100000, 0b00101111, 0b00000000},
        {0b00100000, 0b00100000, 0b00000000},
        {0b11100000, 0b00111111, 0b00000000},
        {0b00000000, 0b00000000, 0b00000000},
};

static const uint8_t FOOTER_SLIDER[19][3] = {
        {0b00000000, 0b00000000, 0b00000000},
        {0b11111110, 0b11111111, 0b00000011},
        {0b11111111, 0b11111111, 0b00000111},
        {0b11111111, 0b11111111, 0b00000111},
        {0b11111111, 0b11111111, 0b00000111},
        {0b11111111, 0b11111111, 0b00000111},
        {0b11111111, 0b11111111, 0b00000111},
        {0b11111111, 0b11111111, 0b00000111},
        {0b11111111, 0b11111111, 0b00000111},
        {0b11111111, 0b11111111, 0b00000111},
        {0b11111110, 0b11111111, 0b00000011},
};

vision_ui_icon_t DEFAULT_LIST_ICON = {
        .list_header = HEADER_LIST_ITEM,
        .switch_header = HEADER_SWITCH_ITEM,
        .slider_header = HEADER_SLIDER_ITEM,
        .default_header = HEADER_OTHER_ITEM,
        .switch_on_footer = (const uint8_t*) FOOTER_SWITCH_ON,
        .switch_off_footer = (const uint8_t*) FOOTER_SWITCH_OFF,
        .slider_footer = (const uint8_t*) FOOTER_SLIDER,

        .header_width = 7,
        .header_height = 7,

        .footer_width = 20,
        .footer_height = 19,
};

extern void vision_ui_list_icon_set(vision_ui_t* ui, const vision_ui_icon_t icon) {
    assert(ui != NULL);
    ui->list_icon = icon;
}

vision_ui_icon_t vision_ui_list_icon_get_current(const vision_ui_t* ui) {
    assert(ui != NULL);
    return ui->list_icon;
}

const vision_ui_notification_t* vision_ui_notification_instance_get(const vision_ui_t* ui) {
    assert(ui != NULL);
    return &ui->notification;
}

vision_ui_notification_t* vision_ui_notification_mutable_instance_get(vision_ui_t* ui) {
    assert(ui != NULL);
    return &ui->notification;
}

void vision_ui_notification_push(vision_ui_t* ui, const char* content, const uint16_t span) {
    assert(ui != NULL);

    vision_ui_notification_t* notification = &ui->notification;
    const uint32_t now = vision_ui_driver_ticks_ms_get(ui);

    if (!notification->is_running) {
        notification->content = content;
        notification->span = span;
        notification->time_start = now;
        notification->time = now;
        notification->y_notification_trg = 0;
        notification->is_running = true;
        notification->is_dismissing = false;
        notification->has_pending = false;
        notification->pending_content = NULL;
        notification->pending_span = 0;
        notification->dismiss_start = 0;
        vision_ui_font_set(ui, vision_ui_font_get(ui));
        notification->w_notification_trg =
                vision_ui_driver_str_utf8_width_get(ui, notification->content) + VISION_UI_NOTIFICATION_WIDTH;
        return;
    }

    notification->pending_content = content;
    notification->pending_span = span;
    notification->has_pending = true;
    notification->is_dismissing = true;
    notification->dismiss_start = now;
    notification->y_notification_trg = 0 - 2 * VISION_UI_NOTIFICATION_HEIGHT;
}

const vision_ui_alert_t* vision_ui_alert_instance_get(const vision_ui_t* ui) {
    assert(ui != NULL);
    return &ui->alert;
}

vision_ui_alert_t* vision_ui_alert_mutable_instance_get(vision_ui_t* ui) {
    assert(ui != NULL);
    return &ui->alert;
}

void vision_ui_alert_push(vision_ui_t* ui, const char* content, const uint16_t span) {
    assert(ui != NULL);

    vision_ui_alert_t* alert = &ui->alert;
    alert->time = vision_ui_driver_ticks_ms_get(ui);
    alert->content = content;
    alert->span = span;
    alert->is_running = false;

    if (!alert->is_running) {
        alert->time_start = vision_ui_driver_ticks_ms_get(ui);
        alert->y_alert_trg = (VISION_UI_SCREEN_HEIGHT - VISION_UI_ALERT_HEIGHT) / 2;
        alert->is_running = true;
    }

    vision_ui_font_set(ui, vision_ui_font_get(ui));
    alert->w_alert_trg = vision_ui_driver_str_utf8_width_get(ui, alert->content) + VISION_UI_ALERT_WIDTH;
}

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

static void vision_ui_background_blur_animation_render(
        const vision_ui_t* ui,
        const uint16_t x0,
        const uint16_t y0,
        uint16_t width,
        uint16_t height,
        const uint8_t fade_level
) {
    if (x0 + width > VISION_UI_SCREEN_WIDTH) {
        width = VISION_UI_SCREEN_WIDTH - x0;
    }
    if (y0 + height > VISION_UI_SCREEN_HEIGHT) {
        height = VISION_UI_SCREEN_HEIGHT - y0;
    }

    if (fade_level < 1 || fade_level > 5) {
        return;
    }

    const uint8_t* buffer_live = vision_ui_driver_buffer_pointer_get(ui);
    if (buffer_live == NULL) {
        return;
    }

    // Define 2x2 grid fade patterns.
    // Each entry marks which pixels in the 2x2 grid should be turned off.
    // 0 keeps the pixel lit, 1 turns it off.
    static const uint8_t patterns[5][2][2] = {
            {{0, 0}, // Level 1: fully lit
             {0, 0}},
            {{1, 0}, // Level 2: top-left off
             {0, 0}},
            {{1, 0}, // Level 3: top-left and bottom-right off
             {0, 1}},
            {{1, 0}, // Level 4: keep only the top-right pixel
             {1, 1}},
            {{1, 1}, // Level 5: fully dark
             {1, 1}}
    };

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
            vision_ui_driver_color_draw(ui, 0);
            for (uint16_t local_x = 0; local_x < row_bits; ++local_x) {
                if (row_mask_prev[local_x >> 3] & (uint8_t) (1u << (local_x & 0x7))) {
                    vision_ui_driver_pixel_draw(ui, x0 + local_x, prev_row_y);
                }
            }
        } else {
            has_prev_row = true;
        }

        memcpy(row_mask_prev, row_mask_curr, row_mask_bytes);
        prev_row_y = y;
    }

    if (has_prev_row) {
        vision_ui_driver_color_draw(ui, 0);
        for (uint16_t local_x = 0; local_x < row_bits; ++local_x) {
            if (row_mask_prev[local_x >> 3] & (uint8_t) (1u << (local_x & 0x7))) {
                vision_ui_driver_pixel_draw(ui, x0 + local_x, prev_row_y);
            }
        }
    }

    vision_ui_driver_color_draw(ui, 1);
}

static bool vision_ui_icon_view_context(
        const vision_ui_selector_t* selector,
        vision_ui_list_item_t** icon_list_out,
        vision_ui_list_item_t** selected_icon_out
) {
    vision_ui_list_item_t* icon_list = NULL;
    vision_ui_list_item_t* selected_icon = NULL;

    if (selector != NULL && selector->selected_item != NULL) {
        vision_ui_list_item_t* item = selector->selected_item;
        if (item->parent != NULL && item->parent->icon_view_mode) {
            icon_list = item->parent;
            if (item->type == IconItem) {
                selected_icon = item;
            }
        } else if (item->parent != NULL && item->parent->type == IconItem && item->parent->child_num == 1 &&
                   item->parent->parent != NULL && item->parent->parent->icon_view_mode &&
                   item->parent->child_list_item[0]->type == UserItem) {
            icon_list = item->parent->parent;
            selected_icon = item->parent;
        }
    }

    if (icon_list_out != NULL) {
        *icon_list_out = icon_list;
    }
    if (selected_icon_out != NULL) {
        *selected_icon_out = selected_icon;
    }

    return icon_list != NULL;
}

static bool vision_ui_icon_view_is_active(const vision_ui_t* ui) {
    return vision_ui_icon_view_context(vision_ui_selector_instance_get(ui), NULL, NULL);
}

static const uint8_t VISION_UI_EXIT_ANIMATION_LEVELS = 5;
static const uint8_t VISION_UI_ENTER_ANIMATION_LEVELS = 4;

void vision_ui_exit_animation_render(vision_ui_t* ui, const float delta_ms) {
    assert(ui != NULL);
    if (vision_ui_exit_animation_is_finished(ui)) {
        return;
    }

    if (!ui->running_exit) {
        ui->exit_elapsed_ms = 0.0f;
        ui->running_exit = true;
    }

    ui->exit_elapsed_ms += delta_ms;
    const float progress = fminf(ui->exit_elapsed_ms / VISION_UI_EXIT_ANIMATION_DURATION_MS, 1.0f);

    // Apply ease-in-out first, then map to discrete levels.
    const float eased = vision_ui_smoothstep(progress); // 0..1
    const float scaled = eased * (float) (VISION_UI_EXIT_ANIMATION_LEVELS - 1);
    const uint8_t fade_level = 1 + (uint8_t) floorf(scaled + 0.5f); // +0.5f gives rounding closer to nearest.
    vision_ui_background_blur_animation_render(ui, 0, 0, VISION_UI_SCREEN_WIDTH, VISION_UI_SCREEN_HEIGHT, fade_level);

    if (progress >= 1.0f) {
        ui->running_exit = false;
        vision_ui_exit_animation_set_is_finished(ui);
        vision_ui_enter_animation_start(ui);
    }
}

void vision_ui_enter_animation_render(vision_ui_t* ui, const float delta_ms) {
    assert(ui != NULL);
    if (vision_ui_enter_animation_is_finished(ui)) {
        return;
    }

    if (!ui->running_enter) {
        ui->enter_elapsed_ms = 0.0f;
        ui->running_enter = true;
    }

    ui->enter_elapsed_ms += delta_ms;
    const float progress = fminf(ui->enter_elapsed_ms / VISION_UI_ENTER_ANIMATION_DURATION_MS, 1.0f);
    const float inverted = 1.0f - progress;

    // Apply smoothstep to the inverted progress first.
    const float eased = vision_ui_smoothstep(inverted);
    const float scaled = eased * (float) (VISION_UI_ENTER_ANIMATION_LEVELS - 1);
    const uint8_t fade_level = 1 + (uint8_t) floorf(scaled + 0.5f);
    vision_ui_background_blur_animation_render(ui, 0, 0, VISION_UI_SCREEN_WIDTH, VISION_UI_SCREEN_HEIGHT, fade_level);

    if (progress >= 1.0f) {
        ui->running_enter = false;
        vision_ui_enter_animation_set_is_finished(ui);
    }
}

static void vision_ui_notification_render(vision_ui_t* ui) {
    assert(ui != NULL);

    const vision_ui_notification_t* notification = vision_ui_notification_instance_get(ui);
    const vision_ui_notification_t* notification_mut = vision_ui_notification_mutable_instance_get(ui);
    if (notification == NULL || notification_mut == NULL || !notification->is_running) {
        return;
    }

    const uint32_t now = vision_ui_driver_ticks_ms_get(ui);

    if (vision_ui_notification_instance_get(ui)->is_dismissing) {
        const bool reached_target = vision_ui_notification_instance_get(ui)->y_notification ==
                                    vision_ui_notification_instance_get(ui)->y_notification_trg;
        const bool removal_time_elapsed = now - vision_ui_notification_instance_get(ui)->dismiss_start >=
                                          VISION_UI_NOTIFICATION_DISMISS_DURATION_MS;

        if (removal_time_elapsed || reached_target) {
            if (!reached_target) {
                vision_ui_notification_mutable_instance_get(ui)->y_notification =
                        vision_ui_notification_instance_get(ui)->y_notification_trg;
            }

            if (vision_ui_notification_instance_get(ui)->has_pending) {
                vision_ui_notification_mutable_instance_get(ui)->is_dismissing = false;
                vision_ui_notification_mutable_instance_get(ui)->content =
                        vision_ui_notification_instance_get(ui)->pending_content;
                vision_ui_notification_mutable_instance_get(ui)->span =
                        vision_ui_notification_instance_get(ui)->pending_span;
                vision_ui_notification_mutable_instance_get(ui)->pending_content = NULL;
                vision_ui_notification_mutable_instance_get(ui)->pending_span = 0;
                vision_ui_notification_mutable_instance_get(ui)->has_pending = false;
                vision_ui_notification_mutable_instance_get(ui)->time_start = now;
                vision_ui_notification_mutable_instance_get(ui)->time = now;
                vision_ui_notification_mutable_instance_get(ui)->y_notification_trg = 0;
                vision_ui_notification_mutable_instance_get(ui)->dismiss_start = 0;
                vision_ui_font_set(ui, vision_ui_font_get(ui));
                vision_ui_notification_mutable_instance_get(ui)->w_notification_trg =
                        vision_ui_driver_str_utf8_width_get(ui, vision_ui_notification_instance_get(ui)->content) +
                        VISION_UI_NOTIFICATION_WIDTH;
                vision_ui_notification_mutable_instance_get(ui)->is_running = true;
            } else {
                vision_ui_notification_mutable_instance_get(ui)->is_running = false;
                vision_ui_notification_mutable_instance_get(ui)->is_dismissing = false;
                vision_ui_notification_mutable_instance_get(ui)->dismiss_start = 0;
                vision_ui_notification_mutable_instance_get(ui)->has_pending = false;
                vision_ui_notification_mutable_instance_get(ui)->pending_content = NULL;
                vision_ui_notification_mutable_instance_get(ui)->pending_span = 0;
                return;
            }
        }
    } else {
        if (vision_ui_notification_instance_get(ui)->y_notification ==
            vision_ui_notification_instance_get(ui)->y_notification_trg) {
            vision_ui_notification_mutable_instance_get(ui)->time = now;
        }

        if (vision_ui_notification_instance_get(ui)->time - vision_ui_notification_instance_get(ui)->time_start >=
            vision_ui_notification_instance_get(ui)->span) {
            vision_ui_notification_mutable_instance_get(ui)->y_notification_trg = 0 - 2 * VISION_UI_NOTIFICATION_HEIGHT;
            vision_ui_notification_mutable_instance_get(ui)->is_dismissing = true;
            vision_ui_notification_mutable_instance_get(ui)->dismiss_start = now;
        }
    }

    if (!vision_ui_notification_instance_get(ui)->is_running) {
        return;
    }

    const int16_t x_notification =
            VISION_UI_SCREEN_WIDTH / 2 - vision_ui_notification_instance_get(ui)->w_notification / 2;
    const int16_t y_notification_1 = vision_ui_notification_instance_get(ui)->y_notification - 4;
    const int16_t y_notification_2 =
            vision_ui_notification_instance_get(ui)->y_notification + VISION_UI_NOTIFICATION_HEIGHT;

    vision_ui_font_set(ui, vision_ui_font_get(ui));

    vision_ui_driver_color_draw(ui, 0); // Black underlay.
    vision_ui_driver_box_r_draw(
            ui,
            (int16_t) (VISION_UI_SCREEN_WIDTH / 2 - (vision_ui_notification_instance_get(ui)->w_notification + 4) / 2),
            y_notification_1,
            (int16_t) (vision_ui_notification_instance_get(ui)->w_notification + 4),
            VISION_UI_NOTIFICATION_HEIGHT + 6,
            4
    );

    vision_ui_driver_color_draw(ui, 1);
    vision_ui_driver_box_r_draw(
            ui,
            x_notification,
            y_notification_1,
            (int16_t) vision_ui_notification_instance_get(ui)->w_notification,
            VISION_UI_NOTIFICATION_HEIGHT + 4,
            3
    );
    // Shift upward by four pixels and extend downward by four pixels to keep only the lower rounded corners.

    vision_ui_driver_color_draw(ui, 2);
    vision_ui_driver_line_h_draw(
            ui,
            x_notification + 2,
            y_notification_2 - 2,
            (int16_t) (vision_ui_notification_instance_get(ui)->w_notification - 4)
    );
    vision_ui_driver_pixel_draw(ui, x_notification + 1, y_notification_2 - 3);
    vision_ui_driver_pixel_draw(ui, x_notification + 1, y_notification_2 - 3);

    const int16_t text_w = vision_ui_driver_str_utf8_width_get(ui, vision_ui_notification_instance_get(ui)->content);
    const int16_t text_h = vision_ui_driver_str_height_get(ui);
    const int16_t text_x =
            x_notification + (int16_t) ((vision_ui_notification_instance_get(ui)->w_notification - text_w) / 2);
    const int16_t text_y = (int16_t) (vision_ui_notification_instance_get(ui)->y_notification +
                                      vision_ui_driver_str_height_get(ui) - 2);

    vision_ui_driver_color_draw(ui, 0);
    vision_ui_driver_box_draw(ui, text_x, text_y - text_h, text_w, text_h);

    vision_ui_driver_color_draw(ui, 1);
    vision_ui_driver_str_utf8_draw(ui, text_x, text_y, vision_ui_notification_instance_get(ui)->content);

    vision_ui_driver_color_draw(ui, 2);
    vision_ui_driver_box_draw(ui, text_x, text_y - text_h, text_w, text_h);
}

static void vision_ui_alert_render(vision_ui_t* ui) {
    assert(ui != NULL);
    const vision_ui_alert_t* alert = vision_ui_alert_instance_get(ui);
    const vision_ui_alert_t* alert_mut = vision_ui_alert_mutable_instance_get(ui);
    if (alert == NULL || alert_mut == NULL) {
        return;
    }
    if (!alert->is_running) {
        return;
    }

    // Start timing only after the alert reaches its target position.
    if (vision_ui_alert_instance_get(ui)->y_alert == vision_ui_alert_instance_get(ui)->y_alert_trg) {
        vision_ui_alert_mutable_instance_get(ui)->time = vision_ui_driver_ticks_ms_get(ui);
    }

    // Retract the alert after its display time expires.
    if (vision_ui_alert_instance_get(ui)->time - vision_ui_alert_instance_get(ui)->time_start >=
        vision_ui_alert_instance_get(ui)->span) {
        vision_ui_alert_mutable_instance_get(ui)->y_alert_trg =
                0 - 2 * VISION_UI_ALERT_HEIGHT; // Move it back off-screen.
        if (vision_ui_alert_instance_get(ui)->y_alert == vision_ui_alert_instance_get(ui)->y_alert_trg) {
            vision_ui_alert_mutable_instance_get(ui)->is_running = false; // End the lifecycle once it returns home.
        }
    }

    const int16_t x_alert = VISION_UI_SCREEN_WIDTH / 2 - vision_ui_alert_instance_get(ui)->w_alert / 2;
    const int16_t y_alert = vision_ui_alert_instance_get(ui)->y_alert + VISION_UI_ALERT_HEIGHT;

    vision_ui_font_set(ui, vision_ui_font_get(ui));

    vision_ui_driver_color_draw(ui, 0); // Black underlay.
    vision_ui_driver_box_r_draw(
            ui,
            (int16_t) (VISION_UI_SCREEN_WIDTH / 2 - (vision_ui_alert_instance_get(ui)->w_alert + 4) / 2 - 2),
            (int16_t) (vision_ui_alert_instance_get(ui)->y_alert - 2),
            (int16_t) (vision_ui_alert_instance_get(ui)->w_alert + 8),
            VISION_UI_ALERT_HEIGHT + 4,
            5
    );

    vision_ui_driver_color_draw(ui, 1);
    vision_ui_driver_box_r_draw(
            ui,
            x_alert - 2,
            (int16_t) vision_ui_alert_instance_get(ui)->y_alert,
            (int16_t) (vision_ui_alert_instance_get(ui)->w_alert + 4),
            VISION_UI_ALERT_HEIGHT,
            3
    );

    vision_ui_driver_color_draw(ui, 2);
    vision_ui_driver_line_h_draw(ui, x_alert, y_alert - 2, (int16_t) vision_ui_alert_instance_get(ui)->w_alert);
    vision_ui_driver_pixel_draw(ui, x_alert - 1, y_alert - 3);
    vision_ui_driver_pixel_draw(
            ui, (int16_t) (VISION_UI_SCREEN_WIDTH / 2 + vision_ui_alert_instance_get(ui)->w_alert / 2), y_alert - 3
    );

    const int16_t text_w = vision_ui_driver_str_utf8_width_get(ui, vision_ui_alert_instance_get(ui)->content);
    const int16_t text_h = vision_ui_driver_str_height_get(ui);
    const int16_t text_x = x_alert + (int16_t) ((vision_ui_alert_instance_get(ui)->w_alert - text_w) / 2);
    const int16_t text_y =
            (int16_t) (vision_ui_alert_instance_get(ui)->y_alert + vision_ui_driver_str_height_get(ui) + 1);

    vision_ui_driver_color_draw(ui, 0);
    vision_ui_driver_box_draw(ui, text_x, text_y - text_h, text_w, text_h);

    vision_ui_driver_color_draw(ui, 1);
    vision_ui_driver_str_utf8_draw(ui, text_x, text_y, vision_ui_alert_instance_get(ui)->content);

    vision_ui_driver_color_draw(ui, 2);
    vision_ui_driver_box_draw(ui, text_x, text_y - text_h, text_w, text_h);
}

static void vision_ui_list_appearance_render(vision_ui_t* ui) {
    vision_ui_driver_color_draw(ui, 1);

    vision_ui_driver_line_v_draw(ui, VISION_UI_SCREEN_WIDTH - 2, 0, VISION_UI_SCREEN_HEIGHT);

    const vision_ui_list_item_t* parent = vision_ui_selector_mutable_instance_get(ui)->selected_item->parent;

    const int16_t slider_top_px = parent ? parent->scroll_bar_top_px : 0;
    const int16_t slider_h_px = parent ? parent->scroll_bar_height_px : VISION_UI_SCREEN_HEIGHT;

    vision_ui_driver_box_draw(
            ui,
            VISION_UI_SCREEN_WIDTH - VISION_UI_LIST_SCROLL_BAR_WIDTH,
            slider_top_px,
            VISION_UI_LIST_SCROLL_BAR_WIDTH,
            slider_h_px
    );

    const uint8_t child_cnt = parent ? parent->child_num : 1;
    const float scale_part = parent ? parent->scroll_bar_scale_part : (float) VISION_UI_SCREEN_HEIGHT;
    const uint8_t mark_cnt = child_cnt > 0 ? child_cnt : 1;

    const int16_t track_x = VISION_UI_SCREEN_WIDTH - VISION_UI_LIST_SCROLL_BAR_WIDTH;
    for (uint8_t i = 0; i <= mark_cnt; ++i) {
        const int16_t y = lrintf(scale_part * i);
        if (y >= 0 && y < VISION_UI_SCREEN_HEIGHT) {
            if (i % 2 == 0) {
                vision_ui_driver_line_h_draw(ui, track_x, y, VISION_UI_LIST_SCROLL_BAR_WIDTH);
            } else {
                vision_ui_driver_line_h_draw(ui, track_x, y, VISION_UI_LIST_SCROLL_BAR_WIDTH - 1);
            }
        }
    }
}

static void vision_ui_text_draw(
        const vision_ui_t* ui,
        const char* text,
        uint32_t* text_scroll_anchor,
        const int16_t x0,
        const int16_t y0,
        const int16_t x1,
        const int16_t y1,
        const uint16_t scroll_speed_s,
        const uint16_t scroll_pause_ms
) {
    static const uint8_t clip_edge_width = 1;

    const int16_t line_h = vision_ui_driver_str_height_get(ui);
    const int16_t text_x = x0;
    const int16_t text_y = y0 + (y1 - y0) / 2 + line_h / 2 + clip_edge_width; // Vertically center within the rect.
    const int16_t visible_width = x1 - x0;

    // Total text width.
    const int16_t text_width = vision_ui_driver_str_utf8_width_get(ui, text);

    const int16_t clip_x0 = x0 < 0 ? 0 : x0;
    const int16_t clip_y0 = y0 < 0 ? 0 : y0;
    const int16_t clip_x1 = x1 > VISION_UI_SCREEN_WIDTH ? VISION_UI_SCREEN_WIDTH : x1;
    const int16_t clip_y1 = y1 > VISION_UI_SCREEN_HEIGHT ? VISION_UI_SCREEN_HEIGHT : y1;
    const bool has_visible_area = clip_x0 < clip_x1 && clip_y0 < clip_y1;

    // Scroll when the text is wider than the visible area.
    if (text_width > visible_width && visible_width > 0 && !vision_ui_is_background_frozen(ui)) {
        int16_t scroll_offset = 0;
        const int16_t overflow = text_width - visible_width;
        const float speed_px_s = scroll_speed_s; // px/s
        const uint32_t forward_ms = (uint32_t) (1000.f * overflow / speed_px_s + 0.5f);

        const uint32_t now = vision_ui_driver_ticks_ms_get(ui);
        if (*text_scroll_anchor == 0) {
            *text_scroll_anchor = now;
        }

        const uint32_t elapsed = now - *text_scroll_anchor;

        const uint32_t cycle_ms = (scroll_pause_ms + forward_ms) * 2u;
        if (forward_ms > 0 && cycle_ms > 0) {
            const uint32_t phase = elapsed % cycle_ms;
            if (phase < scroll_pause_ms) {
                scroll_offset = 0;
            } else if (phase < scroll_pause_ms + forward_ms) {
                const float p = (float) (phase - scroll_pause_ms) / (float) forward_ms;
                scroll_offset = (int16_t) lrintf((float) overflow * p);
            } else if (phase < scroll_pause_ms + forward_ms + scroll_pause_ms) {
                scroll_offset = overflow;
            } else {
                const float p = (float) (phase - scroll_pause_ms - forward_ms - scroll_pause_ms) / (float) forward_ms;
                scroll_offset = (int16_t) lrintf((float) overflow * (1.f - p));
            }
        } else {
            scroll_offset = 0;
        }

        if (has_visible_area) {
#ifdef DEBUG
            vision_ui_driver_frame_draw(ui, clip_x0, clip_y0, clip_x1 - clip_x0, clip_y1 - clip_y0);
            vision_ui_driver_frame_draw(
                    ui,
                    text_x,
                    text_y - vision_ui_driver_str_height_get(ui),
                    vision_ui_driver_str_width_get(ui, text),
                    vision_ui_driver_str_height_get(ui)
            );
#endif
            vision_ui_driver_clip_window_set(ui, clip_x0, clip_y0, clip_x1, clip_y1);
            vision_ui_driver_str_utf8_draw(ui, text_x - scroll_offset, text_y, text);
            vision_ui_driver_clip_window_reset(ui);
        }
    } else {
        // No scrolling needed; draw it directly.
        *text_scroll_anchor = 0;
        if (has_visible_area) {
#ifdef DEBUG
            vision_ui_driver_frame_draw(ui, clip_x0, clip_y0, clip_x1 - clip_x0, clip_y1 - clip_y0);
            vision_ui_driver_frame_draw(
                    ui,
                    text_x,
                    text_y - vision_ui_driver_str_height_get(ui),
                    vision_ui_driver_str_width_get(ui, text),
                    vision_ui_driver_str_height_get(ui)
            );
#endif
            vision_ui_driver_clip_window_set(ui, clip_x0, clip_y0, clip_x1, clip_y1);
            vision_ui_driver_str_utf8_draw(ui, text_x, text_y, text);
            vision_ui_driver_clip_window_reset(ui);
        }
    }
}
static void vision_ui_text_list_item_draw(
        const vision_ui_t* ui,
        vision_ui_list_item_t* list,
        const int16_t x0,
        const int16_t y0,
        const int16_t x1,
        const int16_t y1
) {
    vision_ui_text_draw(
            ui,
            list->content,
            &list->text_scroll_anchor,
            x0,
            y0,
            x1,
            y1,
            VISION_UI_LIST_TEXT_SCROLL_SPEED_PX_S,
            VISION_UI_LIST_TEXT_SCROLL_PAUSE_MS
    );
}

static void vision_ui_draw_list_header(vision_ui_t* ui) {
    for (uint8_t i = 0; i < vision_ui_selector_mutable_instance_get(ui)->selected_item->parent->child_num; i++) {
        const vision_ui_list_item_t* current_list_item =
                vision_ui_selector_instance_get(ui)->selected_item->parent->child_list_item[i];
        const int16_t x_list_item =
                vision_ui_camera_instance_get(ui)->x_camera + VISION_UI_LIST_HEADER_TO_LEFT_DISPLAY_PADDING;
        const int16_t y_list_item =
                current_list_item->y_list_item + vision_ui_camera_instance_get(ui)->y_camera +
                (VISION_UI_LIST_FRAME_FIXED_HEIGHT - vision_ui_list_icon_get_current(ui).header_height) / 2;
        // draw header
        vision_ui_driver_color_draw(ui, 1);
        const uint16_t header_base_x = x_list_item;
        if (current_list_item->type == ListItem) {
            if (vision_ui_list_icon_get_current(ui).list_header == NULL) {
                vision_ui_driver_frame_draw(
                        ui,
                        header_base_x,
                        y_list_item,
                        vision_ui_list_icon_get_current(ui).header_width,
                        vision_ui_list_icon_get_current(ui).header_height
                );
            } else {
                vision_ui_driver_bmp_draw(
                        ui,
                        header_base_x,
                        y_list_item,
                        vision_ui_list_icon_get_current(ui).header_width,
                        vision_ui_list_icon_get_current(ui).header_height,
                        vision_ui_list_icon_get_current(ui).list_header
                );
            }
        } else if (current_list_item->type == SwitchItem) {
            if (vision_ui_list_icon_get_current(ui).switch_header == NULL) {
                vision_ui_driver_frame_draw(
                        ui,
                        header_base_x,
                        y_list_item,
                        vision_ui_list_icon_get_current(ui).header_width,
                        vision_ui_list_icon_get_current(ui).header_height
                );
            } else {
                vision_ui_driver_bmp_draw(
                        ui,
                        header_base_x,
                        y_list_item,
                        vision_ui_list_icon_get_current(ui).header_width,
                        vision_ui_list_icon_get_current(ui).header_height,
                        vision_ui_list_icon_get_current(ui).switch_header
                );
            }
        } else if (current_list_item->type == SliderItem) {
            if (vision_ui_list_icon_get_current(ui).slider_header == NULL) {
                vision_ui_driver_frame_draw(
                        ui,
                        header_base_x,
                        y_list_item,
                        vision_ui_list_icon_get_current(ui).header_width,
                        vision_ui_list_icon_get_current(ui).header_height
                );
            } else {
                vision_ui_driver_bmp_draw(
                        ui,
                        header_base_x,
                        y_list_item,
                        vision_ui_list_icon_get_current(ui).header_width,
                        vision_ui_list_icon_get_current(ui).header_height,
                        vision_ui_list_icon_get_current(ui).slider_header
                );
            }
        } else if (current_list_item->type == TitleItem) {
            // do nothing
        } else {
            if (vision_ui_list_icon_get_current(ui).default_header == NULL) {
                vision_ui_driver_frame_draw(
                        ui,
                        header_base_x,
                        y_list_item,
                        vision_ui_list_icon_get_current(ui).header_width,
                        vision_ui_list_icon_get_current(ui).header_height
                );
            } else {
                vision_ui_driver_bmp_draw(
                        ui,
                        header_base_x,
                        y_list_item,
                        vision_ui_list_icon_get_current(ui).header_width,
                        vision_ui_list_icon_get_current(ui).header_height,
                        vision_ui_list_icon_get_current(ui).default_header
                );
            }
        }
    }
}

static void vision_ui_draw_list_footer(const vision_ui_t* ui) {
    for (uint8_t i = 0; i < vision_ui_selector_instance_get(ui)->selected_item->parent->child_num; i++) {
        vision_ui_list_item_t* current_list_item =
                vision_ui_selector_instance_get(ui)->selected_item->parent->child_list_item[i];
        const int16_t y_list_item = current_list_item->y_list_item + vision_ui_camera_instance_get(ui)->y_camera;

        // draw header
        const int16_t frame_x = VISION_UI_SCREEN_WIDTH - VISION_UI_LIST_SCROLL_BAR_WIDTH -
                                VISION_UI_LIST_FOOTER_RIGHT_TO_SCROLL_BAR_PADDING -
                                (current_list_item->type != SliderItem ? VISION_UI_LIST_FOOTER_MAX_WIDTH
                                                                       : VISION_UI_LIST_SLIDER_FOOTER_WIDTH);
        const int16_t frame_y =
                y_list_item + (VISION_UI_LIST_FRAME_FIXED_HEIGHT - VISION_UI_LIST_FOOTER_MAX_HEIGHT) / 2;
        if (current_list_item->type == ListItem) {
        } else if (current_list_item->type == SwitchItem) {
            if (vision_ui_to_list_switch_item(current_list_item)->value == true) {
                vision_ui_driver_color_draw(ui, 1);
                if (vision_ui_list_icon_get_current(ui).switch_on_footer == NULL) {
                    vision_ui_driver_frame_draw(
                            ui,
                            frame_x,
                            frame_y,
                            vision_ui_list_icon_get_current(ui).footer_width,
                            vision_ui_list_icon_get_current(ui).footer_height
                    );
                } else {
                    vision_ui_driver_bmp_draw(
                            ui,
                            frame_x,
                            frame_y,
                            vision_ui_list_icon_get_current(ui).footer_width,
                            vision_ui_list_icon_get_current(ui).footer_height,
                            vision_ui_list_icon_get_current(ui).switch_on_footer
                    );
                }
            } else {
                vision_ui_driver_color_draw(ui, 1);
                if (vision_ui_list_icon_get_current(ui).switch_off_footer == NULL) {
                    vision_ui_driver_frame_draw(
                            ui,
                            frame_x,
                            frame_y,
                            vision_ui_list_icon_get_current(ui).footer_width,
                            vision_ui_list_icon_get_current(ui).footer_height
                    );
                } else {
                    vision_ui_driver_bmp_draw(
                            ui,
                            frame_x,
                            frame_y,
                            vision_ui_list_icon_get_current(ui).footer_width,
                            vision_ui_list_icon_get_current(ui).footer_height,
                            vision_ui_list_icon_get_current(ui).switch_off_footer
                    );
                }
            }
        } else if (current_list_item->type == SliderItem) {
            const uint16_t shrink_width = VISION_UI_LIST_SLIDER_FOOTER_WIDTH - 4;
            const int16_t footer_x0 = frame_x + (VISION_UI_LIST_SLIDER_FOOTER_WIDTH - shrink_width) / 2;
            const int16_t footer_y0 = frame_y;
            const int16_t footer_x1 = footer_x0 + shrink_width;
            const int16_t footer_y1 = footer_y0 + VISION_UI_LIST_FOOTER_MAX_HEIGHT;

            char value_str[10] = {};
            sprintf(value_str, "%d", vision_ui_to_list_slider_item(current_list_item)->value);

            vision_ui_driver_color_draw(ui, 1);
            vision_ui_text_draw(
                    ui,
                    value_str,
                    &vision_ui_to_list_slider_item(current_list_item)->text_scroll_anchor,
                    footer_x0,
                    footer_y0,
                    footer_x1,
                    footer_y1,
                    VISION_UI_LIST_SLIDER_VALUE_SCROLL_SPEED_PX_S,
                    VISION_UI_LIST_SLIDER_VALUE_SCROLL_PAUSE_MS
            );
            if (vision_ui_to_list_slider_item(current_list_item)->is_confirmed) {
                vision_ui_driver_color_draw(ui, 2);
                if (vision_ui_list_icon_get_current(ui).slider_footer == NULL) {
                    vision_ui_driver_frame_draw(
                            ui,
                            frame_x,
                            frame_y,
                            VISION_UI_LIST_SLIDER_FOOTER_WIDTH,
                            vision_ui_list_icon_get_current(ui).footer_height
                    );
                } else {
                    vision_ui_driver_bmp_draw(
                            ui,
                            frame_x,
                            frame_y,
                            VISION_UI_LIST_SLIDER_FOOTER_WIDTH,
                            vision_ui_list_icon_get_current(ui).footer_height,
                            vision_ui_list_icon_get_current(ui).slider_footer
                    );
                }
            }
        }
    }
}

static void vision_ui_list_item_render(vision_ui_t* ui) {
    vision_ui_draw_list_header(ui);
    vision_ui_draw_list_footer(ui);

    for (uint8_t i = 0; i < vision_ui_selector_instance_get(ui)->selected_item->parent->child_num; i++) {
        vision_ui_list_item_t* current_list_item =
                vision_ui_selector_instance_get(ui)->selected_item->parent->child_list_item[i];
        const int16_t x_list_item =
                vision_ui_camera_instance_get(ui)->x_camera + VISION_UI_LIST_HEADER_TO_LEFT_DISPLAY_PADDING;
        const int16_t y_list_item = current_list_item->y_list_item + vision_ui_camera_instance_get(ui)->y_camera;

        const int16_t frame_x = current_list_item->type == TitleItem ? x_list_item
                                                                     : x_list_item + VISION_UI_LIST_HEADER_MAX_WIDTH +
                                                                               VISION_UI_LIST_HEADER_TO_TEXT_PADDING;

        const int16_t frame_y = y_list_item;

        vision_ui_font_set(ui, vision_ui_font_get(ui));
        vision_ui_driver_color_draw(ui, 1);
        vision_ui_text_list_item_draw(
                ui,
                current_list_item,
                frame_x,
                frame_y,
                frame_x + VISION_UI_LIST_TEXT_MAX_WIDTH(current_list_item),
                y_list_item + VISION_UI_LIST_FRAME_FIXED_HEIGHT
        );
    }
}

static void vision_ui_icon_view_render(vision_ui_t* ui) {
    const vision_ui_selector_t* selector = vision_ui_selector_instance_get(ui);
    vision_ui_list_item_t* icon_list = NULL;
    vision_ui_list_item_t* selected_icon_item = NULL;
    if (!vision_ui_icon_view_context(selector, &icon_list, &selected_icon_item) || icon_list == NULL) {
        return;
    }
    if (selected_icon_item == NULL) {
        return;
    }

    const float scroll_offset = icon_list->icon_scroll_offset;
    const float item_span = (float) (VISION_UI_ICON_VIEW_ICON_SIZE + VISION_UI_ICON_VIEW_ITEM_SPACING);
    const int16_t center_x = VISION_UI_SCREEN_WIDTH / 2;

    for (uint8_t i = 0; i < icon_list->child_num; ++i) {
        vision_ui_list_item_t* child = icon_list->child_list_item[i];
        if (child->type != IconItem) {
            continue;
        }

        const float icon_center = (float) center_x + scroll_offset + (float) i * item_span;
        const int16_t icon_x = (int16_t) lrintf(icon_center) - VISION_UI_ICON_VIEW_ICON_SIZE / 2;
        if (icon_x + VISION_UI_ICON_VIEW_ICON_SIZE < 0 || icon_x > VISION_UI_SCREEN_WIDTH) {
            continue;
        }

        const vision_ui_icon_item_t* icon_item = vision_ui_to_list_icon_item(child);
        vision_ui_driver_color_draw(ui, 1);
        if (icon_item->icon != NULL) {
            vision_ui_driver_bmp_draw(
                    ui,
                    icon_x,
                    VISION_UI_ICON_VIEW_ICON_TO_TOP_DISPLAY_PADDING,
                    VISION_UI_ICON_VIEW_ICON_SIZE,
                    VISION_UI_ICON_VIEW_ICON_SIZE,
                    icon_item->icon
            );

            vision_ui_driver_color_draw(ui, 2);
            vision_ui_driver_box_draw(
                    ui,
                    icon_x,
                    VISION_UI_ICON_VIEW_ICON_TO_TOP_DISPLAY_PADDING,
                    VISION_UI_ICON_VIEW_ICON_SIZE,
                    VISION_UI_ICON_VIEW_ICON_SIZE
            );
        } else {
            vision_ui_driver_frame_draw(
                    ui,
                    icon_x,
                    VISION_UI_ICON_VIEW_ICON_TO_TOP_DISPLAY_PADDING,
                    VISION_UI_ICON_VIEW_ICON_SIZE,
                    VISION_UI_ICON_VIEW_ICON_SIZE
            );
            vision_ui_driver_line_h_dotted_draw(
                    ui,
                    icon_x,
                    VISION_UI_ICON_VIEW_ICON_TO_TOP_DISPLAY_PADDING + VISION_UI_ICON_VIEW_ICON_SIZE / 2,
                    VISION_UI_ICON_VIEW_ICON_SIZE
            );
        }
    }

    const int16_t title_area_y0 = VISION_UI_ICON_VIEW_ICON_TO_TOP_DISPLAY_PADDING + VISION_UI_ICON_VIEW_ICON_SIZE +
                                  VISION_UI_ICON_VIEW_ICON_TO_TITLE_AREA_PADDING;
    const int16_t title_bar_x0 = VISION_UI_ICON_VIEW_TITLE_BAR_TO_LEFT_DISPLAY_PADDING;
    const int16_t title_x0 =
            title_bar_x0 + VISION_UI_ICON_VIEW_TITLE_BAR_WIDTH + VISION_UI_ICON_VIEW_TITLE_BAR_TO_TITLE_PADDING;
    const int16_t title_x1 = VISION_UI_SCREEN_WIDTH - VISION_UI_ICON_VIEW_TITLE_TO_RIGHT_DISPLAY_MIN_PADDING;

    vision_ui_driver_font_set(ui, vision_ui_font_get_title(ui));
    const int16_t title_y1 = title_area_y0 + VISION_UI_ICON_VIEW_TITLE_AREA_HEIGHT;

    vision_ui_driver_color_draw(ui, 1);
    vision_ui_driver_box_draw(
            ui, title_bar_x0, title_area_y0, VISION_UI_ICON_VIEW_TITLE_BAR_WIDTH, VISION_UI_ICON_VIEW_TITLE_AREA_HEIGHT
    );

    vision_ui_icon_item_t* selected_icon = vision_ui_to_list_icon_item(selected_icon_item);
    const float title_offset = selected_icon->title_y;
    const int16_t title_offset_px = (int16_t) lrintf(title_offset);

    vision_ui_text_draw(
            ui,
            selected_icon_item->content,
            &selected_icon_item->text_scroll_anchor,
            title_x0,
            title_area_y0 + title_offset_px,
            title_x1,
            title_y1,
            VISION_UI_LIST_TEXT_SCROLL_SPEED_PX_S,
            VISION_UI_LIST_TEXT_SCROLL_PAUSE_MS
    );
    vision_ui_driver_font_set(ui, vision_ui_font_get_subtitle(ui));

    if (selected_icon->description != NULL) {
        uint16_t spacing;
        if (vision_ui_driver_str_width_get(ui, selected_icon->description) >
            VISION_UI_SCREEN_WIDTH - 2 * VISION_UI_ICON_VIEW_DESCRIPTION_TO_DISPLAY_MIN_SPACING) {
            spacing = VISION_UI_ICON_VIEW_DESCRIPTION_TO_DISPLAY_MIN_SPACING;
        } else {
            spacing = (VISION_UI_SCREEN_WIDTH - vision_ui_driver_str_width_get(ui, selected_icon->description)) / 2;
        }
        const uint16_t description_x0 = spacing;
        const uint16_t description_x1 = VISION_UI_SCREEN_WIDTH - spacing;
        const uint16_t description_y0 = title_area_y0 + VISION_UI_ICON_VIEW_TITLE_AREA_HEIGHT +
                                        VISION_UI_ICON_VIEW_TITLE_AREA_TO_DESCRIPTION_PADDING;
        const uint16_t description_y1 = description_y0 + VISION_UI_ICON_VIEW_DESCRIPTION_AREA_HEIGHT;
        vision_ui_driver_color_draw(ui, 1);
        vision_ui_text_draw(
                ui,
                selected_icon->description,
                &selected_icon->description_scroll_anchor,
                description_x0,
                description_y0,
                description_x1,
                description_y1,
                VISION_UI_LIST_TEXT_SCROLL_SPEED_PX_S,
                VISION_UI_LIST_TEXT_SCROLL_PAUSE_MS
        );
    }
}

static void vision_ui_selector_render(const vision_ui_t* ui) {
    const int16_t x_selector =
            (int16_t) (lrintf(vision_ui_camera_instance_get(ui)->x_camera) +
                       VISION_UI_LIST_HEADER_TO_LEFT_DISPLAY_PADDING - VISION_UI_LIST_SELECTOR_TO_INNER_WIDGET_PADDING);
    const int16_t y_selector = (int16_t) lrintf(
            vision_ui_selector_instance_get(ui)->y_selector + vision_ui_camera_instance_get(ui)->y_camera
    );

    vision_ui_driver_color_draw(ui, 1);
    const int16_t selector_w = (int16_t) lrintf(vision_ui_selector_instance_get(ui)->w_selector);
    const int16_t selector_h = (int16_t) lrintf(vision_ui_selector_instance_get(ui)->h_selector);

    vision_ui_driver_frame_r_draw(ui, x_selector, y_selector, selector_w, selector_h, 3);
    vision_ui_driver_color_draw(ui, 2);
    vision_ui_driver_box_draw(ui, x_selector + 1, y_selector + 2, selector_w - 2, selector_h - 4);
    vision_ui_driver_line_h_draw(ui, x_selector + 2, y_selector + 1, selector_w - 4);
    vision_ui_driver_line_h_draw(ui, x_selector + 2, y_selector + selector_h - 2, selector_w - 4);
}

void vision_ui_widget_render(vision_ui_t* ui) {
    vision_ui_notification_render(ui);

    // everything else should be blured except the alert object
    if (vision_ui_is_background_frozen(ui)) {
        vision_ui_background_blur_animation_render(ui, 0, 0, VISION_UI_SCREEN_WIDTH, VISION_UI_SCREEN_HEIGHT, 4);
    }
    vision_ui_alert_render(ui);
}

void vision_ui_list_render(vision_ui_t* ui) {
    if (vision_ui_icon_view_is_active(ui)) {
        vision_ui_icon_view_render(ui);
        return;
    }

    // Run all list-related draw routines.
    vision_ui_list_appearance_render(ui);
    vision_ui_list_item_render(ui);
    vision_ui_selector_render(ui);
}
