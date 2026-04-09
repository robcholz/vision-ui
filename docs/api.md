# Vision UI API Reference

This document explains the exported Vision UI API in plain language. If you are integrating the library for the first
time, start with the sections below in order:

1. Lifecycle
2. Build the UI tree
3. Notifications and alerts
4. Driver contract

If you are porting the backend driver, read [`migration.md`](migration.md) first.

Header locations:

- [`include/vision/vision_ui_core.h`](../include/vision/vision_ui_core.h)
- [`include/vision/vision_ui_item.h`](../include/vision/vision_ui_item.h)
- [`include/vision/vision_ui_draw_driver.h`](../include/vision/vision_ui_draw_driver.h)
- [`include/vision/vision_ui_renderer.h`](../include/vision/vision_ui_renderer.h)

## Typical Flow

Most applications use Vision UI in this order:

1. Bind a draw driver with `vision_ui_driver_bind(...)`.
2. Configure fonts with `vision_ui_font_set...(...)`.
3. Create the root list and child items.
4. Call `vision_ui_root_item_set(root)`.
5. Call `vision_ui_core_init()`.
6. Call `vision_ui_render_init()`.
7. In your main loop, clear the buffer, call `vision_ui_step_render()`, then send the buffer.

Example:

```cpp
vision_ui_font_t title_font = { .font = title_ptr, .top_compensation = 0, .bottom_compensation = 0 };
vision_ui_font_t subtitle_font = { .font = subtitle_ptr, .top_compensation = 0, .bottom_compensation = 0 };
vision_ui_font_t body_font = { .font = body_ptr, .top_compensation = 0, .bottom_compensation = 0 };

vision_ui_driver_bind(&driver);

vision_ui_font_set_title(title_font);
vision_ui_font_set_subtitle(subtitle_font);
vision_ui_font_set(body_font);

vision_ui_list_item_t* root = vision_ui_list_item_new(8, false, "VisionUI");
vision_ui_root_item_set(root);

vision_ui_list_push_item(root, vision_ui_list_title_item_new("VisionUI"));
vision_ui_list_push_item(root, vision_ui_list_switch_item_new("Invert Display", false, on_invert_changed));

vision_ui_core_init();
vision_ui_render_init();

while (!vision_ui_is_exited()) {
    vision_ui_driver_buffer_clear();
    vision_ui_step_render();
    vision_ui_driver_buffer_send();
}
```

## Core Types

### `vision_ui_font_t`

Use this struct when you configure fonts:

```cpp
vision_ui_font_t font = {
    .font = your_font_pointer,
    .top_compensation = 0,
    .bottom_compensation = 0,
};
```

Fields:

- `font`: the backend font object, such as a u8g2 font pointer.
- `top_compensation`: shifts text metrics upward if the font sits too low.
- `bottom_compensation`: shifts text metrics downward if the font clips or sits too high.

### `vision_ui_action_t`

This enum is returned by the driver input hook:

- `UiActionNone`
- `UiActionGoPrev`
- `UiActionGoNext`
- `UiActionEnter`
- `UiActionExit`

## Lifecycle

These functions control startup and per-frame execution.

| Function                                                      | What it does                                                 | When to call it                     |
|---------------------------------------------------------------|--------------------------------------------------------------|-------------------------------------|
| `vision_ui_render_init()`                                     | Marks the UI as active and prepares the renderer to draw.    | Once before the main render loop.   |
| `vision_ui_core_init()`                                       | Initializes selection, camera state, and list runtime state. | After the root tree is built.       |
| `vision_ui_start_logo_set(const uint8_t* bmp, uint32_t span)` | Shows a startup bitmap for a fixed amount of time.           | Optional, before rendering starts.  |
| `vision_ui_step_render()`                                     | Runs one frame of UI logic and rendering.                    | Every frame.                        |
| `vision_ui_is_exited()`                                       | Reports whether the UI has been closed.                      | Usually as the main loop condition. |
| `vision_ui_is_background_frozen()`                            | Reports whether background interaction should pause.         | Optional, mainly for custom scenes. |

## Build the UI Tree

Vision UI is organized as a tree of `vision_ui_list_item_t` nodes. Every screen is a list, and every row inside that
list is another item.

For container items, `capacity` means "how many direct children this item can hold."

### Root and tree helpers

| Function                                                                                | What it does                        |
|-----------------------------------------------------------------------------------------|-------------------------------------|
| `vision_ui_root_item_set(vision_ui_list_item_t* item)`                                  | Sets the top-level list.            |
| `vision_ui_root_list_get()`                                                             | Returns the current root list.      |
| `vision_ui_list_push_item(vision_ui_list_item_t* parent, vision_ui_list_item_t* child)` | Adds a child item to a parent list. |

### Item constructors

