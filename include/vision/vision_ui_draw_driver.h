//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//

#ifndef VISION_UI_VISION_UI_DRAW_DRIVER_H
#define VISION_UI_VISION_UI_DRAW_DRIVER_H

#include <stdint.h>

#include "vision_ui_types.h"

/**
 * Returns the current high-level input action from the backend.
 *
 * @param ui UI instance whose driver state should be queried. Must not be `NULL`.
 * @return The current mapped action for this frame.
 */
extern vision_ui_action_t vision_ui_driver_action_get(const vision_ui_t* ui);

/**
 * Returns a monotonic millisecond tick value.
 *
 * @param ui UI instance whose driver state should be queried. Must not be `NULL`.
 * @return A monotonic time value in milliseconds.
 */
extern uint32_t vision_ui_driver_ticks_ms_get(const vision_ui_t* ui);

/**
 * Sleeps or yields for roughly the requested number of milliseconds.
 *
 * @param ui UI instance whose driver should perform the delay. Must not be `NULL`.
 * @param ms Approximate delay duration in milliseconds.
 */
extern void vision_ui_driver_delay(const vision_ui_t* ui, uint32_t ms);

/**
 * Stores the backend driver handle on the UI instance.
 *
 * @param ui UI instance to configure. Must not be `NULL`.
 * @param driver Backend-specific handle to store. This pointer is borrowed, not copied.
 */
extern void vision_ui_driver_bind(vision_ui_t* ui, void* driver);

/**
 * Activates a backend font for subsequent text operations.
 *
 * @param ui UI instance whose backend font should change. Must not be `NULL`.
 * @param font Font configuration to activate.
 */
extern void vision_ui_driver_font_set(vision_ui_t* ui, vision_ui_font_t font);

/**
 * Returns the backend font that is currently active.
 *
 * @param ui UI instance whose backend font should be queried. Must not be `NULL`.
 * @return A copy of the active backend font configuration. Its `font` field may be `NULL` if no font is active yet.
 */
extern vision_ui_font_t vision_ui_driver_font_get(const vision_ui_t* ui);

/**
 * Draws plain text using the currently active font.
 *
 * @param ui UI instance whose driver should draw the text. Must not be `NULL`.
 * @param x Left position in pixels.
 * @param y Baseline position in pixels.
 * @param str Borrowed NUL-terminated plain-text string to draw.
 */
extern void vision_ui_driver_str_draw(const vision_ui_t* ui, uint16_t x, uint16_t y, const char* str);

/**
 * Draws UTF-8 text using the currently active font.
 *
 * @param ui UI instance whose driver should draw the text. Must not be `NULL`.
 * @param x Left position in pixels.
 * @param y Baseline position in pixels.
 * @param str Borrowed NUL-terminated UTF-8 string to draw.
 */
extern void vision_ui_driver_str_utf8_draw(const vision_ui_t* ui, uint16_t x, uint16_t y, const char* str);

/**
 * Returns the rendered width of plain text.
 *
 * @param ui UI instance whose driver should measure the text. Must not be `NULL`.
 * @param str Borrowed NUL-terminated plain-text string to measure.
 * @return Width in pixels.
 */
extern uint16_t vision_ui_driver_str_width_get(const vision_ui_t* ui, const char* str);

/**
 * Returns the rendered width of UTF-8 text.
 *
 * @param ui UI instance whose driver should measure the text. Must not be `NULL`.
 * @param str Borrowed NUL-terminated UTF-8 string to measure.
 * @return Width in pixels.
 */
extern uint16_t vision_ui_driver_str_utf8_width_get(const vision_ui_t* ui, const char* str);

/**
 * Returns the current text height for the active font.
 *
 * @param ui UI instance whose driver should be queried. Must not be `NULL`.
 * @return Text height in pixels for the active backend font.
 */
extern uint16_t vision_ui_driver_str_height_get(const vision_ui_t* ui);

/**
 * Draws a single pixel.
 *
 * @param ui UI instance whose driver should draw. Must not be `NULL`.
 * @param x X coordinate in pixels.
 * @param y Y coordinate in pixels.
 */
