use crate::{Action, Font};
use core::ffi::{c_void, CStr};
use core::time::Duration;

/// Input callbacks required by the Vision UI runtime.
///
/// This trait models the behavior behind `vision_ui_driver_action_get(...)`.
/// Implementations are expected to return only valid high-level [`Action`] values.
pub trait Input {
    /// Returns the current UI navigation action.
    ///
    /// Returns:
    /// - the next high-level input action observed by the driver.
    ///
    /// Behavior:
    /// - Implementations should map backend-specific input events onto the small
    ///   Vision UI action set.
    /// - Invalid or out-of-range action codes should not be exposed here.
    fn action(&mut self) -> Action;
}

/// Timekeeping callbacks required by the Vision UI runtime.
///
/// This trait models `vision_ui_driver_ticks_ms_get(...)` and
/// `vision_ui_driver_delay(...)`.
pub trait Timing {
    /// Returns the driver's monotonic tick counter.
    ///
    /// Returns:
    /// - a monotonically increasing duration interpreted by Vision UI as milliseconds.
    ///
    /// Behavior:
    /// - The value should be suitable for animation timing, timeouts, and UI transitions.
    fn ticks(&mut self) -> Duration;

    /// Delays or yields for roughly the requested duration.
    ///
    /// Parameters:
    /// - `duration`: target delay requested by the runtime.
    ///
    /// Behavior:
    /// - Implementations may sleep, busy-wait, or yield depending on the platform.
    /// - The runtime does not require cycle-accurate timing.
    fn delay(&mut self, duration: Duration);
}

/// Text and font operations required by the Vision UI renderer.
///
/// This trait corresponds to the font and string drawing functions declared in
/// `vision_ui_draw_driver.h`.
pub trait Text {
    /// Selects the current font used by subsequent text operations.
    ///
    /// Parameters:
    /// - `font`: backend-specific font descriptor defined by Vision UI.
    fn set_font(&mut self, font: Font);

    /// Returns the currently active font.
    ///
    /// Returns:
    /// - the font currently selected in the backend.
    fn font(&self) -> Font;

    /// Draws an ASCII or byte-oriented string at the given baseline position.
    ///
    /// Parameters:
    /// - `x`: horizontal text origin in pixels.
    /// - `y`: vertical baseline position in pixels.
    /// - `text`: NUL-terminated string borrowed for the duration of the call.
    fn draw_text(&mut self, x: u16, y: u16, text: &CStr);

    /// Draws a UTF-8 string at the given baseline position.
    ///
    /// Parameters:
    /// - `x`: horizontal text origin in pixels.
    /// - `y`: vertical baseline position in pixels.
    /// - `text`: NUL-terminated UTF-8 string borrowed for the duration of the call.
    fn draw_utf8(&mut self, x: u16, y: u16, text: &CStr);

    /// Measures the width of an ASCII or byte-oriented string.
    ///
    /// Parameters:
    /// - `text`: NUL-terminated string to measure.
    ///
    /// Returns:
    /// - the rendered width in pixels using the current font.
    fn text_width(&self, text: &CStr) -> u16;

    /// Measures the width of a UTF-8 string.
    ///
    /// Parameters:
    /// - `text`: NUL-terminated UTF-8 string to measure.
    ///
    /// Returns:
    /// - the rendered width in pixels using the current font.
    fn utf8_width(&self, text: &CStr) -> u16;

    /// Returns the current text height.
    ///
    /// Returns:
    /// - the font height in pixels used by the renderer.
    fn text_height(&self) -> u16;

    /// Configures the backend font draw mode.
    ///
    /// Parameters:
    /// - `mode`: backend-specific font mode value.
    fn set_font_mode(&mut self, mode: u8);

    /// Configures the backend text direction.
    ///
    /// Parameters:
    /// - `direction`: backend-specific text direction value.
    fn set_font_direction(&mut self, direction: u8);
}

/// Primitive drawing operations required by the Vision UI renderer.
///
/// This trait mirrors the geometric and bitmap drawing hooks used by the C renderer.
pub trait Draw {
    /// Draws a single pixel.
    fn pixel(&mut self, x: u16, y: u16);

