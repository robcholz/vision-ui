//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//

#ifndef VISION_UI_VISION_UI_DRAW_DRIVER_H
#define VISION_UI_VISION_UI_DRAW_DRIVER_H

#include "vision_ui_core.h"

#include <stdint.h>

extern vision_ui_action_t vision_ui_driver_action_get();

extern uint32_t vision_ui_driver_ticks_ms_get();

extern void vision_ui_driver_delay(uint32_t ms);

extern void vision_ui_driver_bind(void* driver);

extern void vision_ui_driver_font_set(const uint8_t* font);

extern void vision_ui_driver_str_draw(uint16_t x, uint16_t y, const char* str);

extern void vision_ui_driver_str_utf8_draw(uint16_t x, uint16_t y, const char* str);

extern uint16_t vision_ui_driver_str_width_get(const char* str);

extern uint16_t vision_ui_driver_str_utf8_width_get(const char* str);

extern uint16_t vision_ui_driver_str_height_get();

extern void vision_ui_driver_pixel_draw(uint16_t x, uint16_t y);

extern void vision_ui_driver_circle_draw(uint16_t x, uint16_t y, uint16_t r);

extern void vision_ui_driver_box_r_draw(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r);

extern void vision_ui_driver_box_draw(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

extern void vision_ui_driver_frame_draw(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

extern void vision_ui_driver_frame_r_draw(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r);

extern void vision_ui_driver_line_h_draw(uint16_t x, uint16_t y, uint16_t l);

extern void vision_ui_driver_line_v_draw(uint16_t x, uint16_t y, uint16_t h);

extern void vision_ui_driver_line_draw(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

extern void vision_ui_driver_line_h_dotted_draw(uint16_t x, uint16_t y, uint16_t l);

extern void vision_ui_driver_line_v_dotted_draw(uint16_t x, uint16_t y, uint16_t h);

extern void vision_ui_driver_bmp_draw(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t* bit_map);

extern void vision_ui_driver_color_draw(uint8_t color);

extern void vision_ui_driver_font_mode_set(uint8_t mode);

extern void vision_ui_driver_font_direction_set(uint8_t dir);

extern void vision_ui_driver_clip_window_set(int16_t x0, int16_t y0, int16_t x1, int16_t y1);

extern void vision_ui_driver_clip_window_reset();

extern void vision_ui_driver_buffer_clear();

extern void vision_ui_driver_buffer_send();

extern void vision_ui_driver_buffer_area_send(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

/// @attention this should be a full size buffer
extern void* vision_ui_driver_buffer_pointer_get();

#endif // VISION_UI_VISION_UI_DRAW_DRIVER_H
