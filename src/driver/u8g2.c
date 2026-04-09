//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//

#include "u8g2.h"

#include "vision/vision_ui_core.h"

#include <u8g2.h>

#include <SDL_events.h>
#include <SDL_timer.h>

static u8g2_t* S_U8G2 = NULL;

uint8_t U8G2_BUFFER[VISION_UI_BUFFER_SIZE];

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
            return UiActionGoPrev;
        case SDLK_DOWN:
            return UiActionGoNext;
        case SDLK_ESCAPE:
            return UiActionExit;
        case SDLK_SPACE:
            return UiActionEnter;
        default:
            return UiActionNone;
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

static int8_t STR_TOP = 0;
static int8_t STR_BOTTOM = 0;
static vision_ui_font_t CURRENT;

void vision_ui_driver_font_set(const vision_ui_font_t font) {
    u8g2_SetFont(S_U8G2, font.font);
    CURRENT = font;
    STR_TOP = font.top_compensation;
    STR_BOTTOM = font.bottom_compensation;
}

vision_ui_font_t vision_ui_driver_font_get() {
    return CURRENT;
}

void vision_ui_driver_str_draw(uint16_t x, const uint16_t y, const char* str) {
    u8g2_DrawStr(S_U8G2, x, y - STR_BOTTOM, str);
}

void vision_ui_driver_str_utf8_draw(uint16_t x, uint16_t y, const char* str) {
    u8g2_DrawUTF8(S_U8G2, x, y - STR_BOTTOM, str);
}

uint16_t vision_ui_driver_str_width_get(const char* str) {
    return (uint16_t) u8g2_GetStrWidth(S_U8G2, str);
}

uint16_t vision_ui_driver_str_utf8_width_get(const char* str) {
    return (uint16_t) u8g2_GetUTF8Width(S_U8G2, str);
}

uint16_t vision_ui_driver_str_height_get() {
    int16_t h = u8g2_GetMaxCharHeight(S_U8G2);
    if (h < 0) {
        h = 0;
    }
    return (uint16_t) h + STR_TOP;
}

/* Primitives */
void vision_ui_driver_pixel_draw(uint16_t x, uint16_t y) {
    u8g2_DrawPixel(S_U8G2, x, y);
}

void vision_ui_driver_circle_draw(uint16_t x, uint16_t y, uint16_t r) {
    /* u8g2_DrawCircle draws an outline; use u8g2_DrawDisc for a filled circle. */
    u8g2_DrawCircle(S_U8G2, x, y, r, U8G2_DRAW_ALL);
}

void vision_ui_driver_disc_draw(uint16_t x, uint16_t y, uint16_t r) {
    u8g2_DrawDisc(S_U8G2, x, y, r, U8G2_DRAW_ALL);
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

/* Dotted lines: simple 1-pixel spacing (adjust dash/gap if needed). */
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

/* Bitmap: assumes bitMap uses XBM (1bpp) layout compatible with u8g2_DrawXBM. */
void vision_ui_driver_bmp_draw(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t* bit_map) {
    u8g2_DrawXBM(S_U8G2, x, y, w, h, bit_map);
}

/* Attributes */
void vision_ui_driver_color_draw(uint8_t color) {
    /* u8g2: 0=clear, 1=set, 2=XOR (some devices/backends may not support XOR). */
    u8g2_SetDrawColor(S_U8G2, color);
}

void vision_ui_driver_font_mode_set(uint8_t mode) {
    /* 0=transparent, 1=solid */
    u8g2_SetFontMode(S_U8G2, mode ? 1 : 0);
}

void vision_ui_driver_font_direction_set(uint8_t dir) {
    /* Direction 0..3: 0=left->right, 1=bottom->top, 2=right->left, 3=top->bottom. */
    u8g2_SetFontDirection(S_U8G2, (uint8_t) (dir & 0x03));
}

void vision_ui_driver_clip_window_set(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    u8g2_SetClipWindow(S_U8G2, x0, y0, x1, y1);
}

void vision_ui_driver_clip_window_reset(void) {
    u8g2_SetMaxClipWindow(S_U8G2);
}

/* Buffer */
void vision_ui_driver_buffer_clear(void) {
    u8g2_ClearBuffer(S_U8G2);
}

void vision_ui_driver_buffer_send(void) {
    u8g2_SendBuffer(S_U8G2);
}

/* Partial refresh (uses tile-based updates when supported, otherwise falls back to full refresh). */
void vision_ui_driver_buffer_area_send(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    /* u8g2_UpdateDisplayArea works in tile (8x8) units.
       Only some drivers/backends support it; otherwise the call has no effect. */
#if U8G2_WITH_CLIP_WINDOW /* Some builds define this, but it is not reliable, so handle it loosely. */
    uint16_t x_end = x + w - 1;
    uint16_t y_end = y + h - 1;
    /* Convert pixels to tiles using floor/ceiling rounding. */
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
    u8g2_SendBuffer(S_U8G2); /* Fallback to a full refresh. */
#endif
}

extern void* vision_ui_driver_buffer_pointer_get() {
    return U8G2_BUFFER;
}