    /// Draws a circle outline.
    fn circle(&mut self, x: u16, y: u16, radius: u16);

    /// Draws a filled circle.
    fn disc(&mut self, x: u16, y: u16, radius: u16);

    /// Draws a filled rounded rectangle.
    fn fill_rounded_rect(&mut self, x: u16, y: u16, width: u16, height: u16, radius: u16);

    /// Draws a filled rectangle.
    fn fill_rect(&mut self, x: u16, y: u16, width: u16, height: u16);

    /// Draws a rectangular outline.
    fn stroke_rect(&mut self, x: u16, y: u16, width: u16, height: u16);

    /// Draws a rounded-rectangle outline.
    fn stroke_rounded_rect(&mut self, x: u16, y: u16, width: u16, height: u16, radius: u16);

    /// Draws a horizontal line.
    fn hline(&mut self, x: u16, y: u16, length: u16);

    /// Draws a vertical line.
    fn vline(&mut self, x: u16, y: u16, height: u16);

    /// Draws a straight line segment.
    fn line(&mut self, x1: u16, y1: u16, x2: u16, y2: u16);

    /// Draws a dotted horizontal line.
    fn dotted_hline(&mut self, x: u16, y: u16, length: u16);

    /// Draws a dotted vertical line.
    fn dotted_vline(&mut self, x: u16, y: u16, height: u16);

    /// Draws a monochrome bitmap.
    ///
    /// Parameters:
    /// - `x`: left pixel position.
    /// - `y`: top pixel position.
    /// - `width`: bitmap width in pixels.
    /// - `height`: bitmap height in pixels.
    /// - `bitmap`: borrowed XBM-compatible 1-bit payload.
    fn bitmap(&mut self, x: u16, y: u16, width: u16, height: u16, bitmap: &[u8]);

    /// Selects the current draw color.
    ///
    /// Parameters:
    /// - `color`: backend-specific draw color index used by Vision UI.
    fn set_color(&mut self, color: u8);

    /// Sets the active clip rectangle.
    ///
    /// Parameters:
    /// - `x0`, `y0`: top-left clip corner.
    /// - `x1`, `y1`: bottom-right clip corner.
    fn clip_rect(&mut self, x0: i16, y0: i16, x1: i16, y1: i16);

    /// Resets the clip rectangle to the full drawable area.
    fn reset_clip(&mut self);
}

/// Framebuffer management operations required by the Vision UI renderer.
///
/// This trait corresponds to the buffer-oriented hooks in `vision_ui_draw_driver.h`.
pub trait Buffer {
    /// Clears the current framebuffer or draw target.
    fn clear(&mut self);

    /// Presents the entire framebuffer.
    fn present(&mut self);

    /// Presents only a sub-rectangle of the framebuffer.
    ///
    /// Parameters:
    /// - `x`: left pixel position.
    /// - `y`: top pixel position.
    /// - `width`: region width in pixels.
    /// - `height`: region height in pixels.
    fn present_area(&mut self, x: u16, y: u16, width: u16, height: u16);

    /// Returns a raw framebuffer pointer when the backend exposes one.
    ///
    /// Returns:
    /// - `Some(*mut c_void)` when a live framebuffer pointer is available.
    /// - `None` when the backend does not expose direct buffer access.
    fn buffer_ptr(&self) -> Option<*mut c_void>;
}

/// Convenience supertrait for a complete Vision UI backend implementation.
pub trait Driver: Input + Timing + Text + Draw + Buffer {}

impl<T> Driver for T where T: Input + Timing + Text + Draw + Buffer {}

/// Adapter trait for passing an existing backend object into the native C runtime.
///
/// This is used by [`crate::VisionUi::bind_driver`]. Implementations should return
/// a stable opaque handle understood by the underlying C driver functions.
pub trait RawHandle {
    /// Returns the opaque native driver handle.
    ///
    /// Returns:
    /// - a backend-specific pointer stored by the native UI instance.
    ///
    /// Behavior:
    /// - The returned pointer must remain valid for as long as the UI may invoke driver hooks.
    fn as_raw_handle(&mut self) -> *mut c_void;
}
