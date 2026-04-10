# vision-ui

Safe Rust bindings for the Vision UI C library.

## Coverage

This crate follows the documented public APIs in `docs/api.md`, including:

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

vui::vision_ui_render_init();
vui::vision_ui_core_init();
vui::vision_ui_notification_push("Hello", 1200)?;
```

## Examples

- `examples/lifecycle.rs`: initialize/render loop basics + notifications.
- `examples/menu_tree.rs`: create a root list and add a switch item.

## Notes

- Wrapper functions are safe to call from Rust (no `unsafe` required at call sites).
- Internally-retained storage is used for text/logo pointers that the C API keeps.
- Your application still needs to link against the Vision UI C library.
