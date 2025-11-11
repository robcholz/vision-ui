![Vision UI Simulator](docs/vision-ui-simulator.png)

# Vision UI

Vision UI is a lightweight, list-first UI framework for monochrome/colorful displays built on top of flexible drivers.
It ships with
an SDL2-powered desktop simulator so you can prototype menus, transitions, and custom widgets before flashing firmware
to your board.

## Features

- Focused on MCU dashboards: hierarchical lists, title rows, toggles, sliders, and user-defined scenes rendered at 60
  FPS (`main_english.cpp`).
- Rich micro-interactions: elastic selector, scrolling text, notification bar and alert toasts, scroll bars, and exit
  masks (`include/vision/vision_ui_renderer.c`).
- Event-driven core decoupled from hardware through the `vision_ui_driver_*` interface (`include/driver/u8g2.c`), making
  it portable across OLED, LCD, and simulator targets.
- Configurable layout constants (`include/vision_ui_config.h`) for screen size, paddings, animation speeds, and widget
  measurements.
- Bundled Chinese bitmap font (`include/font/chinese.h`) and both English/Mandarin demo launchers.
- Distributed under GPL-2.0-only so it can stay aligned with the upstream u8g2 license.

## Repository Layout

- `include/vision`: UI core, renderer, item system, and C/C++ headers.
- `include/driver`: Reference driver that maps Vision UI’s drawing API to u8g2/SDL.
- `include/font`: Custom glyph sets that can be swapped at runtime.
- `components/u8g2`: Vendored u8g2 sources (no submodules required).
- `main_english.cpp`, `main_mandarin.cpp`: Simulator entry points that showcase the feature set.
- `xmake.lua`: Primary build script (CMake file is auto-generated; prefer xmake).
- `build/`, `cmake-build-debug/`: Local build artifacts (safe to delete/regenerate).

## Getting Started

### Prerequisites

| Tool                      | Purpose                | macOS install        |
|---------------------------|------------------------|----------------------|
| [xmake](https://xmake.io) | Build orchestration    | `brew install xmake` |
| SDL2 (>=2.0)              | Simulator window/input | `brew install sdl2`  |
| CMake 3.15+ (optional)    | Alternate build system | `brew install cmake` |

> On Linux, install `libsdl2-dev` via your package manager. Windows users can rely on the SDL2 binaries bundled with
> their compiler toolchain.

### Clone & configure

```bash
git clone <repo-url>
cd vision_ui
xmake f -m debug            # optional: switch between debug/release
xmake f --language=mandarin # optional: run the Mandarin demo entry
```

### Build & run the simulator

```bash
xmake                      # builds u8g2, vision_ui, and the simulator
xmake run vision_ui_simulator
```

Keyboard controls inside the simulator:

| Key   | Action                                                                  |
|-------|-------------------------------------------------------------------------|
| ↑ / ↓ | Move between list entries or adjust sliders when “locked in”.           |
| Space | Enter / confirm the selected item.                                      |
| Esc   | Exit the current layer or quit Vision UI when at the root (if enabled). |

## Embed Vision UI in Firmware

Vision UI is organized around three responsibilities: supplying a drawing backend, defining the menu tree, and running
the render loop.

### 1. Bind your display driver

If you already rely on u8g2, you can reuse `include/driver/u8g2.c` as-is. Otherwise, implement the small
`vision_ui_driver_*` surface (text metrics, primitives, buffer swaps, input) and call:

```c
vision_ui_driver_bind(&your_driver);
vision_ui_font_set(your_font_pointer);
```

### 2. Describe your UI tree

```cpp
vision_ui_core_init();

auto *settings = vision_ui_list_item_new(10, "Board Settings");
vision_ui_list_push_item(vision_ui_root_list_get(), settings);

vision_ui_list_push_item(
    vision_ui_root_list_get(),
    vision_ui_list_switch_item_new(1, "Switch Screen", true, [](bool enabled) {
        vision_ui_notification_push(enabled ? "Screen A" : "Screen B", 1500);
    })
);

vision_ui_list_push_item(
    settings,
    vision_ui_list_slider_item_new(1, "Display Style", 1600, 5, 1, 9999, [](int16_t value) {
        printf("Style -> %d\n", value);
    })
);

vision_ui_list_push_item(
    vision_ui_root_list_get(),
    vision_ui_list_user_item_new(1, "About…", init_fn, loop_fn, exit_fn)
);
```

`LIST_ITEM`, `SWITCH_ITEM`, `SLIDER_ITEM`, and `USER_ITEM` preserve their own state (scroll offsets, slider
confirmation, etc.). User items can take over the whole render loop to draw bespoke scenes, as demonstrated in
`main_english.cpp:test_user_item_loop_function`.

### 3. Drive the render loop

```cpp
vision_ui_render_init();

while (!vision_ui_is_exited()) {
    vision_ui_driver_buffer_clear();
    vision_ui_step_render();   // handles input, animation, widgets, and custom scenes
    vision_ui_driver_buffer_send();
}
```

Animations (selector easing, camera tracking, exit masks) are computed in `vision_ui_core.c`, so the loop stays minimal.

## Notifications, Alerts, and Animations

- **Notification bar**: `vision_ui_notification_push("Saved", 2000);`
- **Alert toast**: `vision_ui_alert_push("Hello", 5000);`
- **Exit animation hooks**: inspect `vision_ui_exit_animation_status_get()` if your user item needs to pause while the
  sand-glass animation runs.
- **Background freeze**: call `vision_ui_is_background_frozen()` before changing state when an alert is visible.
- **Text marquee**: list items automatically scroll oversized strings with configurable speed/pause constants from
  `vision_ui_config.h`.

## Configuration & Localization

- Adjust dimensions, paddings, timing, and scroll-bar behaviors in `include/vision_ui_config.h`.
- Swap fonts at runtime with `vision_ui_font_set`. The repo ships with `u8g2_font_my_chinese` so UTF-8 labels render
  correctly on 1-bit displays.
- Provide your own translations by swapping out the simulator entry point (`xmake f --language=mandarin`) or bundling
  both variants in your firmware.

## Roadmap

- [x] rename the pop-up widget to alert
- [x] rename the info bar to notification
- [x] add multiple notification animation
- [ ] decouple the new page, user_item, list_view
- [ ] add fade out effect on the top of current effect when switching the page
- [ ] add animation for scroll bar
- [ ] add icon view
- [ ] use 2nd ODE for animation and fix the animation rate

## License

Vision UI, including the bundled copy of u8g2, is released under the terms of
the [GNU General Public License v2.0](LICENSE).

[u8g2]: https://github.com/olikraus/u8g2
