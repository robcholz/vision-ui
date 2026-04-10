# vision-ui

Safe Rust bindings for the Vision UI C library.

## Coverage

This crate now provides safe wrappers for **all public Vision UI C headers**, including:

- Core/runtime APIs
- Driver APIs (`vision_ui_draw_driver.h`)
- Animation APIs
- Item/menu tree APIs
- Renderer/icon APIs

## Usage

```toml
[dependencies]
vision-ui = { path = "bindings/rust" }
```

```rust
use vision_ui as vui;

vui::vision_ui_render_init_safe();
vui::vision_ui_core_init_safe();
vui::vision_ui_notification_push_safe("Hello", 1200)?;
```

## Examples

- `examples/lifecycle.rs`: initialize/render loop basics + notifications.
- `examples/menu_tree.rs`: create a root list, add a switch item, and bind selector.

## Notes

- Wrapper functions are safe to call from Rust (no `unsafe` required at call sites).
- Internally-retained storage is used for text/logo pointers that the C API keeps.
- Your application still needs to link against the Vision UI C library.
