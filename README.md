![Vision UI Simulator](docs/vision-ui-simulator.png)

# Vision UI

Vision UI is a small list-based UI framework for embedded displays. It provides a menu tree, animated selection and
scrolling, notifications, alerts, and a driver abstraction so the same UI logic can run in a desktop simulator or on
device-specific backends.

The repository includes an SDL2 simulator built on top of u8g2, which makes it easy to prototype menus and custom
full-screen scenes before integrating the library into firmware.

## Features

- Hierarchical list navigation with title rows, switches, sliders, icon items, and custom user-rendered scenes.
- Animated selector, camera tracking, text scrolling, enter/exit transitions, notifications, and alerts.
- Thin driver interface for input, text measurement, drawing primitives, clipping, and buffer submission.
- Configurable layout and timing constants in [`include/vision_ui_config.h`](include/vision_ui_config.h), with a tuning
  guide in [`docs/CONFIG.md`](docs/CONFIG.md).
- SDL2 simulator target for local development and visual iteration.

## Repository Layout

- `include/vision/`: public headers for the core, item model, renderer, and driver interface.
- `include/vision_ui_config.h`: compile-time layout and animation settings.
- `src/`: runtime implementation for the core, renderer, animation helpers, and item system.
- `src/driver/`: simulator-oriented u8g2 driver glue.
- `main.cpp`: simulator demo application.
- `docs/`: screenshots and end-user documentation such as [`docs/api.md`](docs/api.md) and [
  `docs/CONFIG.md`](docs/CONFIG.md).
- `xmake.lua`: build definition for the library and simulator.

## Requirements

- [xmake](https://xmake.io)
- SDL2 development headers
- A local checkout of [u8g2](https://github.com/olikraus/u8g2) at `components/u8g2`

On macOS:

```bash
brew install xmake sdl2
```

On Ubuntu/Debian:

```bash
sudo apt-get update
sudo apt-get install -y xmake libsdl2-dev
```

## Build and Run the Simulator

If `components/u8g2` is missing, fetch it first:

```bash
mkdir -p components
git clone --depth=1 https://github.com/olikraus/u8g2.git components/u8g2
```

Build and run:

```bash
xmake f --simulator=y
xmake
xmake run vision_ui_simulator
```

Simulator controls:

| Key           | Action                                                                |
|---------------|-----------------------------------------------------------------------|
| `Up` / `Down` | Move between items or adjust a confirmed slider                       |
| `Space`       | Enter a list, toggle a switch, or confirm the current item            |
| `Esc`         | Exit the current layer or leave the UI when top-level exit is enabled |

## Integration Overview

Vision UI integration has three parts:

1. Implement or reuse a draw driver.
2. Build the UI tree.
3. Run the frame loop.

### 1. Bind a Driver

The simulator uses the reference driver in [`src/driver/u8g2.c`](src/driver/u8g2.c). If you are targeting another
platform, implement the functions declared in [
`include/vision/vision_ui_draw_driver.h`](include/vision/vision_ui_draw_driver.h).

Typical setup looks like this:

```cpp
vision_ui_driver_bind(&your_driver);

vision_ui_font_set_title({ .font = title_font, .top_compensation = 0, .bottom_compensation = 0 });
vision_ui_font_set_subtitle({ .font = subtitle_font, .top_compensation = 0, .bottom_compensation = 0 });
vision_ui_font_set({ .font = body_font, .top_compensation = 0, .bottom_compensation = 0 });
```

### 2. Build the UI Tree

Create the root list, attach child items, then call `vision_ui_core_init()` after the tree has at least one selectable
item.

```cpp
vision_ui_list_item_t* root = vision_ui_list_item_new(8, false, "VisionUI");
vision_ui_root_item_set(root);

vision_ui_list_push_item(root, vision_ui_list_title_item_new("VisionUI"));

vision_ui_list_item_t* settings = vision_ui_list_item_new(4, false, "Settings");
vision_ui_list_push_item(root, settings);

vision_ui_list_push_item(
    root,
    vision_ui_list_switch_item_new("Invert Display", false, [](bool enabled) {
        printf("invert: %s\n", enabled ? "on" : "off");
    })
);

vision_ui_list_push_item(
    settings,
    vision_ui_list_slider_item_new("Brightness", 50, 5, 0, 100, [](int16_t value) {
        printf("brightness: %d\n", value);
    })
);

vision_ui_list_push_item(
    root,
    vision_ui_list_user_item_new("About", init_fn, loop_fn, exit_fn)
);

vision_ui_core_init();
```

Available item constructors:

- `vision_ui_list_item_new(capacity, icon_mode, content)`
- `vision_ui_list_title_item_new(title)`
- `vision_ui_list_icon_item_new(capacity, icon_bitmap, title, description)`
- `vision_ui_list_switch_item_new(content, default_value, on_changed)`
- `vision_ui_list_slider_item_new(content, default_value, step, min, max, on_changed)`
- `vision_ui_list_user_item_new(content, init_function, loop_function, exit_function)`

### 3. Run the Frame Loop

```cpp
vision_ui_render_init();

while (!vision_ui_is_exited()) {
    vision_ui_driver_buffer_clear();
    vision_ui_step_render();
    vision_ui_driver_buffer_send();
}
```

`vision_ui_step_render()` handles input dispatch, animations, list rendering, notifications, alerts, and user-item
rendering.

## API Reference

The API is documented in [`docs/api.md`](docs/api.md). That page groups the exported functions by task, explains when to
call them, and separates the common integration path from lower-level helpers.

## Configuration Guide

Display sizing, list spacing, selector sizing, notification dimensions, and icon-view layout are documented in [
`docs/config.md`](docs/config.md). That page explains the naming in [
`include/vision_ui_config.h`](include/vision_ui_config.h) using the layout sketch in [
`docs/layout.png`](docs/layout.png).

## Notes

- The demo in [`main.cpp`](main.cpp) is the best reference for current simulator setup and item composition.
- `components/u8g2` is treated as an external dependency and is not managed by the README through submodule commands.
- Most customization is done through compile-time constants in [
  `include/vision_ui_config.h`](include/vision_ui_config.h). See [`docs/CONFIG.md`](docs/CONFIG.md) before changing
  spacing values.

## License

This project is licensed under `GPL-2.0-only`.
