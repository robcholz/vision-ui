#include "mock_driver.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static uint16_t mock_driver_text_width(const char* str) {
    return str == NULL ? 0u : (uint16_t) (strlen(str) * 6u);
}

static bool mock_driver_in_clip(const mock_driver_t* driver, const uint16_t x, const uint16_t y) {
    if (!driver->clip_enabled) {
        return x < VISION_UI_SCREEN_WIDTH && y < VISION_UI_SCREEN_HEIGHT;
    }
    return x >= (uint16_t) driver->clip_x0 && y >= (uint16_t) driver->clip_y0 && x < (uint16_t) driver->clip_x1 &&
           y < (uint16_t) driver->clip_y1;
}

static void mock_driver_set_pixel_raw(mock_driver_t* driver, const uint16_t x, const uint16_t y) {
    if (x >= VISION_UI_SCREEN_WIDTH || y >= VISION_UI_SCREEN_HEIGHT || !mock_driver_in_clip(driver, x, y)) {
        return;
    }

    const uint32_t index = ((uint32_t) (y >> 3) * VISION_UI_SCREEN_WIDTH) + x;
    const uint8_t mask = (uint8_t) (1u << (y & 0x7u));
    if (driver->draw_color == 0u) {
        driver->buffer[index] &= (uint8_t) ~mask;
    } else if (driver->draw_color == 2u) {
        driver->buffer[index] ^= mask;
    } else {
        driver->buffer[index] |= mask;
    }
}

void mock_driver_init(mock_driver_t* driver) {
    assert(driver != NULL);
    memset(driver, 0, sizeof(*driver));
    driver->draw_color = 1;
}

void mock_driver_set_action(mock_driver_t* driver, const vision_ui_action_t action) {
    assert(driver != NULL);
    driver->action = action;
}

void mock_driver_advance_time(mock_driver_t* driver, const uint32_t delta_ms) {
    assert(driver != NULL);
    driver->ticks_ms += delta_ms;
}

void mock_driver_seed_checkerboard(mock_driver_t* driver) {
    assert(driver != NULL);
    for (uint16_t y = 0; y < VISION_UI_SCREEN_HEIGHT; ++y) {
        for (uint16_t x = 0; x < VISION_UI_SCREEN_WIDTH; ++x) {
            if (((x + y) & 1u) == 0u) {
                const uint32_t index = ((uint32_t) (y >> 3) * VISION_UI_SCREEN_WIDTH) + x;
                driver->buffer[index] |= (uint8_t) (1u << (y & 0x7u));
            }
        }
    }
}

static vision_ui_action_t mock_driver_action_get(void* context) {
    mock_driver_t* driver = (mock_driver_t*) context;
    const vision_ui_action_t action = driver->action;
    driver->action_get_count++;
    driver->action = UiActionNone;
    return action;
}

static uint32_t mock_driver_ticks_ms_get(void* context) {
    return ((mock_driver_t*) context)->ticks_ms;
}

static void mock_driver_delay(void* context, const uint32_t ms) {
    mock_driver_t* driver = (mock_driver_t*) context;
    driver->total_delay_ms += ms;
}

static void mock_driver_font_set(void* context, const vision_ui_font_t font) {
    mock_driver_t* driver = (mock_driver_t*) context;
    driver->active_font = font;
    driver->font_set_count++;
}

static vision_ui_font_t mock_driver_font_get(void* context) {
    return ((mock_driver_t*) context)->active_font;
}

static void mock_driver_str_draw(void* context, const uint16_t x, const uint16_t y, const char* str) {
    mock_driver_t* driver = (mock_driver_t*) context;
    driver->str_draw_count++;
    driver->last_x = x;
    driver->last_y = y;
    snprintf(driver->last_text, sizeof(driver->last_text), "%s", str != NULL ? str : "");
}