extern void vision_ui_driver_pixel_draw(const vision_ui_t* ui, uint16_t x, uint16_t y);

/**
 * Draws a circle outline.
 *
 * @param ui UI instance whose driver should draw. Must not be `NULL`.
 * @param x Circle center X coordinate in pixels.
 * @param y Circle center Y coordinate in pixels.
 * @param r Circle radius in pixels.
 */
extern void vision_ui_driver_circle_draw(const vision_ui_t* ui, uint16_t x, uint16_t y, uint16_t r);

/**
 * Draws a filled circle.
 *
 * @param ui UI instance whose driver should draw. Must not be `NULL`.
 * @param x Circle center X coordinate in pixels.
 * @param y Circle center Y coordinate in pixels.
 * @param r Circle radius in pixels.
 */
extern void vision_ui_driver_disc_draw(const vision_ui_t* ui, uint16_t x, uint16_t y, uint16_t r);

/**
 * Draws a rounded filled rectangle.
 *
 * @param ui UI instance whose driver should draw. Must not be `NULL`.
 * @param x Left position in pixels.
 * @param y Top position in pixels.
 * @param w Width in pixels.
 * @param h Height in pixels.
 * @param r Corner radius in pixels.
 */
extern void vision_ui_driver_box_r_draw(
        const vision_ui_t* ui,
        uint16_t x,
        uint16_t y,
        uint16_t w,
        uint16_t h,
        uint16_t r
);

/**
 * Draws a filled rectangle.
 *
 * @param ui UI instance whose driver should draw. Must not be `NULL`.
 * @param x Left position in pixels.
 * @param y Top position in pixels.
 * @param w Width in pixels.
 * @param h Height in pixels.
 */
