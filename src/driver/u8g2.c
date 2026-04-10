//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//

#include "u8g2.h"

#include "vision/vision_ui.h"

#include <u8g2.h>

#include <SDL_events.h>
#include <SDL_timer.h>
#include <assert.h>

uint8_t U8G2_BUFFER[VISION_UI_BUFFER_SIZE];

static u8g2_t* vision_ui_u8g2_get(const vision_ui_t* ui) {
    assert(ui != NULL);
    return ui->driver;
}

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

vision_ui_action_t vision_ui_driver_action_get(const vision_ui_t* ui) {
    (void) ui;
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

uint32_t vision_ui_driver_ticks_ms_get(const vision_ui_t* ui) {
    (void) ui;
    return SDL_GetTicks();
}

void vision_ui_driver_delay(const vision_ui_t* ui, const uint32_t ms) {
    (void) ui;
    SDL_Delay(ms);
}

void vision_ui_driver_bind(vision_ui_t* ui, void* driver) {
    ui->driver = driver;
}

void vision_ui_driver_font_set(vision_ui_t* ui, const vision_ui_font_t font) {
    if (ui == NULL) {
        return;
    }
    u8g2_SetFont(vision_ui_u8g2_get(ui), font.font);
    ui->driver_current_font = font;
    ui->driver_str_top = font.top_compensation;
    ui->driver_str_bottom = font.bottom_compensation;
}

vision_ui_font_t vision_ui_driver_font_get(const vision_ui_t* ui) {
    return ui->driver_current_font;
}

void vision_ui_driver_str_draw(const vision_ui_t* ui, const uint16_t x, const uint16_t y, const char* str) {
    u8g2_DrawStr(vision_ui_u8g2_get(ui), x, y - ui->driver_str_bottom, str);
}

void vision_ui_driver_str_utf8_draw(const vision_ui_t* ui, const uint16_t x, const uint16_t y, const char* str) {
    u8g2_DrawUTF8(vision_ui_u8g2_get(ui), x, y - ui->driver_str_bottom, str);
}

uint16_t vision_ui_driver_str_width_get(const vision_ui_t* ui, const char* str) {
    return u8g2_GetStrWidth(vision_ui_u8g2_get(ui), str);
}

uint16_t vision_ui_driver_str_utf8_width_get(const vision_ui_t* ui, const char* str) {
    return u8g2_GetUTF8Width(vision_ui_u8g2_get(ui), str);
}

uint16_t vision_ui_driver_str_height_get(const vision_ui_t* ui) {
    int16_t h = u8g2_GetMaxCharHeight(vision_ui_u8g2_get(ui));
    if (h < 0) {
        h = 0;
    }
    return (uint16_t) h + ui->driver_str_top;
}

/* Primitives */
void vision_ui_driver_pixel_draw(const vision_ui_t* ui, const uint16_t x, const uint16_t y) {
    u8g2_DrawPixel(vision_ui_u8g2_get(ui), x, y);
}

void vision_ui_driver_circle_draw(const vision_ui_t* ui, const uint16_t x, const uint16_t y, const uint16_t r) {
    /* u8g2_DrawCircle draws an outline; use u8g2_DrawDisc for a filled circle. */
    u8g2_DrawCircle(vision_ui_u8g2_get(ui), x, y, r, U8G2_DRAW_ALL);
}

void vision_ui_driver_disc_draw(const vision_ui_t* ui, const uint16_t x, const uint16_t y, const uint16_t r) {
    u8g2_DrawDisc(vision_ui_u8g2_get(ui), x, y, r, U8G2_DRAW_ALL);
}

void vision_ui_driver_box_r_draw(
        const vision_ui_t* ui,
        const uint16_t x,
        const uint16_t y,
        const uint16_t w,
        const uint16_t h,
        const uint16_t r
) {
    u8g2_DrawRBox(vision_ui_u8g2_get(ui), x, y, w, h, r);
}

void vision_ui_driver_box_draw(
        const vision_ui_t* ui,
        const uint16_t x,
        const uint16_t y,
        const uint16_t w,
        const uint16_t h
) {
    u8g2_DrawBox(vision_ui_u8g2_get(ui), x, y, w, h);
}

void vision_ui_driver_frame_draw(
        const vision_ui_t* ui,
        const uint16_t x,
        const uint16_t y,
        const uint16_t w,
        const uint16_t h
) {
    u8g2_DrawFrame(vision_ui_u8g2_get(ui), x, y, w, h);
}

void vision_ui_driver_frame_r_draw(
        const vision_ui_t* ui,
        const uint16_t x,
        const uint16_t y,
        const uint16_t w,
        const uint16_t h,
        const uint16_t r
) {
    u8g2_DrawRFrame(vision_ui_u8g2_get(ui), x, y, w, h, r);
}

void vision_ui_driver_line_h_draw(const vision_ui_t* ui, const uint16_t x, const uint16_t y, const uint16_t l) {
    u8g2_DrawHLine(vision_ui_u8g2_get(ui), x, y, l);
}

void vision_ui_driver_line_v_draw(const vision_ui_t* ui, const uint16_t x, const uint16_t y, const uint16_t h) {
    u8g2_DrawVLine(vision_ui_u8g2_get(ui), x, y, h);
}

void vision_ui_driver_line_draw(
        const vision_ui_t* ui,
        const uint16_t x1,
        const uint16_t y1,
        const uint16_t x2,
        const uint16_t y2
) {
    u8g2_DrawLine(vision_ui_u8g2_get(ui), x1, y1, x2, y2);
}

/* Dotted lines: simple 1-pixel spacing (adjust dash/gap if needed). */
void vision_ui_driver_line_h_dotted_draw(const vision_ui_t* ui, const uint16_t x, const uint16_t y, const uint16_t l) {
    for (uint16_t i = 0; i < l; i += 2) {
        u8g2_DrawPixel(vision_ui_u8g2_get(ui), x + i, y);
    }
}

void vision_ui_driver_line_v_dotted_draw(const vision_ui_t* ui, const uint16_t x, const uint16_t y, const uint16_t h) {
    for (uint16_t i = 0; i < h; i += 2) {
        u8g2_DrawPixel(vision_ui_u8g2_get(ui), x, y + i);
    }
}

/* Bitmap: assumes bitMap uses XBM (1bpp) layout compatible with u8g2_DrawXBM. */
void vision_ui_driver_bmp_draw(
        const vision_ui_t* ui,
        const uint16_t x,
        const uint16_t y,
        const uint16_t w,
        const uint16_t h,
        const uint8_t* bit_map
) {
    u8g2_DrawXBM(vision_ui_u8g2_get(ui), x, y, w, h, bit_map);
}

/* Attributes */
void vision_ui_driver_color_draw(const vision_ui_t* ui, const uint8_t color) {
    /* u8g2: 0=clear, 1=set, 2=XOR (some devices/backends may not support XOR). */
    u8g2_SetDrawColor(vision_ui_u8g2_get(ui), color);
}

void vision_ui_driver_font_mode_set(const vision_ui_t* ui, const uint8_t mode) {
    /* 0=transparent, 1=solid */
    u8g2_SetFontMode(vision_ui_u8g2_get(ui), mode ? 1 : 0);
}

void vision_ui_driver_font_direction_set(const vision_ui_t* ui, const uint8_t dir) {
    /* Direction 0..3: 0=left->right, 1=bottom->top, 2=right->left, 3=top->bottom. */
    u8g2_SetFontDirection(vision_ui_u8g2_get(ui), (uint8_t) (dir & 0x03));
}

void vision_ui_driver_clip_window_set(
        const vision_ui_t* ui,
        const int16_t x0,
        const int16_t y0,
        const int16_t x1,
        const int16_t y1
) {
    u8g2_SetClipWindow(vision_ui_u8g2_get(ui), x0, y0, x1, y1);
}

void vision_ui_driver_clip_window_reset(const vision_ui_t* ui) {
    u8g2_SetMaxClipWindow(vision_ui_u8g2_get(ui));
}

/* Buffer */
void vision_ui_driver_buffer_clear(const vision_ui_t* ui) {
    u8g2_ClearBuffer(vision_ui_u8g2_get(ui));
}

void vision_ui_driver_buffer_send(const vision_ui_t* ui) {
    u8g2_SendBuffer(vision_ui_u8g2_get(ui));
}

/* Partial refresh (uses tile-based updates when supported, otherwise falls back to full refresh). */
void vision_ui_driver_buffer_area_send(
        const vision_ui_t* ui,
        const uint16_t x,
        const uint16_t y,
        const uint16_t w,
        const uint16_t h
) {
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
    u8g2_UpdateDisplayArea(vision_ui_u8g2_get(ui), tx, ty, tw, th);
#else
    (void) x;
    (void) y;
    (void) w;
    (void) h;
    u8g2_SendBuffer(vision_ui_u8g2_get(ui)); /* Fallback to a full refresh. */
#endif
}

extern void* vision_ui_driver_buffer_pointer_get(const vision_ui_t* ui) {
    (void) ui;
    return U8G2_BUFFER;
}
