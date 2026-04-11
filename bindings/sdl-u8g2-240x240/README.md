# vision-ui-simulator-240x240

SDL + `u8g2` simulator backend for `vision-ui`.

This crate is intentionally separate from `vision-ui` so the core UI crate does not
pull in SDL or `u8g2` unless you opt into this backend explicitly.

## Usage

```toml
[dependencies]
vision-ui = { path = "../rust" }
vision-ui-simulator-240x240 = { path = "." }
```

```rust
use vision_ui as vui;
use vision_ui_simulator_240x240::Simulator240x240;

let driver = Simulator240x240::new()?;
let mut ui = vui::VisionUi::new(driver);
# Ok::<(), Box<dyn std::error::Error>>(())
```
