# Vision UI Driver Migration Guide

This page is only about porting the Vision UI driver layer.

Use it when you want to replace the simulator backend with a backend for:

- another graphics library
- a microcontroller display stack
- a custom framebuffer
- a different desktop renderer

This page does not explain:

- how to build the UI tree
- what each public API does
- how to tune layout constants

Those live in:

- [`api.md`](api.md)
- [`config.md`](config.md)

## What You Are Replacing

The reference backend is [`../src/driver/u8g2.c`](../src/driver/u8g2.c).

When you migrate Vision UI to another platform, the main job is to replace that backend with your own implementation of
[`vision_ui_draw_driver.h`](../include/vision/vision_ui_draw_driver.h).

In other words, migration here means:

- keep the Vision UI core
- keep the renderer
- keep your menu tree
- replace only the driver functions

## Reference Files

Start with these two files side by side:

- [`../include/vision/vision_ui_draw_driver.h`](../include/vision/vision_ui_draw_driver.h)
- [`../src/driver/u8g2.c`](../src/driver/u8g2.c)

The header tells you the required contract. The u8g2 file shows one complete backend implementation.

## Driver Responsibilities

The driver contract breaks into seven parts.

## 1. Input Mapping

Your backend must convert platform input into:

- `UiActionNone`
- `UiActionGoPrev`
- `UiActionGoNext`
- `UiActionEnter`
- `UiActionExit`

The simulator maps:

- `Up` to previous
- `Down` to next
- `Space` to enter
- `Esc` to exit

On real hardware, this is usually backed by:

- buttons
- a rotary encoder
- a touchscreen gesture layer

## 2. Timing

Your backend must provide:

- `vision_ui_driver_ticks_ms_get()`
- `vision_ui_driver_delay(uint32_t ms)`

Requirements:

- the tick counter should be monotonic
- the unit must really be milliseconds
- large timing jumps will make animation quality visibly worse

## 3. Font Selection and Text Metrics

Your backend must implement:

- `vision_ui_driver_font_set(...)`
- `vision_ui_driver_font_get()`
- `vision_ui_driver_str_draw(...)`
- `vision_ui_driver_str_utf8_draw(...)`
- `vision_ui_driver_str_width_get(...)`
- `vision_ui_driver_str_utf8_width_get(...)`
- `vision_ui_driver_str_height_get()`
- `vision_ui_driver_font_mode_set(...)`
- `vision_ui_driver_font_direction_set(...)`

This is one of the most important parts of the port.

If text looks wrong, the problem is usually here before it is in the layout constants.

Typical symptoms of broken text metrics:

- selector width looks wrong
- labels overlap the footer
- text scroll triggers too early or too late
- title and subtitle fonts do not sit correctly inside rows

## 4. Drawing Primitives

Your backend must provide the primitives used by the renderer:

- pixels
- circles and discs
- filled boxes and rounded boxes
- frames and rounded frames
- horizontal, vertical, and arbitrary lines
- dotted lines
- monochrome bitmap drawing
- draw color switching

If the target graphics library does not support a primitive directly, emulate it inside the driver instead of changing
the Vision UI core.

## 5. Clipping

Your backend must implement:

- `vision_ui_driver_clip_window_set(...)`
- `vision_ui_driver_clip_window_reset()`

Clipping is required for:

- scrolling text
- some list rendering paths
- icon-view text containment

Common clipping bugs:

- long text draws outside the selected row
- description text spills into neighboring areas
- transition artifacts appear near edges

## 6. Buffer Management

Your backend must implement:

- `vision_ui_driver_buffer_clear()`
- `vision_ui_driver_buffer_send()`
- `vision_ui_driver_buffer_area_send(...)`
- `vision_ui_driver_buffer_pointer_get()`

The important detail is `vision_ui_driver_buffer_pointer_get()`.

The renderer expects this to return a readable pointer to the full display buffer. It is used by visual effects such as
blur and transition logic.

If the target platform cannot provide a full readable buffer, that is a driver limitation you should document early.

## 7. Backend Handle Binding

`vision_ui_driver_bind(void* driver)` is how the platform-specific backend handle is passed into Vision UI.

In the simulator, that handle is a `u8g2_t*`.

On another platform it might be:

- a display object
- a framebuffer context
- a device handle
- a renderer state struct

The driver implementation decides what the pointer means.

## Migration Checklist

Use this order when bringing up a new backend:

1. Copy the reference backend structure from [`../src/driver/u8g2.c`](../src/driver/u8g2.c).
2. Replace the backend-specific types and calls with your platform equivalents.
3. Make `vision_ui_driver_bind(...)` store the backend handle.
4. Get input mapping working.
5. Get text width and height reporting working.
6. Get the basic primitives drawing correctly.
7. Get clipping working.
8. Get full buffer clear/send working.
9. Verify `vision_ui_driver_buffer_pointer_get()` returns a valid full buffer.

Do not start by changing the UI tree or layout constants. That makes backend bugs harder to isolate.

## Suggested Bring-Up Test

Use a very small test menu first:

- one title row
- one switch row
- one slider row

That gives you a quick way to verify:

- input mapping
- selector movement
- text metrics
- footer rendering
- clipping

Use [`../main.cpp`](../main.cpp) as the reference behavior.

## Common Driver Bugs

### Nothing is visible

Check:

- the backend handle was bound
- `buffer_clear` and `buffer_send` affect the real display
- draw color `1` really means visible pixels in your backend

### Text width feels wrong

Check:

- UTF-8 width and plain width functions separately
- active font changes are actually applied
- text height reflects the same font that width uses

### Scrolling text leaks outside its box

Check:

- clip-window coordinates
- whether the backend clip API uses inclusive or exclusive end coordinates

### Blur or transition effects break

Check:

- `vision_ui_driver_buffer_pointer_get()`
- whether the returned pointer is a full frame buffer rather than a partial tile buffer

### Animation feels unstable

Check:

- monotonic tick source
- millisecond units
- frame loop stalls

## What Not to Change First

When a migration is failing, do not first modify:

- the menu tree
- item constructors
- renderer internals
- layout constants

If the backend contract is wrong, those changes only hide the actual problem.

## Related Files

- [`../include/vision/vision_ui_draw_driver.h`](../include/vision/vision_ui_draw_driver.h): required driver contract
- [`../src/driver/u8g2.c`](../src/driver/u8g2.c): reference backend implementation
- [`../main.cpp`](../main.cpp): working simulator example
- [`api.md`](api.md): public API reference
- [`config.md`](config.md): layout and timing configuration
