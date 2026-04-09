![Vision UI Simulator](docs/cover.jpeg)

<h1 align="center">Vision UI</h1>
<div align="center">

<a href="https://github.com/robcholz/vision-ui/actions"><img src="https://img.shields.io/github/actions/workflow/status/robcholz/vision-ui/build.yml?label=CI&branch=main"/>
<a href="https://github.com/robcholz/vision-ui"><img src="https://img.shields.io/badge/ui-open--source-brightgreen"/>
<a href="https://github.com/robcholz/vision-ui/blob/main/LICENSE"><img src="https://img.shields.io/github/license/robcholz/vision-ui?color=2b9348" alt="License Badge"/></a>

<p align="center">
     <a href="README.md">English</a> | <a href="README-zh-CN.md">简体中文</a>
  </p>

<i>Like this project? Please consider giving it a Star ⭐️ to help it grow!</i>

</div>

Vision UI is a small list-based UI framework for embedded displays. It provides a menu tree, animated selection and
scrolling, notifications, alerts, and a driver abstraction so the same UI logic can run in a desktop simulator or on
device-specific backends. The core library is platform independent and can be integrated with any backend that
implements the drawing and input driver interface.

The repository includes an simulator as one example backend for prototyping menus and custom full-screen scenes
before integrating the library into firmware.

## Features

- Hierarchical list navigation with title rows, switches, sliders, icon items, and custom user-rendered scenes.
- Animated selector, camera tracking, text scrolling, enter/exit transitions, notifications, and alerts.
- Thin driver interface for input, text measurement, drawing primitives, clipping, and buffer submission.
- Configurable layout and timing constants in [`include/vision_ui_config.h`](include/vision_ui_config.h), with a tuning
  guide in [`docs/config.md`](docs/config.md).
- Platform-independent core with pluggable backends.
- Example backends for local simulation and embedded targets.

## Examples

- [`examples/simulator/README.md`](examples/simulator/README.md): build and run the SDL2 desktop simulator.
- [`examples/esp32/README.md`](examples/esp32/README.md): ESP32 integration example.

## Documentation

- [`docs/migration.md`](docs/migration.md): driver migration guide for porting `vision_ui_draw_driver.h` to a new
  backend.
- [`docs/api.md`](docs/api.md): public API reference grouped by task.
- [`docs/config.md`](docs/config.md): configuration reference for layout, spacing, and animation constants.
- [`docs/project-layout.md`](docs/project-layout.md): repository structure and where each major piece lives.

For a concrete desktop integration, see the simulator demo in [`main.cpp`](main.cpp).

## License

Copyright (C) 2026 Finn Sheng.

This project is licensed under `GPL-3.0-only`. See [`LICENSE`](LICENSE).