static void mock_driver_str_utf8_draw(void* context, const uint16_t x, const uint16_t y, const char* str) {
    mock_driver_t* driver = (mock_driver_t*) context;
    driver->str_utf8_draw_count++;
    driver->last_x = x;
    driver->last_y = y;
    snprintf(driver->last_text, sizeof(driver->last_text), "%s", str != NULL ? str : "");
}

static uint16_t mock_driver_str_width_get(void* context, const char* str) {
    (void) context;
    return mock_driver_text_width(str);
}

static uint16_t mock_driver_str_utf8_width_get(void* context, const char* str) {
    (void) context;
    return mock_driver_text_width(str);
}

static uint16_t mock_driver_str_height_get(void* context) {
    const mock_driver_t* driver = (const mock_driver_t*) context;
    return (uint16_t) (8 + (driver->active_font.top_compensation > 0 ? driver->active_font.top_compensation : 0));
}

static void mock_driver_pixel_draw(void* context, const uint16_t x, const uint16_t y) {
    mock_driver_t* driver = (mock_driver_t*) context;
    driver->pixel_draw_count++;
    driver->last_x = x;
    driver->last_y = y;
    mock_driver_set_pixel_raw(driver, x, y);
}

static void mock_driver_circle_draw(void* context, const uint16_t x, const uint16_t y, const uint16_t r) {
    mock_driver_t* driver = (mock_driver_t*) context;
    driver->circle_draw_count++;
    driver->last_x = x;
    driver->last_y = y;
    driver->last_r = r;
    mock_driver_set_pixel_raw(driver, x, y);
}

static void mock_driver_disc_draw(void* context, const uint16_t x, const uint16_t y, const uint16_t r) {
    mock_driver_t* driver = (mock_driver_t*) context;
    driver->disc_draw_count++;
    driver->last_x = x;
    driver->last_y = y;
    driver->last_r = r;
    mock_driver_set_pixel_raw(driver, x, y);
}

static void mock_driver_box_r_draw(
        void* context,
        const uint16_t x,
        const uint16_t y,
        const uint16_t w,
        const uint16_t h,
        const uint16_t r
) {
    mock_driver_t* driver = (mock_driver_t*) context;
    driver->box_r_draw_count++;
    driver->last_x = x;
    driver->last_y = y;
    driver->last_w = w;
    driver->last_h = h;
    driver->last_r = r;
    for (uint16_t dy = 0; dy < h; ++dy) {
        for (uint16_t dx = 0; dx < w; ++dx) {
            mock_driver_set_pixel_raw(driver, (uint16_t) (x + dx), (uint16_t) (y + dy));
        }
    }
}

static void mock_driver_box_draw(
        void* context,
        const uint16_t x,
        const uint16_t y,
        const uint16_t w,
        const uint16_t h
) {
    mock_driver_t* driver = (mock_driver_t*) context;
    driver->box_draw_count++;
    driver->last_x = x;
    driver->last_y = y;
    driver->last_w = w;
    driver->last_h = h;
    for (uint16_t dy = 0; dy < h; ++dy) {
        for (uint16_t dx = 0; dx < w; ++dx) {
            mock_driver_set_pixel_raw(driver, (uint16_t) (x + dx), (uint16_t) (y + dy));
        }
    }
}

static void mock_driver_frame_draw(
        void* context,
        const uint16_t x,
        const uint16_t y,
        const uint16_t w,
        const uint16_t h
) {
    mock_driver_t* driver = (mock_driver_t*) context;
    driver->frame_draw_count++;
    driver->last_x = x;
    driver->last_y = y;
    driver->last_w = w;
    driver->last_h = h;
    for (uint16_t dx = 0; dx < w; ++dx) {
        mock_driver_set_pixel_raw(driver, (uint16_t) (x + dx), y);
        if (h > 0) {
            mock_driver_set_pixel_raw(driver, (uint16_t) (x + dx), (uint16_t) (y + h - 1));
        }
    }
    for (uint16_t dy = 0; dy < h; ++dy) {
        mock_driver_set_pixel_raw(driver, x, (uint16_t) (y + dy));
        if (w > 0) {
            mock_driver_set_pixel_raw(driver, (uint16_t) (x + w - 1), (uint16_t) (y + dy));
        }
    }
}

