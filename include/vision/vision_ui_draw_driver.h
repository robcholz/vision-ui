//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//

#ifndef VISION_UI_VISION_UI_DRAW_DRIVER_H
#define VISION_UI_VISION_UI_DRAW_DRIVER_H

#include <assert.h>
#include <stdint.h>

#include "vision_ui_types.h"

typedef struct vision_ui_driver_ops_t {
    /**
     * Returns the current high-level input action from the backend.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     * @return The current mapped action for this frame.
     */
    vision_ui_action_t (*action_get)(void* context);
    /**
     * Returns a monotonic millisecond tick value.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     * @return A monotonic time value in milliseconds.
     */
    uint32_t (*ticks_ms_get)(void* context);
    /**
     * Sleeps or yields for roughly the requested number of milliseconds.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     * @param ms Approximate delay duration in milliseconds.
     */
    void (*delay)(void* context, uint32_t ms);

    /**
     * Activates a backend font for subsequent text operations.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     * @param font Font configuration to activate.
     */
    void (*font_set)(void* context, vision_ui_font_t font);
    /**
     * Returns the backend font that is currently active.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     * @return A copy of the active backend font configuration. Its `font` field may be `NULL` if no font is active yet.
     */
    vision_ui_font_t (*font_get)(void* context);
    /**
     * Draws plain text using the currently active font.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     * @param x Left position in pixels.
     * @param y Baseline position in pixels.
     * @param str Borrowed NUL-terminated plain-text string to draw.
     */
    void (*str_draw)(void* context, uint16_t x, uint16_t y, const char* str);
    /**
     * Draws UTF-8 text using the currently active font.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     * @param x Left position in pixels.
     * @param y Baseline position in pixels.
     * @param str Borrowed NUL-terminated UTF-8 string to draw.
     */
    void (*str_utf8_draw)(void* context, uint16_t x, uint16_t y, const char* str);
    /**
     * Returns the rendered width of plain text.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     * @param str Borrowed NUL-terminated plain-text string to measure.
     * @return Width in pixels.
     */
    uint16_t (*str_width_get)(void* context, const char* str);
    /**
     * Returns the rendered width of UTF-8 text.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     * @param str Borrowed NUL-terminated UTF-8 string to measure.
     * @return Width in pixels.
     */
    uint16_t (*str_utf8_width_get)(void* context, const char* str);
    /**
     * Returns the current text height for the active font.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     * @return Text height in pixels for the active backend font.
     */
    uint16_t (*str_height_get)(void* context);
    /**
     * Changes the backend text drawing mode.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     * @param mode Backend-specific text drawing mode.
     */
    void (*font_mode_set)(void* context, uint8_t mode);
    /**
     * Changes the backend text direction.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     * @param dir Backend-specific text direction value.
     */
    void (*font_direction_set)(void* context, uint8_t dir);

    /**
     * Draws a single pixel.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     * @param x X coordinate in pixels.
     * @param y Y coordinate in pixels.
     */
    void (*pixel_draw)(void* context, uint16_t x, uint16_t y);
    /**
     * Draws a circle outline.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     * @param x Circle center X coordinate in pixels.
     * @param y Circle center Y coordinate in pixels.
     * @param r Circle radius in pixels.
     */
    void (*circle_draw)(void* context, uint16_t x, uint16_t y, uint16_t r);
    /**
     * Draws a filled circle.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     * @param x Circle center X coordinate in pixels.
     * @param y Circle center Y coordinate in pixels.
     * @param r Circle radius in pixels.
     */
    void (*disc_draw)(void* context, uint16_t x, uint16_t y, uint16_t r);
    /**
     * Draws a rounded filled rectangle.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     * @param x Left position in pixels.
     * @param y Top position in pixels.
     * @param w Width in pixels.
     * @param h Height in pixels.
     * @param r Corner radius in pixels.
     */
    void (*box_r_draw)(void* context, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r);
    /**
     * Draws a filled rectangle.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     * @param x Left position in pixels.
     * @param y Top position in pixels.
     * @param w Width in pixels.
     * @param h Height in pixels.
     */
    void (*box_draw)(void* context, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    /**
     * Draws a rectangular outline.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     * @param x Left position in pixels.
     * @param y Top position in pixels.
     * @param w Width in pixels.
     * @param h Height in pixels.
     */
    void (*frame_draw)(void* context, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    /**
     * Draws a rounded rectangular outline.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     * @param x Left position in pixels.
     * @param y Top position in pixels.
     * @param w Width in pixels.
     * @param h Height in pixels.
     * @param r Corner radius in pixels.
     */
    void (*frame_r_draw)(void* context, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r);
    /**
     * Draws a horizontal line.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     * @param x Left position in pixels.
     * @param y Y coordinate in pixels.
     * @param l Length in pixels.
     */
    void (*line_h_draw)(void* context, uint16_t x, uint16_t y, uint16_t l);
    /**
     * Draws a vertical line.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     * @param x X coordinate in pixels.
     * @param y Top position in pixels.
     * @param h Height in pixels.
     */
    void (*line_v_draw)(void* context, uint16_t x, uint16_t y, uint16_t h);
    /**
     * Draws an arbitrary line segment.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     * @param x1 Start X coordinate in pixels.
     * @param y1 Start Y coordinate in pixels.
     * @param x2 End X coordinate in pixels.
     * @param y2 End Y coordinate in pixels.
     */
    void (*line_draw)(void* context, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
    /**
     * Draws a dotted horizontal line.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     * @param x Left position in pixels.
     * @param y Y coordinate in pixels.
     * @param l Length in pixels.
     */
    void (*line_h_dotted_draw)(void* context, uint16_t x, uint16_t y, uint16_t l);
    /**
     * Draws a dotted vertical line.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     * @param x X coordinate in pixels.
     * @param y Top position in pixels.
     * @param h Height in pixels.
     */
    void (*line_v_dotted_draw)(void* context, uint16_t x, uint16_t y, uint16_t h);
    /**
     * Draws a monochrome bitmap.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     * @param x Left position in pixels.
     * @param y Top position in pixels.
     * @param w Bitmap width in pixels.
     * @param h Bitmap height in pixels.
     * @param bit_map Borrowed bitmap data pointer. The pointed-to bitmap should remain valid for the duration of the draw call.
     */
    void (*bmp_draw)(void* context, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t* bit_map);
    /**
     * Changes the current draw color.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     * @param color Backend-specific draw color value.
     */
    void (*color_draw)(void* context, uint8_t color);
    /**
     * Restricts drawing to a rectangular clip region.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     * @param x0 Clip region start X coordinate.
     * @param y0 Clip region start Y coordinate.
     * @param x1 Clip region end X coordinate.
     * @param y1 Clip region end Y coordinate.
     */
    void (*clip_window_set)(void* context, int16_t x0, int16_t y0, int16_t x1, int16_t y1);
    /**
     * Clears the current clip region.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     */
    void (*clip_window_reset)(void* context);

    /**
     * Clears the full frame buffer.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     */
    void (*buffer_clear)(void* context);
    /**
     * Presents the current frame buffer.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     */
    void (*buffer_send)(void* context);
    /**
     * Presents only a sub-rectangle of the frame buffer if the backend supports it.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     * @param x Left position of the dirty rectangle.
     * @param y Top position of the dirty rectangle.
     * @param w Width of the dirty rectangle.
     * @param h Height of the dirty rectangle.
     */
    void (*buffer_area_send)(void* context, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    /**
     * Returns a readable pointer to the full-size frame buffer used by transition and blur effects.
     *
     * @param context Backend-owned driver context installed on the UI instance. Must not be `NULL`.
     * @return A pointer to the readable full-size frame buffer when the backend exposes one. Backends that cannot expose a
     * readable full buffer may return `NULL` or another unusable pointer for transition/blur effects, in which case those
     * effects are not fully supported.
     *
     * @attention this should be a full size buffer
     */
    void* (*buffer_pointer_get)(void* context);
} vision_ui_driver_ops_t;

static inline void vision_ui_init_driver(vision_ui_t* ui, const vision_ui_driver_t* driver) {
    assert(ui != NULL);
    assert(driver != NULL);
    assert(driver->ops != NULL);
    ui->driver = *driver;
}

static inline vision_ui_action_t vision_ui_driver_action_get(const vision_ui_t* ui) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    assert(ui->driver.ops->action_get != NULL);
    return ui->driver.ops->action_get(ui->driver.context);
}

static inline uint32_t vision_ui_driver_ticks_ms_get(const vision_ui_t* ui) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    assert(ui->driver.ops->ticks_ms_get != NULL);
    return ui->driver.ops->ticks_ms_get(ui->driver.context);
}

static inline void vision_ui_driver_delay(const vision_ui_t* ui, const uint32_t ms) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    assert(ui->driver.ops->delay != NULL);
    ui->driver.ops->delay(ui->driver.context, ms);
}

static inline void vision_ui_driver_font_set(vision_ui_t* ui, const vision_ui_font_t font) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    assert(ui->driver.ops->font_set != NULL);
    ui->driver.ops->font_set(ui->driver.context, font);
}

