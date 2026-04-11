# vision-ui

Idiomatic Rust crate for Vision UI, with the C core built by Cargo.

## Coverage

This crate provides:

- `vision_ui::VisionUi`: a safe, Rust-style wrapper around a `vision_ui_t` instance.
- `vision_ui::UiRef`: a callback-safe handle for closures invoked by the UI runtime.
- `vision_ui::config`: grouped Rust config for the compile-time knobs the native code actually uses.
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

By default the crate builds the bundled Vision UI core with the `preset-240x240` native configuration.

Available preset features:

- `preset-240x240` (default)
- `preset-128x64`

Other build features:

- `alloc` to enable closure-based callback helpers (`switch_with_closure`, `slider_with_closure`, `scene_with_closure`)
- `debug-overlay` to compile the native C renderer with `DEBUG`, which draws helper outlines for key UI regions

Compile-time config can be overridden from Cargo with environment variables, for example:

```sh
VISION_UI_SCREEN_WIDTH=128 \
VISION_UI_SCREEN_HEIGHT=64 \
cargo build
```

Precedence is:

1. explicit environment variables
2. selected preset feature
3. header defaults in the bundled native source

For the debug overlay, either enable the Cargo feature:

```sh
cargo build --features debug-overlay
```

or set:

```sh
VISION_UI_DEBUG=1 cargo build
```

The grouped Rust config surface then reflects those values through:

- `vision_ui::config::DISPLAY`
- `vision_ui::config::SYSTEM`
- `vision_ui::config::NOTIFICATION`
- `vision_ui::config::ALERT`
- `vision_ui::config::LIST_VIEW`
- `vision_ui::config::ICON_VIEW`

```rust
use vision_ui as vui;
use std::time::Duration;

struct BacklightState;

fn on_backlight_changed(ui: vui::UiRef, enabled: bool, _state: &'static BacklightState) {
    let message = if enabled {
        vui::text!("Backlight on")
    } else {
        vui::text!("Backlight off")
    };
    let _ = ui.notify(message, Duration::from_millis(800));
}

static BACKLIGHT_STATE: BacklightState = BacklightState;
static BACKLIGHT_BINDING: vui::ToggleBinding<BacklightState> =
    vui::ToggleBinding::new(&BACKLIGHT_STATE, on_backlight_changed);

let driver = /* your backend driver */;
let mut ui = vui::VisionUi::new(driver);
let root = ui.list(vui::text!("VisionUI"), 8) ?;
let toggle = ui.switch_with(vui::text!("Backlight"), true, & BACKLIGHT_BINDING) ?;

ui.push(root, toggle);
ui.set_root(root);
ui.initialize_runtime();
ui.initialize_rendering();
ui.notify(vui::text!("Hello"), Duration::from_millis(1200)) ?;
# Ok::<(), vui::Error>(())
```

## Native Build

- `build.rs` generates the raw FFI module and config constants into `OUT_DIR`.
- `build.rs` compiles the bundled Vision UI core sources under `native/`.
- The crate does not bundle a specific driver implementation.

## Notes

- The top-level API uses Rust-style names and `Duration` instead of C-style setters and raw integer spans.
- Bitmap inputs are validated with typed wrappers such as `MonoBitmap` and `StartupLogo`.
- Item callbacks use static bindings through `ToggleBinding`, `SlideBinding`, and `SceneBindings` by default.
- Enable the `alloc` feature for closure-based convenience helpers.
- Long-lived text and bitmap assets are passed as static borrowed data instead of being copied into hidden heap storage.
- Advanced or unsupported operations are still available through `vision_ui::raw`.
- `native/` is the packaged copy of the core C sources. Refresh it from the repo root with
  `bindings/rust/scripts/sync-native.sh`.
- The crate links the bundled Vision UI core automatically, but driver implementations remain outside this crate.
