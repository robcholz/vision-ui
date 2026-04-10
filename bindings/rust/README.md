# vision-ui

Safe Rust bindings for the Vision UI C library.

## What this crate provides

- Rust-friendly APIs for core Vision UI lifecycle and notification/alert calls.
- No `unsafe` required at call sites.
- Internal lifetime management for C pointers used by Vision UI.

## Usage

```toml
[dependencies]
vision-ui = { path = "bindings/rust" }
```

```rust
use vision_ui as vui;

vui::render_init();
vui::core_init();
vui::push_notification("Hello", 1200)?;
```

## Notes

- This crate wraps selected public Vision UI C APIs.
- It retains message/logo storage internally to satisfy C pointer lifetime requirements.
- Your application still needs to link against the Vision UI C library.
