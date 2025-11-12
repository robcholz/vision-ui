//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//

#include "u8g2.h"

#include "vision/vision_ui_core.h"

#include <u8g2.h>

#include <SDL_events.h>
#include <SDL_timer.h>

static u8g2_t* S_U8G2 = NULL;

uint8_t U8G2_BUFFER[128 * 8];

static int get_key() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return SDLK_ESCAPE;
        }
        if (event.type == SDL_KEYDOWN) {
            return event.key.keysym.sym;
        }
    }
    return -1;
}

vision_ui_action_t vision_ui_driver_action_get() {
    switch (get_key()) {
        case SDLK_UP:
            return UI_ACTION_GO_PREV;
        case SDLK_DOWN:
            return UI_ACTION_GO_NEXT;
        case SDLK_ESCAPE:
            return UI_ACTION_EXIT;
        case SDLK_SPACE:
            return UI_ACTION_ENTER;
        default:
            return UI_ACTION_NONE;
    }
}

uint32_t vision_ui_driver_ticks_ms_get() {
    return SDL_GetTicks();
}

void vision_ui_driver_delay(uint32_t ms) {
    SDL_Delay(ms);
}

void vision_ui_driver_bind(void* driver) {
    S_U8G2 = (u8g2_t*) driver;
}

void vision_ui_driver_font_set(const uint8_t* font) {
    u8g2_SetFont(S_U8G2, font);
}

void vision_ui_driver_str_draw(uint16_t x, const uint16_t y, const char* str) {
    u8g2_DrawStr(S_U8G2, x, y, str);
}

void vision_ui_driver_str_utf8_draw(uint16_t x, uint16_t y, const char* str) {
    u8g2_DrawUTF8(S_U8G2, x, y, str);
}

uint16_t vision_ui_driver_str_width_get(const char* str) {
    return (uint16_t) u8g2_GetStrWidth(S_U8G2, str);
}

uint16_t vision_ui_driver_str_utf8_width_get(const char* str) {
    return (uint16_t) u8g2_GetUTF8Width(S_U8G2, str);
}

uint16_t vision_ui_driver_str_height_get() {
    /* 更稳妥：使用 Ascent/Descent 计算文字高度 */
    int8_t ascent = u8g2_GetAscent(S_U8G2);
    int8_t descent = u8g2_GetDescent(S_U8G2); /* 注意：通常为负值 */
    return (uint16_t) (ascent - descent);
}

/* 图元 */
void vision_ui_driver_pixel_draw(uint16_t x, uint16_t y) {
    u8g2_DrawPixel(S_U8G2, x, y);
}

void vision_ui_driver_circle_draw(uint16_t x, uint16_t y, uint16_t r) {
    /* u8g2_DrawCircle 画轮廓；若需实心可用 u8g2_DrawDisc */
    u8g2_DrawCircle(S_U8G2, x, y, r, U8G2_DRAW_ALL);
}

void vision_ui_driver_box_r_draw(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r) {
    u8g2_DrawRBox(S_U8G2, x, y, w, h, r);
}

void vision_ui_driver_box_draw(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    u8g2_DrawBox(S_U8G2, x, y, w, h);
}

void vision_ui_driver_frame_draw(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    u8g2_DrawFrame(S_U8G2, x, y, w, h);
}

void vision_ui_driver_frame_r_draw(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r) {
    u8g2_DrawRFrame(S_U8G2, x, y, w, h, r);
}

void vision_ui_driver_line_h_draw(uint16_t x, uint16_t y, uint16_t l) {
    u8g2_DrawHLine(S_U8G2, x, y, l);
}

void vision_ui_driver_line_v_draw(uint16_t x, uint16_t y, uint16_t h) {
    u8g2_DrawVLine(S_U8G2, x, y, h);
}

void vision_ui_driver_line_draw(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    u8g2_DrawLine(S_U8G2, x1, y1, x2, y2);
}

/* 虚线：简单 1像素间隔（可按需改为更大的 dash/gap） */
void vision_ui_driver_line_h_dotted_draw(uint16_t x, uint16_t y, uint16_t l) {
    for (uint16_t i = 0; i < l; i += 2) {
        u8g2_DrawPixel(S_U8G2, x + i, y);
    }
}

void vision_ui_driver_line_v_dotted_draw(uint16_t x, uint16_t y, uint16_t h) {
    for (uint16_t i = 0; i < h; i += 2) {
        u8g2_DrawPixel(S_U8G2, x, y + i);
    }
}

/* 位图：假定 bitMap 为 XBM(1bpp) 布局（和 u8g2_DrawXBM 兼容） */
void vision_ui_driver_bmp_draw(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t* bit_map) {
    u8g2_DrawXBM(S_U8G2, x, y, w, h, bit_map);
}

/* 属性 */
void vision_ui_driver_color_draw(uint8_t color) {
    /* u8g2: 0=clear, 1=set, 2=XOR (部分设备/后端可能不支持 XOR) */
    u8g2_SetDrawColor(S_U8G2, color);
}

void vision_ui_driver_font_mode_set(uint8_t mode) {
    /* 0=transparent, 1=solid */
    u8g2_SetFontMode(S_U8G2, mode ? 1 : 0);
}

void vision_ui_driver_font_direction_set(uint8_t dir) {
    /* 方向 0..3：0=左→右，1=下→上，2=右→左，3=上→下 */
    u8g2_SetFontDirection(S_U8G2, (uint8_t) (dir & 0x03));
}

void vision_ui_driver_clip_window_set(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    u8g2_SetClipWindow(S_U8G2, x0, y0, x1, y1);
}

void vision_ui_driver_clip_window_reset(void) {
    u8g2_SetMaxClipWindow(S_U8G2);
}

/* 缓冲区 */
void vision_ui_driver_buffer_clear(void) {
    u8g2_ClearBuffer(S_U8G2);
}

void vision_ui_driver_buffer_send(void) {
    u8g2_SendBuffer(S_U8G2);
}

/* 部分区域刷新（若驱动支持 tile-based 更新则调用；否则退化为全刷） */
void vision_ui_driver_buffer_area_send(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    /* u8g2_UpdateDisplayArea 以 tile(8x8) 为单位；
       只有部分驱动/后端支持；否则该调用不会生效。 */
#if U8G2_WITH_CLIP_WINDOW /* 一些构建里会定义，但不可靠；做宽松处理 */
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
    u8g2_SendBuffer(S_U8G2); /* 回退全刷 */
#endif
}

extern void* vision_ui_driver_buffer_pointer_get() {
    return U8G2_BUFFER;
}
