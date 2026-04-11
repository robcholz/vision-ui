# vision-ui

Idiomatic Rust bindings for the Vision UI C library.

## Coverage

This crate provides:

- `vision_ui::VisionUi`: a safe, Rust-style wrapper around a `vision_ui_t` instance.
- `vision_ui::UiRef`: a callback-safe handle for closures invoked by the UI runtime.
- `vision_ui::raw`: the generated raw `bindgen` layer for low-level or advanced use.
- `vision_ui::driver`: Rust-native driver traits inspired by `embedded-hal`.

The safe wrapper covers the documented public APIs in `docs/api.md`, including:

- Lifecycle APIs
- UI tree builder APIs
- Notifications and alerts
- Font configuration and allocator APIs
- Driver contract APIs (`vision_ui_draw_driver.h`)

## Usage

```toml
[dependencies]
vision-ui = { path = "bindings/rust" }
```

```rust
use vision_ui as vui;
use std::time::Duration;

let mut ui = vui::VisionUi::new()?;
let root = ui.list("VisionUI", 8)?;
let toggle = ui.switch_with("Backlight", true, |ui, enabled| {
    let message = if enabled { "Backlight on" } else { "Backlight off" };
    let _ = ui.notify(message, Duration::from_millis(800));
})?;

ui.push(root, toggle);
ui.set_root(root);
ui.initialize_runtime();
ui.show();
ui.notify("Hello", Duration::from_millis(1200))?;
# Ok::<(), vui::Error>(())
```

## Regeneration

From `bindings/rust`:

```sh
bindgen ../../include/vision/vision_ui.h \
  --raw-line '#![allow(non_camel_case_types, non_snake_case, non_upper_case_globals)]' \
  --allowlist-function 'vision_ui_.*' \
  --allowlist-type 'vision_.*' \
  --allowlist-var 'IS_IN_VISION_UI|DEFAULT_LIST_ICON|UiAction.*|ListItem|TitleItem|IconItem|SwitchItem|SliderItem|UserItem|VisionAlloc.*' \
  --merge-extern-blocks \
  --rust-edition 2021 \
  -o src/bindings.rs \
  -- -I../../include
```

## Notes

- The top-level API uses Rust-style names and `Duration` instead of C-style setters and raw integer spans.
- Item callbacks are closure-based through `switch_with`, `slider_with`, and `scene_with`.
- The wrapper retains strings and bitmap buffers that the C API borrows.
- Advanced or unsupported operations are still available through `vision_ui::raw`.
- Your application still needs to link against the Vision UI C library.
