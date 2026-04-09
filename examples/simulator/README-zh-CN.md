# Simulator 示例

本页说明如何为 Vision UI 构建并运行基于 SDL2 的桌面 simulator。

## 依赖

- [xmake](https://xmake.io)
- SDL2 开发头文件
- 本地放在 `components/u8g2` 下的 [u8g2](https://github.com/olikraus/u8g2) 仓库

在 macOS 上：

```bash
brew install xmake sdl2
```

在 Ubuntu 或 Debian 上：

```bash
sudo apt-get update
sudo apt-get install -y xmake libsdl2-dev
```

如果 `components/u8g2` 还不存在，先拉取：

```bash
mkdir -p components
git clone --depth=1 https://github.com/olikraus/u8g2.git components/u8g2
```

## 构建与运行

```bash
xmake f --simulator=y
xmake
xmake run vision_ui_simulator
```

## 控制方式

| 按键            | 作用                    |
|---------------|-----------------------|
| `Up` / `Down` | 在项目间移动，或在已确认的滑块上调整数值。 |
| `Space`       | 进入列表、切换开关，或确认当前项。     |
| `Esc`         | 退出当前层级，或者在允许时退出整个 UI。 |

[`main.cpp`](../../main.cpp) 也是一个很好的实际接入参考。