static inline vision_ui_font_t vision_ui_driver_font_get(const vision_ui_t* ui) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    assert(ui->driver.ops->font_get != NULL);
    return ui->driver.ops->font_get(ui->driver.context);
}

static inline void vision_ui_driver_str_draw(const vision_ui_t* ui, const uint16_t x, const uint16_t y, const char* str) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    assert(ui->driver.ops->str_draw != NULL);
    ui->driver.ops->str_draw(ui->driver.context, x, y, str);
}

static inline void vision_ui_driver_str_utf8_draw(
        const vision_ui_t* ui,
        const uint16_t x,
        const uint16_t y,
        const char* str
) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    assert(ui->driver.ops->str_utf8_draw != NULL);
    ui->driver.ops->str_utf8_draw(ui->driver.context, x, y, str);
}

static inline uint16_t vision_ui_driver_str_width_get(const vision_ui_t* ui, const char* str) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    assert(ui->driver.ops->str_width_get != NULL);
    return ui->driver.ops->str_width_get(ui->driver.context, str);
}

static inline uint16_t vision_ui_driver_str_utf8_width_get(const vision_ui_t* ui, const char* str) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    assert(ui->driver.ops->str_utf8_width_get != NULL);
    return ui->driver.ops->str_utf8_width_get(ui->driver.context, str);
}