| Function                                                                                                                                          | Use it for                                                                                        |
|---------------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------------------------------------------------------------------------|
| `vision_ui_list_item_new(size_t capacity, bool icon_mode, const char* content)`                                                                   | A normal list container that can hold child items. Set `icon_mode` to `true` for icon-view lists. |
| `vision_ui_list_title_item_new(const char* title)`                                                                                                | A non-interactive title row.                                                                      |
| `vision_ui_list_icon_item_new(size_t capacity, const uint8_t* icon, const char* title, const char* description)`                                  | A selectable icon card, optionally with child items.                                              |
| `vision_ui_list_switch_item_new(const char* content, bool default_value, void (*on_changed)(bool))`                                               | A toggle row. The callback runs when the value changes.                                           |
| `vision_ui_list_slider_item_new(const char* content, int16_t default_value, uint8_t step, int16_t min, int16_t max, void (*on_changed)(int16_t))` | A numeric slider row.                                                                             |
| `vision_ui_list_user_item_new(const char* content, void (*init_function)(), void (*loop_function)(), void (*exit_function)())`                    | A custom full-screen scene that takes over drawing while selected.                                |

### User items

`vision_ui_list_user_item_new(...)` is the escape hatch for fully custom screens.

- `init_function`: runs the first time the user item becomes active.
- `loop_function`: runs every frame while the user item is active.
- `exit_function`: runs when leaving the user item.

Inside a user item, you draw using the same `vision_ui_driver_*` functions that the normal renderer uses.

### Type helpers

These cast a generic `vision_ui_list_item_t*` to a more specific item type:

- `vision_ui_to_list_switch_item(...)`
- `vision_ui_to_list_slider_item(...)`
- `vision_ui_to_list_icon_item(...)`
- `vision_ui_to_list_user_item(...)`

You usually only need these if you are doing custom logic outside the default callbacks.

## Notifications and Alerts

These show temporary UI messages on top of the current screen.

| Function                                                          | What it does                                      |
|-------------------------------------------------------------------|---------------------------------------------------|
| `vision_ui_notification_push(const char* content, uint16_t span)` | Shows a notification bar for `span` milliseconds. |
| `vision_ui_alert_push(const char* content, uint16_t span)`        | Shows a centered alert for `span` milliseconds.   |

There are also instance getter functions in `vision_ui_item.h`:

- `vision_ui_notification_instance_get()`
- `vision_ui_notification_mutable_instance_get()`
- `vision_ui_alert_instance_get()`
- `vision_ui_alert_mutable_instance_get()`

Most applications do not need those getters. They are useful if you are extending renderer behavior.

## Fonts and Memory

### Font configuration

| Function                                             | What it does                            |
|------------------------------------------------------|-----------------------------------------|
| `vision_ui_minifont_set(vision_ui_font_t font)`      | Sets the small utility font.            |
| `vision_ui_font_set(vision_ui_font_t font)`          | Sets the main body font.                |
| `vision_ui_font_set_title(vision_ui_font_t font)`    | Sets the title font.                    |
| `vision_ui_font_set_subtitle(vision_ui_font_t font)` | Sets the subtitle font.                 |
| `vision_ui_minifont_get()`                           | Returns the current small utility font. |
| `vision_ui_font_get()`                               | Returns the current body font.          |
| `vision_ui_font_get_title()`                         | Returns the current title font.         |
| `vision_ui_font_get_subtitle()`                      | Returns the current subtitle font.      |

### Custom allocator

`vision_ui_allocator_set(...)` lets you replace the default memory allocation hooks.

```cpp
void vision_ui_allocator_set(
    void* (*allocator)(vision_alloc_op_t op, size_t size, size_t count, void* ptr)
);
```

`vision_alloc_op_t` tells your allocator what to do:

- `VisionAllocMalloc`
- `VisionAllocCalloc`
- `VisionAllocFree`

Most users can ignore this unless they need a custom allocator for embedded memory control.

## Driver Contract

To run Vision UI on a new platform, implement the functions in `vision_ui_draw_driver.h`.

### Input and timing

| Function                              | What it does                                    |
|---------------------------------------|-------------------------------------------------|
| `vision_ui_driver_action_get()`       | Returns the current high-level user action.     |
| `vision_ui_driver_ticks_ms_get()`     | Returns a monotonic time in milliseconds.       |
| `vision_ui_driver_delay(uint32_t ms)` | Sleeps or yields for roughly `ms` milliseconds. |
| `vision_ui_driver_bind(void* driver)` | Stores the backend driver handle.               |

### Font and text