static void mock_driver_frame_r_draw(
        void* context,
        const uint16_t x,
        const uint16_t y,
        const uint16_t w,
        const uint16_t h,
        const uint16_t r
) {
    mock_driver_t* driver = (mock_driver_t*) context;
    driver->frame_r_draw_count++;
    driver->last_x = x;
    driver->last_y = y;
    driver->last_w = w;
    driver->last_h = h;
    driver->last_r = r;
    mock_driver_frame_draw(context, x, y, w, h);
}

static void mock_driver_line_h_draw(void* context, const uint16_t x, const uint16_t y, const uint16_t l) {
    mock_driver_t* driver = (mock_driver_t*) context;
    driver->line_h_draw_count++;
    driver->last_x = x;
    driver->last_y = y;
    driver->last_w = l;
    for (uint16_t i = 0; i < l; ++i) {
        mock_driver_set_pixel_raw(driver, (uint16_t) (x + i), y);
    }
}

static void mock_driver_line_v_draw(void* context, const uint16_t x, const uint16_t y, const uint16_t h) {
    mock_driver_t* driver = (mock_driver_t*) context;
    driver->line_v_draw_count++;
    driver->last_x = x;
    driver->last_y = y;
    driver->last_h = h;
    for (uint16_t i = 0; i < h; ++i) {
        mock_driver_set_pixel_raw(driver, x, (uint16_t) (y + i));
    }
}

static void mock_driver_line_draw(
        void* context,
        const uint16_t x1,
        const uint16_t y1,
        const uint16_t x2,
        const uint16_t y2
) {
    mock_driver_t* driver = (mock_driver_t*) context;
    driver->line_draw_count++;
    driver->last_x = x1;
    driver->last_y = y1;
    driver->last_w = x2;
    driver->last_h = y2;
    mock_driver_set_pixel_raw(driver, x1, y1);
    mock_driver_set_pixel_raw(driver, x2, y2);
}

static void mock_driver_line_h_dotted_draw(void* context, const uint16_t x, const uint16_t y, const uint16_t l) {
    mock_driver_t* driver = (mock_driver_t*) context;
    driver->line_h_dotted_draw_count++;
    for (uint16_t i = 0; i < l; i += 2) {
        mock_driver_set_pixel_raw(driver, (uint16_t) (x + i), y);
    }
}

static void mock_driver_line_v_dotted_draw(void* context, const uint16_t x, const uint16_t y, const uint16_t h) {
    mock_driver_t* driver = (mock_driver_t*) context;
    driver->line_v_dotted_draw_count++;
    for (uint16_t i = 0; i < h; i += 2) {
        mock_driver_set_pixel_raw(driver, x, (uint16_t) (y + i));
    }
}

static void mock_driver_bmp_draw(
        void* context,
        const uint16_t x,
        const uint16_t y,
        const uint16_t w,
        const uint16_t h,
        const uint8_t* bit_map
) {
    mock_driver_t* driver = (mock_driver_t*) context;
    (void) bit_map;
    driver->bmp_draw_count++;
    driver->last_x = x;
    driver->last_y = y;
    driver->last_w = w;
    driver->last_h = h;
    mock_driver_set_pixel_raw(driver, x, y);
}

static void mock_driver_color_draw(void* context, const uint8_t color) {
    mock_driver_t* driver = (mock_driver_t*) context;
    driver->color_draw_count++;
    driver->draw_color = color;
}

static void mock_driver_font_mode_set(void* context, const uint8_t mode) {
    mock_driver_t* driver = (mock_driver_t*) context;
    driver->font_mode_set_count++;
    driver->font_mode = mode;
}

