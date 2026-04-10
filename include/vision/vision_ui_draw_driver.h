//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//

#ifndef VISION_UI_VISION_UI_DRAW_DRIVER_H
#define VISION_UI_VISION_UI_DRAW_DRIVER_H

#include <stdint.h>

#include "vision_ui_types.h"

extern vision_ui_action_t vision_ui_driver_action_get(const vision_ui_t* ui);

extern uint32_t vision_ui_driver_ticks_ms_get(const vision_ui_t* ui);

extern void vision_ui_driver_delay(const vision_ui_t* ui, uint32_t ms);

extern void vision_ui_driver_bind(vision_ui_t* ui, void* driver);

extern void vision_ui_driver_font_set(vision_ui_t* ui, vision_ui_font_t font);

extern vision_ui_font_t vision_ui_driver_font_get(const vision_ui_t* ui);

extern void vision_ui_driver_str_draw(const vision_ui_t* ui, uint16_t x, uint16_t y, const char* str);

extern void vision_ui_driver_str_utf8_draw(const vision_ui_t* ui, uint16_t x, uint16_t y, const char* str);

extern uint16_t vision_ui_driver_str_width_get(const vision_ui_t* ui, const char* str);

extern uint16_t vision_ui_driver_str_utf8_width_get(const vision_ui_t* ui, const char* str);

extern uint16_t vision_ui_driver_str_height_get(const vision_ui_t* ui);

extern void vision_ui_driver_pixel_draw(const vision_ui_t* ui, uint16_t x, uint16_t y);

extern void vision_ui_driver_circle_draw(const vision_ui_t* ui, uint16_t x, uint16_t y, uint16_t r);

extern void vision_ui_driver_disc_draw(const vision_ui_t* ui, uint16_t x, uint16_t y, uint16_t r);

extern void vision_ui_driver_box_r_draw(
        const vision_ui_t* ui,
        uint16_t x,
        uint16_t y,
        uint16_t w,
        uint16_t h,
        uint16_t r
);

extern void vision_ui_driver_box_draw(const vision_ui_t* ui, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

extern void vision_ui_driver_frame_draw(const vision_ui_t* ui, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

extern void vision_ui_driver_frame_r_draw(
        const vision_ui_t* ui,
        uint16_t x,
        uint16_t y,
        uint16_t w,
        uint16_t h,
        uint16_t r
);

extern void vision_ui_driver_line_h_draw(const vision_ui_t* ui, uint16_t x, uint16_t y, uint16_t l);

extern void vision_ui_driver_line_v_draw(const vision_ui_t* ui, uint16_t x, uint16_t y, uint16_t h);

extern void vision_ui_driver_line_draw(const vision_ui_t* ui, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

extern void vision_ui_driver_line_h_dotted_draw(const vision_ui_t* ui, uint16_t x, uint16_t y, uint16_t l);

extern void vision_ui_driver_line_v_dotted_draw(const vision_ui_t* ui, uint16_t x, uint16_t y, uint16_t h);

extern void vision_ui_driver_bmp_draw(
        const vision_ui_t* ui,
        uint16_t x,
        uint16_t y,
        uint16_t w,
        uint16_t h,
        const uint8_t* bit_map
);

extern void vision_ui_driver_color_draw(const vision_ui_t* ui, uint8_t color);

extern void vision_ui_driver_font_mode_set(const vision_ui_t* ui, uint8_t mode);

extern void vision_ui_driver_font_direction_set(const vision_ui_t* ui, uint8_t dir);

extern void vision_ui_driver_clip_window_set(const vision_ui_t* ui, int16_t x0, int16_t y0, int16_t x1, int16_t y1);

extern void vision_ui_driver_clip_window_reset(const vision_ui_t* ui);

extern void vision_ui_driver_buffer_clear(const vision_ui_t* ui);

extern void vision_ui_driver_buffer_send(const vision_ui_t* ui);

extern void vision_ui_driver_buffer_area_send(const vision_ui_t* ui, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

/// @attention this should be a full size buffer
extern void* vision_ui_driver_buffer_pointer_get(const vision_ui_t* ui);

#endif // VISION_UI_VISION_UI_DRAW_DRIVER_H
