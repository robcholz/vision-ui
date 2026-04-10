//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//

#ifndef VISION_UI_VISION_UI_DRAW_DRIVER_H
#define VISION_UI_VISION_UI_DRAW_DRIVER_H

#include <stdint.h>

#include "vision_ui_types.h"

/// Returns the current high-level input action from the backend.
extern vision_ui_action_t vision_ui_driver_action_get(const vision_ui_t* ui);

/// Returns a monotonic millisecond tick value.
extern uint32_t vision_ui_driver_ticks_ms_get(const vision_ui_t* ui);

/// Sleeps or yields for roughly the requested number of milliseconds.
extern void vision_ui_driver_delay(const vision_ui_t* ui, uint32_t ms);

/// Stores the backend driver handle on the UI instance.
extern void vision_ui_driver_bind(vision_ui_t* ui, void* driver);

/// Activates a backend font for subsequent text operations.
extern void vision_ui_driver_font_set(vision_ui_t* ui, vision_ui_font_t font);

/// Returns the backend font that is currently active.
extern vision_ui_font_t vision_ui_driver_font_get(const vision_ui_t* ui);

/// Draws plain text using the currently active font.
extern void vision_ui_driver_str_draw(const vision_ui_t* ui, uint16_t x, uint16_t y, const char* str);

/// Draws UTF-8 text using the currently active font.
extern void vision_ui_driver_str_utf8_draw(const vision_ui_t* ui, uint16_t x, uint16_t y, const char* str);

/// Returns the rendered width of plain text.
extern uint16_t vision_ui_driver_str_width_get(const vision_ui_t* ui, const char* str);

/// Returns the rendered width of UTF-8 text.
extern uint16_t vision_ui_driver_str_utf8_width_get(const vision_ui_t* ui, const char* str);

/// Returns the current text height for the active font.
extern uint16_t vision_ui_driver_str_height_get(const vision_ui_t* ui);

/// Draws a single pixel.
extern void vision_ui_driver_pixel_draw(const vision_ui_t* ui, uint16_t x, uint16_t y);

/// Draws a circle outline.
extern void vision_ui_driver_circle_draw(const vision_ui_t* ui, uint16_t x, uint16_t y, uint16_t r);

/// Draws a filled circle.
extern void vision_ui_driver_disc_draw(const vision_ui_t* ui, uint16_t x, uint16_t y, uint16_t r);

/// Draws a rounded filled rectangle.
extern void vision_ui_driver_box_r_draw(
        const vision_ui_t* ui,
        uint16_t x,
        uint16_t y,
        uint16_t w,
        uint16_t h,
        uint16_t r
);

/// Draws a filled rectangle.
extern void vision_ui_driver_box_draw(const vision_ui_t* ui, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

/// Draws a rectangular outline.
extern void vision_ui_driver_frame_draw(const vision_ui_t* ui, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

/// Draws a rounded rectangular outline.
extern void vision_ui_driver_frame_r_draw(
        const vision_ui_t* ui,
        uint16_t x,
        uint16_t y,
        uint16_t w,
        uint16_t h,
        uint16_t r
);

/// Draws a horizontal line.
extern void vision_ui_driver_line_h_draw(const vision_ui_t* ui, uint16_t x, uint16_t y, uint16_t l);

/// Draws a vertical line.
extern void vision_ui_driver_line_v_draw(const vision_ui_t* ui, uint16_t x, uint16_t y, uint16_t h);

/// Draws an arbitrary line segment.
extern void vision_ui_driver_line_draw(const vision_ui_t* ui, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

/// Draws a dotted horizontal line.
extern void vision_ui_driver_line_h_dotted_draw(const vision_ui_t* ui, uint16_t x, uint16_t y, uint16_t l);

/// Draws a dotted vertical line.
extern void vision_ui_driver_line_v_dotted_draw(const vision_ui_t* ui, uint16_t x, uint16_t y, uint16_t h);

/// Draws a monochrome bitmap.
extern void vision_ui_driver_bmp_draw(
        const vision_ui_t* ui,
        uint16_t x,
        uint16_t y,
        uint16_t w,
        uint16_t h,
        const uint8_t* bit_map
);

/// Changes the current draw color.
extern void vision_ui_driver_color_draw(const vision_ui_t* ui, uint8_t color);

/// Changes the backend text drawing mode.
extern void vision_ui_driver_font_mode_set(const vision_ui_t* ui, uint8_t mode);

/// Changes the backend text direction.
extern void vision_ui_driver_font_direction_set(const vision_ui_t* ui, uint8_t dir);

/// Restricts drawing to a rectangular clip region.
extern void vision_ui_driver_clip_window_set(const vision_ui_t* ui, int16_t x0, int16_t y0, int16_t x1, int16_t y1);

/// Clears the current clip region.
extern void vision_ui_driver_clip_window_reset(const vision_ui_t* ui);

/// Clears the full frame buffer.
extern void vision_ui_driver_buffer_clear(const vision_ui_t* ui);

/// Presents the current frame buffer.
extern void vision_ui_driver_buffer_send(const vision_ui_t* ui);

/// Presents only a sub-rectangle of the frame buffer if the backend supports it.
extern void vision_ui_driver_buffer_area_send(const vision_ui_t* ui, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

/// Returns a readable pointer to the full-size frame buffer used by transition and blur effects.
/// @attention this should be a full size buffer
extern void* vision_ui_driver_buffer_pointer_get(const vision_ui_t* ui);

#endif // VISION_UI_VISION_UI_DRAW_DRIVER_H
