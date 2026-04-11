# Project Layout

This page describes the main directories and files in the repository.

## Top-Level Structure

- `include/vision/`: public headers for shared types, the core lifecycle, the item model, the renderer, and the driver
  interface.
- `include/vision_ui_config.h`: compile-time layout and animation settings.
- `src/`: runtime implementation for the core lifecycle, renderer/widgets, animation helpers, and item system.
- `src/driver/`: simulator-oriented u8g2 driver glue.
- `main.c`: simulator demo application.
- `docs/`: user-facing documentation and screenshots.
- `xmake.lua`: build definition for the library and simulator.

## External Dependency

- `components/u8g2/`: local checkout of the u8g2 project used by the simulator build.

This repository expects `components/u8g2` to exist locally when building the simulator.

## Good Starting Points

- [`../README.md`](../README.md) for the top-level overview and build steps.
- [`migration.md`](migration.md) for porting the driver layer to another backend.
- [`api.md`](api.md) for the exported API.
- [`config.md`](config.md) for configuration constants.
- [`../main.c`](../main.c) for a full working example.

- `bindings/rust/`: Rust package (`vision-ui`) exposing safe wrappers over key C APIs.
