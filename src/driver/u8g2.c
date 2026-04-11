//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//

#include "u8g2.h"

#include <SDL_events.h>
#include <SDL_timer.h>
#include <assert.h>

uint8_t U8G2_BUFFER[VISION_UI_BUFFER_SIZE];

static vision_ui_u8g2_driver_t* vision_ui_u8g2_get(void* context) {
    assert(context != NULL);
    return (vision_ui_u8g2_driver_t*) context;
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

static vision_ui_action_t vision_ui_u8g2_action_get(void* context) {
    (void) context;
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

static uint32_t vision_ui_u8g2_ticks_ms_get(void* context) {
    (void) context;
    return SDL_GetTicks();
}

static void vision_ui_u8g2_delay(void* context, const uint32_t ms) {
    (void) context;
    SDL_Delay(ms);
}

void vision_ui_u8g2_driver_init(vision_ui_u8g2_driver_t* driver, u8g2_t* handle) {
    assert(driver != NULL);
    driver->handle = handle;
    driver->current_font = (vision_ui_font_t){0};
    driver->str_top = 0;
    driver->str_bottom = 0;
}

vision_ui_driver_t vision_ui_u8g2_driver_descriptor(vision_ui_u8g2_driver_t* driver) {
    vision_ui_driver_t descriptor = {
            .context = driver,
            .ops = &VISION_UI_U8G2_DRIVER_OPS,
    };
    return descriptor;
}

static void vision_ui_u8g2_font_set(void* context, const vision_ui_font_t font) {
    vision_ui_u8g2_driver_t* driver = vision_ui_u8g2_get(context);
    u8g2_SetFont(driver->handle, font.font);
    driver->current_font = font;
    driver->str_top = font.top_compensation;
    driver->str_bottom = font.bottom_compensation;
}

static vision_ui_font_t vision_ui_u8g2_font_get(void* context) {
    return vision_ui_u8g2_get(context)->current_font;
}

static void vision_ui_u8g2_str_draw(void* context, const uint16_t x, const uint16_t y, const char* str) {
    vision_ui_u8g2_driver_t* driver = vision_ui_u8g2_get(context);
    u8g2_DrawStr(driver->handle, x, y - driver->str_bottom, str);
}

static void vision_ui_u8g2_str_utf8_draw(void* context, const uint16_t x, const uint16_t y, const char* str) {
    vision_ui_u8g2_driver_t* driver = vision_ui_u8g2_get(context);
    u8g2_DrawUTF8(driver->handle, x, y - driver->str_bottom, str);
}

static uint16_t vision_ui_u8g2_str_width_get(void* context, const char* str) {
    return u8g2_GetStrWidth(vision_ui_u8g2_get(context)->handle, str);
}

static uint16_t vision_ui_u8g2_str_utf8_width_get(void* context, const char* str) {
    return u8g2_GetUTF8Width(vision_ui_u8g2_get(context)->handle, str);
}

static uint16_t vision_ui_u8g2_str_height_get(void* context) {
    vision_ui_u8g2_driver_t* driver = vision_ui_u8g2_get(context);
    int16_t h = u8g2_GetMaxCharHeight(driver->handle);
    if (h < 0) {
        h = 0;
    }
    return (uint16_t) h + driver->str_top;
}

/* Primitives */
static void vision_ui_u8g2_pixel_draw(void* context, const uint16_t x, const uint16_t y) {
    u8g2_DrawPixel(vision_ui_u8g2_get(context)->handle, x, y);
}

static void vision_ui_u8g2_circle_draw(void* context, const uint16_t x, const uint16_t y, const uint16_t r) {
    /* u8g2_DrawCircle draws an outline; use u8g2_DrawDisc for a filled circle. */
    u8g2_DrawCircle(vision_ui_u8g2_get(context)->handle, x, y, r, U8G2_DRAW_ALL);
}

static void vision_ui_u8g2_disc_draw(void* context, const uint16_t x, const uint16_t y, const uint16_t r) {
    u8g2_DrawDisc(vision_ui_u8g2_get(context)->handle, x, y, r, U8G2_DRAW_ALL);
}

static void vision_ui_u8g2_box_r_draw(
        void* context,
        const uint16_t x,
        const uint16_t y,
        const uint16_t w,
        const uint16_t h,
        const uint16_t r
) {
    u8g2_DrawRBox(vision_ui_u8g2_get(context)->handle, x, y, w, h, r);
}

static void vision_ui_u8g2_box_draw(
        void* context,
        const uint16_t x,
        const uint16_t y,
        const uint16_t w,
        const uint16_t h
) {
    u8g2_DrawBox(vision_ui_u8g2_get(context)->handle, x, y, w, h);
}

static void vision_ui_u8g2_frame_draw(
        void* context,
        const uint16_t x,
        const uint16_t y,
        const uint16_t w,
        const uint16_t h
) {
    u8g2_DrawFrame(vision_ui_u8g2_get(context)->handle, x, y, w, h);
}

static void vision_ui_u8g2_frame_r_draw(
        void* context,
        const uint16_t x,
        const uint16_t y,
        const uint16_t w,
        const uint16_t h,
        const uint16_t r
) {
    u8g2_DrawRFrame(vision_ui_u8g2_get(context)->handle, x, y, w, h, r);
}

static void vision_ui_u8g2_line_h_draw(void* context, const uint16_t x, const uint16_t y, const uint16_t l) {
    u8g2_DrawHLine(vision_ui_u8g2_get(context)->handle, x, y, l);
}

static void vision_ui_u8g2_line_v_draw(void* context, const uint16_t x, const uint16_t y, const uint16_t h) {
    u8g2_DrawVLine(vision_ui_u8g2_get(context)->handle, x, y, h);
}

static void vision_ui_u8g2_line_draw(
        void* context,
        const uint16_t x1,
        const uint16_t y1,
        const uint16_t x2,
        const uint16_t y2
) {
    u8g2_DrawLine(vision_ui_u8g2_get(context)->handle, x1, y1, x2, y2);
}

/* Dotted lines: simple 1-pixel spacing (adjust dash/gap if needed). */
static void vision_ui_u8g2_line_h_dotted_draw(void* context, const uint16_t x, const uint16_t y, const uint16_t l) {
    for (uint16_t i = 0; i < l; i += 2) {
        u8g2_DrawPixel(vision_ui_u8g2_get(context)->handle, x + i, y);
    }
}

static void vision_ui_u8g2_line_v_dotted_draw(void* context, const uint16_t x, const uint16_t y, const uint16_t h) {
    for (uint16_t i = 0; i < h; i += 2) {
        u8g2_DrawPixel(vision_ui_u8g2_get(context)->handle, x, y + i);
    }
}

/* Bitmap: assumes bitMap uses XBM (1bpp) layout compatible with u8g2_DrawXBM. */
static void vision_ui_u8g2_bmp_draw(
        void* context,
        const uint16_t x,
        const uint16_t y,
        const uint16_t w,
        const uint16_t h,
        const uint8_t* bit_map
) {
    u8g2_DrawXBM(vision_ui_u8g2_get(context)->handle, x, y, w, h, bit_map);
}

/* Attributes */
static void vision_ui_u8g2_color_draw(void* context, const uint8_t color) {
    /* u8g2: 0=clear, 1=set, 2=XOR (some devices/backends may not support XOR). */
    u8g2_SetDrawColor(vision_ui_u8g2_get(context)->handle, color);
}

static void vision_ui_u8g2_font_mode_set(void* context, const uint8_t mode) {
    /* 0=transparent, 1=solid */
    u8g2_SetFontMode(vision_ui_u8g2_get(context)->handle, mode ? 1 : 0);
}

static void vision_ui_u8g2_font_direction_set(void* context, const uint8_t dir) {
    /* Direction 0..3: 0=left->right, 1=bottom->top, 2=right->left, 3=top->bottom. */
    u8g2_SetFontDirection(vision_ui_u8g2_get(context)->handle, (uint8_t) (dir & 0x03));
}

static void vision_ui_u8g2_clip_window_set(
        void* context,
        const int16_t x0,
        const int16_t y0,
        const int16_t x1,
        const int16_t y1
) {
    u8g2_SetClipWindow(vision_ui_u8g2_get(context)->handle, x0, y0, x1, y1);
}

static void vision_ui_u8g2_clip_window_reset(void* context) {
    u8g2_SetMaxClipWindow(vision_ui_u8g2_get(context)->handle);
}

/* Buffer */
static void vision_ui_u8g2_buffer_clear(void* context) {
    u8g2_ClearBuffer(vision_ui_u8g2_get(context)->handle);
}

static void vision_ui_u8g2_buffer_send(void* context) {
    u8g2_SendBuffer(vision_ui_u8g2_get(context)->handle);
}

/* Partial refresh (uses tile-based updates when supported, otherwise falls back to full refresh). */
static void vision_ui_u8g2_buffer_area_send(
        void* context,
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
    u8g2_UpdateDisplayArea(vision_ui_u8g2_get(context)->handle, tx, ty, tw, th);
#else
    (void) x;
    (void) y;
    (void) w;
    (void) h;
    u8g2_SendBuffer(vision_ui_u8g2_get(context)->handle); /* Fallback to a full refresh. */
#endif
}

static void* vision_ui_u8g2_buffer_pointer_get(void* context) {
    (void) context;
    return U8G2_BUFFER;
}

const vision_ui_driver_ops_t VISION_UI_U8G2_DRIVER_OPS = {
        .action_get = vision_ui_u8g2_action_get,
        .ticks_ms_get = vision_ui_u8g2_ticks_ms_get,
        .delay = vision_ui_u8g2_delay,
        .font_set = vision_ui_u8g2_font_set,
        .font_get = vision_ui_u8g2_font_get,
        .str_draw = vision_ui_u8g2_str_draw,
        .str_utf8_draw = vision_ui_u8g2_str_utf8_draw,
        .str_width_get = vision_ui_u8g2_str_width_get,
        .str_utf8_width_get = vision_ui_u8g2_str_utf8_width_get,
        .str_height_get = vision_ui_u8g2_str_height_get,
        .pixel_draw = vision_ui_u8g2_pixel_draw,
        .circle_draw = vision_ui_u8g2_circle_draw,
        .disc_draw = vision_ui_u8g2_disc_draw,
        .box_r_draw = vision_ui_u8g2_box_r_draw,
        .box_draw = vision_ui_u8g2_box_draw,
        .frame_draw = vision_ui_u8g2_frame_draw,
        .frame_r_draw = vision_ui_u8g2_frame_r_draw,
        .line_h_draw = vision_ui_u8g2_line_h_draw,
        .line_v_draw = vision_ui_u8g2_line_v_draw,
        .line_draw = vision_ui_u8g2_line_draw,
        .line_h_dotted_draw = vision_ui_u8g2_line_h_dotted_draw,
        .line_v_dotted_draw = vision_ui_u8g2_line_v_dotted_draw,
        .bmp_draw = vision_ui_u8g2_bmp_draw,
        .color_draw = vision_ui_u8g2_color_draw,
        .font_mode_set = vision_ui_u8g2_font_mode_set,
        .font_direction_set = vision_ui_u8g2_font_direction_set,
        .clip_window_set = vision_ui_u8g2_clip_window_set,
        .clip_window_reset = vision_ui_u8g2_clip_window_reset,
        .buffer_clear = vision_ui_u8g2_buffer_clear,
        .buffer_send = vision_ui_u8g2_buffer_send,
        .buffer_area_send = vision_ui_u8g2_buffer_area_send,
        .buffer_pointer_get = vision_ui_u8g2_buffer_pointer_get,
};
