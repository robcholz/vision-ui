//
// Created by Fir on 24-11-29.
//

#ifndef FUCKCLION_CORE_SRC_ASTRA_UI_LITE_DRAW_DRIVER_H_
#define FUCKCLION_CORE_SRC_ASTRA_UI_LITE_DRAW_DRIVER_H_

#include <stdint.h>

#define OLED_HEIGHT 160
#define OLED_WIDTH 240

#ifdef __cplusplus
extern "C" {



#endif

extern uint32_t get_ticks(void);

extern void delay(uint32_t ms);

extern void vision_ui_bind_driver(void* driver);

extern void oled_set_font(const uint8_t* font);

extern void oled_draw_str(uint16_t x, uint16_t y, const char* str);

extern void oled_draw_UTF8(uint16_t x, uint16_t y, const char* str);

extern uint16_t oled_get_str_width(const char* str);

extern uint16_t oled_get_UTF8_width(const char* str);

extern uint16_t oled_get_str_height(void);

extern void oled_draw_pixel(uint16_t x, uint16_t y);

extern void oled_draw_circle(uint16_t x, uint16_t y, uint16_t r);

extern void oled_draw_R_box(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r);

extern void oled_draw_box(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

extern void oled_draw_frame(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

extern void oled_draw_R_frame(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r);

extern void oled_draw_H_line(uint16_t x, uint16_t y, uint16_t l);

extern void oled_draw_V_line(uint16_t x, uint16_t y, uint16_t h);

extern void oled_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

extern void oled_draw_H_dotted_line(uint16_t x, uint16_t y, uint16_t l);

extern void oled_draw_V_dotted_line(uint16_t x, uint16_t y, uint16_t h);

extern void oled_draw_bMP(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t* bitMap);

extern void oled_set_draw_color(uint8_t color);

extern void oled_set_font_mode(uint8_t mode);

extern void oled_set_font_direction(uint8_t dir);

extern void oled_clear_buffer(void);

extern void oled_send_buffer(void);

extern void oled_send_area_buffer(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

#ifdef __cplusplus
}
#endif

#endif //FUCKCLION_CORE_SRC_ASTRA_UI_LITE_DRAW_DRIVER_H_
