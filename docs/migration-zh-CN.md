# Vision UI 驱动迁移指南

本页只讨论如何移植 Vision UI 的驱动层。

适用场景：

- 你想把 simulator 后端替换成另一套图形库
- 你要接入单片机显示栈
- 你要接入自定义 framebuffer
- 你想换成另一种桌面渲染器

本页不解释：

- 如何构建 UI 树
- 每个公开 API 是做什么的
- 如何调节布局常量

这些内容分别在：

- [`api-zh-CN.md`](api-zh-CN.md)
- [`config-zh-CN.md`](config-zh-CN.md)

## 你要替换的是什么

参考后端在 [`../src/driver/u8g2.c`](../src/driver/u8g2.c)。

当你把 Vision UI 迁移到别的平台时，核心工作就是用你自己的 `vision_ui_driver_t` 描述符和
`vision_ui_driver_ops_t` 回调实现替换这个后端，并满足
[`vision_ui_draw_driver.h`](../include/vision/vision_ui_draw_driver.h) 定义的契约。

也就是说，这里的迁移指的是：

- 保留 Vision UI core
- 保留 renderer
- 保留你的菜单树
- 只替换驱动描述符和它的回调

## 参考文件

建议把这两个文件并排看：

- [`../include/vision/vision_ui_draw_driver.h`](../include/vision/vision_ui_draw_driver.h)
- [`../src/driver/u8g2.c`](../src/driver/u8g2.c)

头文件定义了你必须实现的契约，u8g2 文件给出了一个完整的后端实现示例。

## 驱动职责

驱动契约可以拆成七部分。

## 1. 输入映射

你的后端必须把平台输入转换成：

- `UiActionNone`
- `UiActionGoPrev`
- `UiActionGoNext`
- `UiActionEnter`
- `UiActionExit`

simulator 里的映射是：

- `Up` 映射为 previous
- `Down` 映射为 next
- `Space` 映射为 enter
- `Esc` 映射为 exit

在真实硬件上，这通常来自：

- 按键
- 旋钮编码器
- 触摸手势层

## 2. 时间

你的后端必须提供：

- `vision_ui_driver_ticks_ms_get(const vision_ui_t* ui)`
- `vision_ui_driver_delay(const vision_ui_t* ui, uint32_t ms)`

要求：

- tick 计数器必须单调递增
- 单位必须是真正的毫秒
- 较大的时间跳变会明显破坏动画质量

## 3. 字体选择与文本度量

你的后端必须实现：

- `vision_ui_driver_font_set(...)`
- `vision_ui_driver_font_get(const vision_ui_t* ui)`
- `vision_ui_driver_str_draw(...)`
- `vision_ui_driver_str_utf8_draw(...)`
- `vision_ui_driver_str_width_get(...)`
- `vision_ui_driver_str_utf8_width_get(...)`
- `vision_ui_driver_str_height_get(const vision_ui_t* ui)`
- `vision_ui_driver_font_mode_set(...)`
- `vision_ui_driver_font_direction_set(...)`

这是移植中最关键的一部分之一。

如果文本看起来不对，问题通常先出在这里，而不是布局常量上。

文本度量出错的典型症状：

- 选择器宽度不对
- 标签文本压到 footer
- 文本滚动触发得过早或过晚
- 标题和副标题在列表行内的位置不正确

## 4. 基础图元

你的后端必须提供渲染器需要的图元：

- 像素
- 圆和实心圆
- 实心矩形和圆角实心矩形
- 边框矩形和圆角边框矩形
- 水平线、垂直线和任意直线
- 虚线
- 单色位图绘制
- 绘制颜色切换

如果目标图形库不直接支持某个图元，应在驱动内部模拟它，而不是去改 Vision UI core。

## 5. 裁剪

你的后端必须实现：

- `vision_ui_driver_clip_window_set(...)`
- `vision_ui_driver_clip_window_reset(const vision_ui_t* ui)`

裁剪对以下场景是必需的：

- 滚动文本
- 某些列表渲染路径
- 图标视图中的文本包裹

常见裁剪问题：

- 长文本画出了选中行之外
- 描述文本溢出到相邻区域
- 边缘附近出现过渡残影

## 6. 缓冲区管理

