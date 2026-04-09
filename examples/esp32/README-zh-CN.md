# ESP32 示例

Lumen 是一个可以直接参考的 Vision UI on ESP32 集成案例。
它把 Vision UI 作为设备界面层，运行在基于 ESP32-C3 的桌面硬件节点中，
并结合 240x240 屏幕、旋钮编码器、蜂鸣器、运动传感器和电源监测等硬件，
把电脑状态、游戏事件和系统反馈映射成一个可以长期放在桌面的实体交互设备。

在这个项目里，Vision UI 负责嵌入式设备上的交互界面，
外围固件与硬件栈则负责传感、控制和设备行为本身。
因此它适合作为 Vision UI 在 ESP32 实际产品中的落地参考，
而不只是一个桌面模拟器示例。

参考项目：

- https://github.com/robcholz/Lumen/tree/main/components/vision-ui