| Function                                           | What it does                                        |
|----------------------------------------------------|-----------------------------------------------------|
| `vision_ui_driver_font_set(vision_ui_font_t font)` | Activates a font in the backend.                    |
| `vision_ui_driver_font_get()`                      | Returns the currently active backend font.          |
| `vision_ui_driver_str_draw(...)`                   | Draws plain text.                                   |
| `vision_ui_driver_str_utf8_draw(...)`              | Draws UTF-8 text.                                   |
| `vision_ui_driver_str_width_get(...)`              | Returns the width of plain text.                    |
| `vision_ui_driver_str_utf8_width_get(...)`         | Returns the width of UTF-8 text.                    |
| `vision_ui_driver_str_height_get()`                | Returns the current text height.                    |
| `vision_ui_driver_font_mode_set(uint8_t mode)`     | Changes transparent versus solid text drawing mode. |
| `vision_ui_driver_font_direction_set(uint8_t dir)` | Changes text direction.                             |

### Drawing primitives

| Function                                     | What it does                         |
|----------------------------------------------|--------------------------------------|
| `vision_ui_driver_pixel_draw(...)`           | Draws one pixel.                     |
| `vision_ui_driver_circle_draw(...)`          | Draws a circle outline.              |
| `vision_ui_driver_disc_draw(...)`            | Draws a filled circle.               |
| `vision_ui_driver_box_r_draw(...)`           | Draws a rounded filled rectangle.    |
| `vision_ui_driver_box_draw(...)`             | Draws a filled rectangle.            |
| `vision_ui_driver_frame_draw(...)`           | Draws a rectangular outline.         |
| `vision_ui_driver_frame_r_draw(...)`         | Draws a rounded rectangular outline. |
| `vision_ui_driver_line_h_draw(...)`          | Draws a horizontal line.             |
| `vision_ui_driver_line_v_draw(...)`          | Draws a vertical line.               |
| `vision_ui_driver_line_draw(...)`            | Draws an arbitrary line.             |
| `vision_ui_driver_line_h_dotted_draw(...)`   | Draws a dotted horizontal line.      |
| `vision_ui_driver_line_v_dotted_draw(...)`   | Draws a dotted vertical line.        |
| `vision_ui_driver_bmp_draw(...)`             | Draws a bitmap.                      |
| `vision_ui_driver_color_draw(uint8_t color)` | Changes the current draw color.      |

### Clipping and buffers

| Function                                 | What it does                                                |
|------------------------------------------|-------------------------------------------------------------|
| `vision_ui_driver_clip_window_set(...)`  | Limits drawing to a rectangular area.                       |
| `vision_ui_driver_clip_window_reset()`   | Removes the clip window.                                    |
| `vision_ui_driver_buffer_clear()`        | Clears the frame buffer.                                    |
| `vision_ui_driver_buffer_send()`         | Presents the current frame buffer.                          |
| `vision_ui_driver_buffer_area_send(...)` | Updates only part of the buffer if the backend supports it. |
| `vision_ui_driver_buffer_pointer_get()`  | Returns the raw full-size buffer pointer.                   |

`vision_ui_driver_buffer_pointer_get()` is mainly used by transition and blur effects. If your backend supports Vision
UI fully, this must point to a valid frame buffer.

## List Icon Helpers

The renderer exposes a small icon-theme hook in `vision_ui_renderer.h`.

| Function                                         | What it does                            |
|--------------------------------------------------|-----------------------------------------|
| `vision_ui_list_icon_set(vision_ui_icon_t icon)` | Replaces the current list icon pack.    |
| `vision_ui_list_icon_get_current()`              | Returns the currently active icon pack. |
| `DEFAULT_LIST_ICON`                              | Built-in icon pack used by the demo.    |

`vision_ui_icon_t` contains bitmap pointers and dimensions for list headers and footers.

## Exposed but Usually Not Called Directly

Some symbols are public because they live in exported headers, but most applications should not call them manually:

- `vision_ui_exit_animation_is_finished()`
- `vision_ui_exit_animation_set_is_finished()`
- `vision_ui_exit_animation_start()`
- `vision_ui_enter_animation_is_finished()`
- `vision_ui_enter_animation_set_is_finished()`
- `vision_ui_enter_animation_start()`
- `vision_ui_selector_instance_get()`
- `vision_ui_selector_mutable_instance_get()`
- `vision_ui_selector_t_selector_bind_item()`
- `vision_ui_selector_go_next_item()`
- `vision_ui_selector_go_prev_item()`
- `vision_ui_selector_jump_to_selected_item()`
- `vision_ui_selector_exit_current_item()`
- `vision_ui_camera_instance_get()`
- `vision_ui_camera_mutable_instance_get()`
- `vision_ui_camera_instance_x_trg_set()`
- `vision_ui_camera_instance_y_trg_set()`
- `vision_ui_camera_bind_selector()`
- `vision_ui_exit_animation_render()`
- `vision_ui_enter_animation_render()`
- `vision_ui_widget_render()`
- `vision_ui_list_render()`

These are useful if you are extending the framework itself, writing a custom shell around it, or debugging state
transitions. For normal app integration, stick to the lifecycle, tree-building, notification, font, and driver APIs
above.