你的后端必须实现：

- `vision_ui_driver_buffer_clear(const vision_ui_t* ui)`
- `vision_ui_driver_buffer_send(const vision_ui_t* ui)`
- `vision_ui_driver_buffer_area_send(const vision_ui_t* ui, ...)`
- `vision_ui_driver_buffer_pointer_get(const vision_ui_t* ui)`

所有驱动入口现在都把当前 `vision_ui_t` 实例作为第一个参数。其中最关键的是
`vision_ui_driver_buffer_pointer_get(const vision_ui_t* ui)`。

渲染器要求它返回一个可读的完整显示缓冲区指针。视觉效果，例如模糊和过渡逻辑，会依赖它。

如果目标平台无法提供完整且可读的整帧缓冲区，你应该尽早把这件事记录为该驱动的限制。

## 7. 后端句柄绑定

`vision_ui_init_driver(vision_ui_t* ui, const vision_ui_driver_t* driver)` 用来把平台相关的后端状态和回调表传进
Vision UI。

在 simulator 里，安装进去的是一个 `vision_ui_u8g2_driver_t` 上下文，它内部再持有 `u8g2_t*`。

在别的平台上，它可能是：

- display 对象
- framebuffer 上下文
- device handle
- renderer 状态结构体

上下文指针具体代表什么，由驱动实现自己决定；Vision UI 调什么，由 ops 表决定。

## 迁移检查清单

接入一个新后端时，建议按这个顺序推进：

1. 复制参考后端 [`../src/driver/u8g2.c`](../src/driver/u8g2.c) 的整体结构。
2. 用你平台上的等价类型和调用替换后端相关部分。
3. 先准备好后端上下文结构体，再通过 `vision_ui_init_driver(...)` 正确安装进去。
4. 让输入映射工作起来。
5. 让文本宽度和高度报告正确。
6. 让基础图元正确绘制。
7. 让裁剪工作起来。
8. 让整帧缓冲区的清空与发送工作起来。
9. 确认 `vision_ui_driver_buffer_pointer_get(const vision_ui_t* ui)` 返回有效的完整缓冲区。

不要一开始就去改 UI 树或布局常量，那样只会让后端问题更难隔离。

## 建议的 Bring-Up 测试

先用一个非常小的测试菜单：

- 一条标题行
- 一条开关行
- 一条滑块行

这样你可以快速验证：

- 输入映射
- 选择器移动
- 文本度量
- footer 渲染
- 裁剪

参考行为可以直接看 [`../main.c`](../main.c)。

## 常见驱动问题

### 什么都看不到

检查：

- 后端句柄是否已经绑定
- `buffer_clear` 和 `buffer_send` 是否真的作用到了显示设备
- 在你的后端里，绘制颜色 `1` 是否确实表示“可见像素”

### 文本宽度明显不对

检查：

- UTF-8 宽度函数和普通宽度函数是否分别正确
- 激活字体后是否真的被后端应用
- 文本高度是否基于和宽度测量相同的字体

### 滚动文本溢出自己的区域

检查：

- 裁剪窗口坐标
- 后端裁剪 API 的结束坐标是 inclusive 还是 exclusive

### 模糊或过渡效果异常

检查：

- `vision_ui_driver_buffer_pointer_get(const vision_ui_t* ui)`
- 返回的指针是否真的是整帧缓冲区，而不是局部 tile buffer

### 动画感觉不稳定

检查：

- tick 源是否单调递增
- 单位是否确实是毫秒
- 帧循环是否存在明显卡顿

## 一开始不要改什么

当迁移失败时，最开始不要去改：

- 菜单树
- item 构造函数
- renderer 内部
- 布局常量

如果后端契约本身是错的，这些修改只会掩盖真正的问题。

## 相关文件

- [`../include/vision/vision_ui_draw_driver.h`](../include/vision/vision_ui_draw_driver.h)：必须满足的驱动契约
- [`../src/driver/u8g2.c`](../src/driver/u8g2.c)：参考后端实现
- [`../main.c`](../main.c)：可运行的 simulator 示例
- [`api-zh-CN.md`](api-zh-CN.md)：公开 API 参考
- [`config-zh-CN.md`](config-zh-CN.md)：布局和时序配置
