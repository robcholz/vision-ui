/*
U8G2Simulator
Copyright (C) 2025  Finn Sheng

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include "u8g2.h"

#include "vision/astra_ui_draw_driver.h"

#include <u8g2.h>

#include <SDL_events.h>
#include <SDL_timer.h>

static u8g2_t* s_u8g2 = NULL;

uint8_t buffer[128*8];

static int getKey() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT)
            return SDLK_ESCAPE;
        if (event.type == SDL_KEYDOWN)
            return event.key.keysym.sym;
    }
    return -1;
}

enum UIAction vision_ui_get_ui_action(void) {
    switch (getKey()) {
        case SDLK_UP:
            return UIActionGoPrev;
        case SDLK_DOWN:
            return UIActionGoNext;
        case SDLK_ESCAPE:
            return UIActionExit;
        case SDLK_SPACE:
            return UIActionEnter;
        default:
            return UIActionNone;
    }
}

uint32_t get_ticks_ms(void) {
    return SDL_GetTicks();
}

void delay(uint32_t ms) {
    SDL_Delay(ms);
}

void vision_ui_bind_driver(void* driver) {
    s_u8g2 = (u8g2_t*) driver;
}

void oled_set_font(const uint8_t* font) {
    u8g2_SetFont(s_u8g2, font);
}

void oled_draw_str(uint16_t x, const uint16_t y, const char* str) {
    u8g2_DrawStr(s_u8g2, x, y, str);
}

void oled_draw_UTF8(uint16_t x, uint16_t y, const char* str) {
    u8g2_DrawUTF8(s_u8g2, x, y, str);
}

uint16_t oled_get_str_width(const char* str) {
    return (uint16_t) u8g2_GetStrWidth(s_u8g2, str);
}

uint16_t oled_get_UTF8_width(const char* str) {
    return (uint16_t) u8g2_GetUTF8Width(s_u8g2, str);
}

uint16_t oled_get_str_height(void) {
    /* 更稳妥：使用 Ascent/Descent 计算文字高度 */
    int8_t ascent = u8g2_GetAscent(s_u8g2);
    int8_t descent = u8g2_GetDescent(s_u8g2); /* 注意：通常为负值 */
    return (uint16_t) (ascent - descent);
}

/* 图元 */
void oled_draw_pixel(uint16_t x, uint16_t y) {
    u8g2_DrawPixel(s_u8g2, x, y);
}

void oled_draw_circle(uint16_t x, uint16_t y, uint16_t r) {
    /* u8g2_DrawCircle 画轮廓；若需实心可用 u8g2_DrawDisc */
    u8g2_DrawCircle(s_u8g2, x, y, r, U8G2_DRAW_ALL);
}

void oled_draw_R_box(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r) {
    u8g2_DrawRBox(s_u8g2, x, y, w, h, r);
}

void oled_draw_box(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    u8g2_DrawBox(s_u8g2, x, y, w, h);
}

void oled_draw_frame(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    u8g2_DrawFrame(s_u8g2, x, y, w, h);
}

void oled_draw_R_frame(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r) {
    u8g2_DrawRFrame(s_u8g2, x, y, w, h, r);
}

void oled_draw_H_line(uint16_t x, uint16_t y, uint16_t l) {
    u8g2_DrawHLine(s_u8g2, x, y, l);
}

void oled_draw_V_line(uint16_t x, uint16_t y, uint16_t h) {
    u8g2_DrawVLine(s_u8g2, x, y, h);
}

void oled_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    u8g2_DrawLine(s_u8g2, x1, y1, x2, y2);
}

/* 虚线：简单 1像素间隔（可按需改为更大的 dash/gap） */
void oled_draw_H_dotted_line(uint16_t x, uint16_t y, uint16_t l) {
    for (uint16_t i = 0; i < l; i += 2) {
        u8g2_DrawPixel(s_u8g2, x + i, y);
    }
}

void oled_draw_V_dotted_line(uint16_t x, uint16_t y, uint16_t h) {
    for (uint16_t i = 0; i < h; i += 2) {
        u8g2_DrawPixel(s_u8g2, x, y + i);
    }
}

/* 位图：假定 bitMap 为 XBM(1bpp) 布局（和 u8g2_DrawXBM 兼容） */
void oled_draw_bMP(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t* bitMap) {
    u8g2_DrawXBM(s_u8g2, x, y, w, h, bitMap);
}

/* 属性 */
void oled_set_draw_color(uint8_t color) {
    /* u8g2: 0=clear, 1=set, 2=XOR (部分设备/后端可能不支持 XOR) */
    u8g2_SetDrawColor(s_u8g2, color);
}

void oled_set_font_mode(uint8_t mode) {
    /* 0=transparent, 1=solid */
    u8g2_SetFontMode(s_u8g2, mode ? 1 : 0);
}

void oled_set_font_direction(uint8_t dir) {
    /* 方向 0..3：0=左→右，1=下→上，2=右→左，3=上→下 */
    u8g2_SetFontDirection(s_u8g2, (uint8_t) (dir & 0x03));
}

void oled_set_clip_window(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    u8g2_SetClipWindow(s_u8g2, x0, y0, x1, y1);
}

void oled_reset_clip_window(void) {
    u8g2_SetMaxClipWindow(s_u8g2);
}

/* 缓冲区 */
void oled_clear_buffer(void) {
    u8g2_ClearBuffer(s_u8g2);
}

void oled_send_buffer(void) {
    u8g2_SendBuffer(s_u8g2);
}

/* 部分区域刷新（若驱动支持 tile-based 更新则调用；否则退化为全刷） */
void oled_send_area_buffer(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    /* u8g2_UpdateDisplayArea 以 tile(8x8) 为单位；
       只有部分驱动/后端支持；否则该调用不会生效。 */
#if U8G2_WITH_CLIP_WINDOW  /* 一些构建里会定义，但不可靠；做宽松处理 */
    uint16_t x_end = x + w - 1;
    uint16_t y_end = y + h - 1;
    /* 像素->tile 的向下取整/向上取整 */
    uint8_t tx = (uint8_t) (x / 8);
    uint8_t ty = (uint8_t) (y / 8);
    uint8_t tw = (uint8_t) ((x_end / 8) - tx + 1);
    uint8_t th = (uint8_t) ((y_end / 8) - ty + 1);
    u8g2_UpdateDisplayArea(s_u8g2, tx, ty, tw, th);
#else
    (void) x;
    (void) y;
    (void) w;
    (void) h;
    u8g2_SendBuffer(s_u8g2); /* 回退全刷 */
#endif
}

extern void* oled_get_raw_buffer_pointer() {
    return buffer;
}
