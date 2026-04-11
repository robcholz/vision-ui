#ifndef TEST_MOCK_DRIVER_H
#define TEST_MOCK_DRIVER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <vision_ui_config.h>
#include <vision/vision_ui.h>

#define MOCK_DRIVER_BUFFER_SIZE (VISION_UI_SCREEN_WIDTH * (VISION_UI_SCREEN_HEIGHT / 8))

typedef struct mock_driver_t {
    vision_ui_action_t action;
    uint32_t ticks_ms;
    uint32_t total_delay_ms;

    vision_ui_font_t active_font;
    uint8_t draw_color;
    uint8_t font_mode;
    uint8_t font_direction;

    bool clip_enabled;
    int16_t clip_x0;
    int16_t clip_y0;
    int16_t clip_x1;
    int16_t clip_y1;

    uint32_t action_get_count;
    uint32_t bind_count;
    uint32_t font_set_count;
    uint32_t str_draw_count;
    uint32_t str_utf8_draw_count;
    uint32_t pixel_draw_count;
    uint32_t circle_draw_count;
    uint32_t disc_draw_count;
    uint32_t box_r_draw_count;
    uint32_t box_draw_count;
    uint32_t frame_draw_count;
    uint32_t frame_r_draw_count;
    uint32_t line_h_draw_count;
    uint32_t line_v_draw_count;
    uint32_t line_draw_count;
    uint32_t line_h_dotted_draw_count;
    uint32_t line_v_dotted_draw_count;
    uint32_t bmp_draw_count;
    uint32_t color_draw_count;
    uint32_t font_mode_set_count;
    uint32_t font_direction_set_count;
    uint32_t clip_set_count;
    uint32_t clip_reset_count;
    uint32_t buffer_clear_count;
    uint32_t buffer_send_count;
    uint32_t buffer_area_send_count;
    uint32_t buffer_pointer_get_count;

    uint16_t last_x;
    uint16_t last_y;
    uint16_t last_w;
    uint16_t last_h;
    uint16_t last_r;
    char last_text[128];

    uint8_t buffer[MOCK_DRIVER_BUFFER_SIZE];
} mock_driver_t;

extern void mock_driver_init(mock_driver_t* driver);
extern void mock_driver_set_action(mock_driver_t* driver, vision_ui_action_t action);
extern void mock_driver_advance_time(mock_driver_t* driver, uint32_t delta_ms);
extern void mock_driver_seed_checkerboard(mock_driver_t* driver);

#endif
