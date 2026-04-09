# 项目结构

本页说明仓库中的主要目录和文件。

## 顶层结构

- `include/vision/`：core、item model、renderer 和 driver interface 的公开头文件。
- `include/vision_ui_config.h`：编译期布局与动画配置。
- `src/`：core、renderer、动画辅助和 item 系统的运行时实现。
- `src/driver/`：面向 simulator 的 u8g2 驱动适配层。
- `main.cpp`：simulator 演示程序。
- `docs/`：面向使用者的文档和截图。
- `xmake.lua`：库和 simulator 的构建定义。

## 外部依赖

- `components/u8g2/`：simulator 构建所需的本地 u8g2 仓库。

构建 simulator 时，仓库默认假设本地存在 `components/u8g2`。

## 推荐阅读起点

- [`../README.md`](../README.md)：项目总览与入口说明。
- [`migration-zh-CN.md`](migration-zh-CN.md)：如何把驱动层迁移到新后端。
- [`api-zh-CN.md`](api-zh-CN.md)：公开 API 说明。
- [`config-zh-CN.md`](config-zh-CN.md)：配置常量说明。
- [`../main.cpp`](../main.cpp)：完整可运行示例。