extern void vision_ui_driver_box_draw(const vision_ui_t* ui, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

/**
 * Draws a rectangular outline.
 *
 * @param ui UI instance whose driver should draw. Must not be `NULL`.
 * @param x Left position in pixels.
 * @param y Top position in pixels.
 * @param w Width in pixels.
 * @param h Height in pixels.
 */
extern void vision_ui_driver_frame_draw(const vision_ui_t* ui, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

/**
 * Draws a rounded rectangular outline.
 *
 * @param ui UI instance whose driver should draw. Must not be `NULL`.
 * @param x Left position in pixels.
 * @param y Top position in pixels.
 * @param w Width in pixels.
 * @param h Height in pixels.
 * @param r Corner radius in pixels.
 */
extern void vision_ui_driver_frame_r_draw(
        const vision_ui_t* ui,
        uint16_t x,
        uint16_t y,
        uint16_t w,
        uint16_t h,
        uint16_t r
);

/**
 * Draws a horizontal line.
 *
 * @param ui UI instance whose driver should draw. Must not be `NULL`.
 * @param x Left position in pixels.
 * @param y Y coordinate in pixels.
 * @param l Length in pixels.
 */
extern void vision_ui_driver_line_h_draw(const vision_ui_t* ui, uint16_t x, uint16_t y, uint16_t l);

/**
 * Draws a vertical line.
 *
 * @param ui UI instance whose driver should draw. Must not be `NULL`.
 * @param x X coordinate in pixels.
 * @param y Top position in pixels.
 * @param h Height in pixels.
 */
extern void vision_ui_driver_line_v_draw(const vision_ui_t* ui, uint16_t x, uint16_t y, uint16_t h);

/**
 * Draws an arbitrary line segment.
 *
 * @param ui UI instance whose driver should draw. Must not be `NULL`.
 * @param x1 Start X coordinate in pixels.
 * @param y1 Start Y coordinate in pixels.
 * @param x2 End X coordinate in pixels.
 * @param y2 End Y coordinate in pixels.
 */
extern void vision_ui_driver_line_draw(const vision_ui_t* ui, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

/**
 * Draws a dotted horizontal line.
 *
 * @param ui UI instance whose driver should draw. Must not be `NULL`.
 * @param x Left position in pixels.
 * @param y Y coordinate in pixels.
 * @param l Length in pixels.
 */
extern void vision_ui_driver_line_h_dotted_draw(const vision_ui_t* ui, uint16_t x, uint16_t y, uint16_t l);

/**
 * Draws a dotted vertical line.
 *
 * @param ui UI instance whose driver should draw. Must not be `NULL`.
 * @param x X coordinate in pixels.
 * @param y Top position in pixels.
 * @param h Height in pixels.
 */
extern void vision_ui_driver_line_v_dotted_draw(const vision_ui_t* ui, uint16_t x, uint16_t y, uint16_t h);

/**
 * Draws a monochrome bitmap.
 *
 * @param ui UI instance whose driver should draw. Must not be `NULL`.
 * @param x Left position in pixels.
 * @param y Top position in pixels.
 * @param w Bitmap width in pixels.
 * @param h Bitmap height in pixels.
 * @param bit_map Borrowed bitmap data pointer. The pointed-to bitmap should remain valid for the duration of the draw
 * call.
 */
extern void vision_ui_driver_bmp_draw(
        const vision_ui_t* ui,
        uint16_t x,
        uint16_t y,
        uint16_t w,
        uint16_t h,
        const uint8_t* bit_map
);

/**
 * Changes the current draw color.
 *
 * @param ui UI instance whose driver state should change. Must not be `NULL`.
 * @param color Backend-specific draw color value.
 */
extern void vision_ui_driver_color_draw(const vision_ui_t* ui, uint8_t color);

/**
 * Changes the backend text drawing mode.
 *
 * @param ui UI instance whose driver state should change. Must not be `NULL`.
 * @param mode Backend-specific text drawing mode.
 */
extern void vision_ui_driver_font_mode_set(const vision_ui_t* ui, uint8_t mode);

/**
 * Changes the backend text direction.
 *
 * @param ui UI instance whose driver state should change. Must not be `NULL`.
 * @param dir Backend-specific text direction value.
 */
extern void vision_ui_driver_font_direction_set(const vision_ui_t* ui, uint8_t dir);

/**
 * Restricts drawing to a rectangular clip region.
 *
 * @param ui UI instance whose driver state should change. Must not be `NULL`.
 * @param x0 Clip region start X coordinate.
 * @param y0 Clip region start Y coordinate.
 * @param x1 Clip region end X coordinate.
 * @param y1 Clip region end Y coordinate.
 */
extern void vision_ui_driver_clip_window_set(const vision_ui_t* ui, int16_t x0, int16_t y0, int16_t x1, int16_t y1);

/**
 * Clears the current clip region.
 *
 * @param ui UI instance whose driver state should change. Must not be `NULL`.
 */
extern void vision_ui_driver_clip_window_reset(const vision_ui_t* ui);

/**
 * Clears the full frame buffer.
 *
 * @param ui UI instance whose driver should clear the buffer. Must not be `NULL`.
 */
extern void vision_ui_driver_buffer_clear(const vision_ui_t* ui);

/**
 * Presents the current frame buffer.
 *
 * @param ui UI instance whose driver should present the buffer. Must not be `NULL`.
 */
extern void vision_ui_driver_buffer_send(const vision_ui_t* ui);

/**
 * Presents only a sub-rectangle of the frame buffer if the backend supports it.
 *
 * @param ui UI instance whose driver should present the buffer. Must not be `NULL`.
 * @param x Left position of the dirty rectangle.
 * @param y Top position of the dirty rectangle.
 * @param w Width of the dirty rectangle.
 * @param h Height of the dirty rectangle.
 */
extern void vision_ui_driver_buffer_area_send(const vision_ui_t* ui, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

/**
 * Returns a readable pointer to the full-size frame buffer used by transition and blur effects.
 *
 * @param ui UI instance whose driver buffer should be queried. Must not be `NULL`.
 * @return A pointer to the readable full-size frame buffer when the backend exposes one. Backends that cannot expose a
 * readable full buffer may return `NULL` or another unusable pointer for transition/blur effects, in which case those
 * effects are not fully supported.
 *
 * @attention this should be a full size buffer
 */
extern void* vision_ui_driver_buffer_pointer_get(const vision_ui_t* ui);

#endif // VISION_UI_VISION_UI_DRAW_DRIVER_H
