# Simulator Example

This example shows how to build and run the SDL2 desktop simulator for Vision UI.

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

If `components/u8g2` is missing, fetch it first:

```bash
mkdir -p components
git clone --depth=1 https://github.com/olikraus/u8g2.git components/u8g2
```

## Build And Run

```bash
xmake f --simulator=y
xmake
xmake run vision_ui_simulator
```

## Controls

| Key           | Action                                                                |
|---------------|-----------------------------------------------------------------------|
| `Up` / `Down` | Move between items or adjust a confirmed slider                       |
| `Space`       | Enter a list, toggle a switch, or confirm the current item            |
| `Esc`         | Exit the current layer or leave the UI when top-level exit is enabled |

The simulator demo in [`main.cpp`](../../main.cpp) is also a good
reference for real usage.