static inline uint16_t vision_ui_driver_str_height_get(const vision_ui_t* ui) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    assert(ui->driver.ops->str_height_get != NULL);
    return ui->driver.ops->str_height_get(ui->driver.context);
}

static inline void vision_ui_driver_font_mode_set(const vision_ui_t* ui, const uint8_t mode) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    assert(ui->driver.ops->font_mode_set != NULL);
    ui->driver.ops->font_mode_set(ui->driver.context, mode);
}

static inline void vision_ui_driver_font_direction_set(const vision_ui_t* ui, const uint8_t dir) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    assert(ui->driver.ops->font_direction_set != NULL);
    ui->driver.ops->font_direction_set(ui->driver.context, dir);
}

static inline void vision_ui_driver_pixel_draw(const vision_ui_t* ui, const uint16_t x, const uint16_t y) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    assert(ui->driver.ops->pixel_draw != NULL);
    ui->driver.ops->pixel_draw(ui->driver.context, x, y);
}

static inline void vision_ui_driver_circle_draw(const vision_ui_t* ui, const uint16_t x, const uint16_t y, const uint16_t r) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    assert(ui->driver.ops->circle_draw != NULL);
    ui->driver.ops->circle_draw(ui->driver.context, x, y, r);
}

static inline void vision_ui_driver_disc_draw(const vision_ui_t* ui, const uint16_t x, const uint16_t y, const uint16_t r) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    assert(ui->driver.ops->disc_draw != NULL);
    ui->driver.ops->disc_draw(ui->driver.context, x, y, r);
}

static inline void vision_ui_driver_box_r_draw(
        const vision_ui_t* ui,
        const uint16_t x,
        const uint16_t y,
        const uint16_t w,
        const uint16_t h,
        const uint16_t r
) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    assert(ui->driver.ops->box_r_draw != NULL);
    ui->driver.ops->box_r_draw(ui->driver.context, x, y, w, h, r);
}

static inline void vision_ui_driver_box_draw(const vision_ui_t* ui, const uint16_t x, const uint16_t y, const uint16_t w, const uint16_t h) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    assert(ui->driver.ops->box_draw != NULL);
    ui->driver.ops->box_draw(ui->driver.context, x, y, w, h);
}

static inline void vision_ui_driver_frame_draw(const vision_ui_t* ui, const uint16_t x, const uint16_t y, const uint16_t w, const uint16_t h) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    assert(ui->driver.ops->frame_draw != NULL);
    ui->driver.ops->frame_draw(ui->driver.context, x, y, w, h);
}