static void mock_driver_font_direction_set(void* context, const uint8_t dir) {
    mock_driver_t* driver = (mock_driver_t*) context;
    driver->font_direction_set_count++;
    driver->font_direction = dir;
}

static void mock_driver_clip_window_set(
        void* context,
        const int16_t x0,
        const int16_t y0,
        const int16_t x1,
        const int16_t y1
) {
    mock_driver_t* driver = (mock_driver_t*) context;
    driver->clip_set_count++;
    driver->clip_enabled = true;
    driver->clip_x0 = x0;
    driver->clip_y0 = y0;
    driver->clip_x1 = x1;
    driver->clip_y1 = y1;
}

static void mock_driver_clip_window_reset(void* context) {
    mock_driver_t* driver = (mock_driver_t*) context;
    driver->clip_reset_count++;
    driver->clip_enabled = false;
}

static void mock_driver_buffer_clear(void* context) {
    mock_driver_t* driver = (mock_driver_t*) context;
    driver->buffer_clear_count++;
    memset(driver->buffer, 0, sizeof(driver->buffer));
}

static void mock_driver_buffer_send(void* context) {
    ((mock_driver_t*) context)->buffer_send_count++;
}

static void mock_driver_buffer_area_send(
        void* context,
        const uint16_t x,
        const uint16_t y,
        const uint16_t w,
        const uint16_t h
) {
    mock_driver_t* driver = (mock_driver_t*) context;
    driver->buffer_area_send_count++;
    driver->last_x = x;
    driver->last_y = y;
    driver->last_w = w;
    driver->last_h = h;
}

static void* mock_driver_buffer_pointer_get(void* context) {
    mock_driver_t* driver = (mock_driver_t*) context;
    driver->buffer_pointer_get_count++;
    return driver->buffer;
}

vision_ui_driver_t mock_driver_descriptor(mock_driver_t* driver) {
    static const vision_ui_driver_ops_t ops = {
            .action_get = mock_driver_action_get,
            .ticks_ms_get = mock_driver_ticks_ms_get,
            .delay = mock_driver_delay,
            .font_set = mock_driver_font_set,
            .font_get = mock_driver_font_get,
            .str_draw = mock_driver_str_draw,
            .str_utf8_draw = mock_driver_str_utf8_draw,
            .str_width_get = mock_driver_str_width_get,
            .str_utf8_width_get = mock_driver_str_utf8_width_get,
            .str_height_get = mock_driver_str_height_get,
            .pixel_draw = mock_driver_pixel_draw,
            .circle_draw = mock_driver_circle_draw,
            .disc_draw = mock_driver_disc_draw,
            .box_r_draw = mock_driver_box_r_draw,
            .box_draw = mock_driver_box_draw,
            .frame_draw = mock_driver_frame_draw,
            .frame_r_draw = mock_driver_frame_r_draw,
            .line_h_draw = mock_driver_line_h_draw,
            .line_v_draw = mock_driver_line_v_draw,
            .line_draw = mock_driver_line_draw,
            .line_h_dotted_draw = mock_driver_line_h_dotted_draw,
            .line_v_dotted_draw = mock_driver_line_v_dotted_draw,
            .bmp_draw = mock_driver_bmp_draw,
            .color_draw = mock_driver_color_draw,
            .font_mode_set = mock_driver_font_mode_set,
            .font_direction_set = mock_driver_font_direction_set,
            .clip_window_set = mock_driver_clip_window_set,
            .clip_window_reset = mock_driver_clip_window_reset,
            .buffer_clear = mock_driver_buffer_clear,
            .buffer_send = mock_driver_buffer_send,
            .buffer_area_send = mock_driver_buffer_area_send,
            .buffer_pointer_get = mock_driver_buffer_pointer_get,
    };

    driver->bind_count++;
    return (vision_ui_driver_t){
            .context = driver,
            .ops = &ops,
    };
}
