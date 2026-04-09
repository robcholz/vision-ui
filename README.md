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
  guide in [`docs/config.md`](docs/config.md).
- SDL2 simulator target for local development and visual iteration.

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

## Documentation

- [`docs/migration.md`](docs/migration.md): driver migration guide for porting `vision_ui_draw_driver.h` to a new backend.
- [`docs/api.md`](docs/api.md): public API reference grouped by task.
- [`docs/config.md`](docs/config.md): configuration reference for layout, spacing, and animation constants.
- [`docs/project-layout.md`](docs/project-layout.md): repository structure and where each major piece lives.

The simulator demo in [`main.cpp`](main.cpp) is also a good reference for real usage.

`components/u8g2` is treated as an external dependency and is not managed here through submodule commands.

## License

This project is licensed under `GPL-2.0-only`.