static inline void vision_ui_driver_frame_r_draw(
        const vision_ui_t* ui,
        const uint16_t x,
        const uint16_t y,
        const uint16_t w,
        const uint16_t h,
        const uint16_t r
) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    assert(ui->driver.ops->frame_r_draw != NULL);
    ui->driver.ops->frame_r_draw(ui->driver.context, x, y, w, h, r);
}

static inline void vision_ui_driver_line_h_draw(const vision_ui_t* ui, const uint16_t x, const uint16_t y, const uint16_t l) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    assert(ui->driver.ops->line_h_draw != NULL);
    ui->driver.ops->line_h_draw(ui->driver.context, x, y, l);
}

static inline void vision_ui_driver_line_v_draw(const vision_ui_t* ui, const uint16_t x, const uint16_t y, const uint16_t h) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    assert(ui->driver.ops->line_v_draw != NULL);
    ui->driver.ops->line_v_draw(ui->driver.context, x, y, h);
}

static inline void vision_ui_driver_line_draw(
        const vision_ui_t* ui,
        const uint16_t x1,
        const uint16_t y1,
        const uint16_t x2,
        const uint16_t y2
) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    assert(ui->driver.ops->line_draw != NULL);
    ui->driver.ops->line_draw(ui->driver.context, x1, y1, x2, y2);
}

static inline void vision_ui_driver_line_h_dotted_draw(const vision_ui_t* ui, const uint16_t x, const uint16_t y, const uint16_t l) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    assert(ui->driver.ops->line_h_dotted_draw != NULL);
    ui->driver.ops->line_h_dotted_draw(ui->driver.context, x, y, l);
}

static inline void vision_ui_driver_line_v_dotted_draw(const vision_ui_t* ui, const uint16_t x, const uint16_t y, const uint16_t h) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    assert(ui->driver.ops->line_v_dotted_draw != NULL);
    ui->driver.ops->line_v_dotted_draw(ui->driver.context, x, y, h);
}

static inline void vision_ui_driver_bmp_draw(
        const vision_ui_t* ui,
        const uint16_t x,
        const uint16_t y,
        const uint16_t w,
        const uint16_t h,
        const uint8_t* bit_map
) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    assert(ui->driver.ops->bmp_draw != NULL);
    ui->driver.ops->bmp_draw(ui->driver.context, x, y, w, h, bit_map);
}

static inline void vision_ui_driver_color_draw(const vision_ui_t* ui, const uint8_t color) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    assert(ui->driver.ops->color_draw != NULL);
    ui->driver.ops->color_draw(ui->driver.context, color);
}

static inline void vision_ui_driver_clip_window_set(
        const vision_ui_t* ui,
        const int16_t x0,
        const int16_t y0,
        const int16_t x1,
        const int16_t y1
) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    assert(ui->driver.ops->clip_window_set != NULL);
    ui->driver.ops->clip_window_set(ui->driver.context, x0, y0, x1, y1);
}

static inline void vision_ui_driver_clip_window_reset(const vision_ui_t* ui) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    assert(ui->driver.ops->clip_window_reset != NULL);
    ui->driver.ops->clip_window_reset(ui->driver.context);
}

static inline void vision_ui_driver_buffer_clear(const vision_ui_t* ui) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    assert(ui->driver.ops->buffer_clear != NULL);
    ui->driver.ops->buffer_clear(ui->driver.context);
}

static inline void vision_ui_driver_buffer_send(const vision_ui_t* ui) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    assert(ui->driver.ops->buffer_send != NULL);
    ui->driver.ops->buffer_send(ui->driver.context);
}

static inline void vision_ui_driver_buffer_area_send(
        const vision_ui_t* ui,
        const uint16_t x,
        const uint16_t y,
        const uint16_t w,
        const uint16_t h
) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    if (ui->driver.ops->buffer_area_send != NULL) {
        ui->driver.ops->buffer_area_send(ui->driver.context, x, y, w, h);
        return;
    }
    assert(ui->driver.ops->buffer_send != NULL);
    ui->driver.ops->buffer_send(ui->driver.context);
}

static inline void* vision_ui_driver_buffer_pointer_get(const vision_ui_t* ui) {
    assert(ui != NULL);
    assert(ui->driver.ops != NULL);
    if (ui->driver.ops->buffer_pointer_get == NULL) {
        return NULL;
    }
    return ui->driver.ops->buffer_pointer_get(ui->driver.context);
}

#endif // VISION_UI_VISION_UI_DRAW_DRIVER_H
