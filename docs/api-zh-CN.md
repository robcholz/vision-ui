# Vision UI API 参考

本文用较直白的方式解释 Vision UI 对外导出的 API。第一次接入库时，建议按下面顺序阅读：

1. 生命周期
2. 构建 UI 树
3. 通知与警告
4. 驱动契约

如果你正在移植后端驱动，请先看 [`migration-zh-CN.md`](migration-zh-CN.md)。

头文件位置：

- [`include/vision/vision_ui_core.h`](../include/vision/vision_ui_core.h)
- [`include/vision/vision_ui_item.h`](../include/vision/vision_ui_item.h)
- [`include/vision/vision_ui_draw_driver.h`](../include/vision/vision_ui_draw_driver.h)
- [`include/vision/vision_ui_renderer.h`](../include/vision/vision_ui_renderer.h)

## 典型流程

大多数应用会按下面的顺序使用 Vision UI：

1. 通过 `vision_ui_driver_bind(...)` 绑定绘制驱动。
2. 通过 `vision_ui_font_set...(...)` 配置字体。
3. 创建根列表和子项。
4. 调用 `vision_ui_root_item_set(root)`。
5. 调用 `vision_ui_core_init()`。
6. 调用 `vision_ui_render_init()`。
7. 在主循环中清空缓冲区，调用 `vision_ui_step_render()`，然后发送缓冲区。

示例：

```cpp
vision_ui_font_t title_font = { .font = title_ptr, .top_compensation = 0, .bottom_compensation = 0 };
vision_ui_font_t subtitle_font = { .font = subtitle_ptr, .top_compensation = 0, .bottom_compensation = 0 };
vision_ui_font_t body_font = { .font = body_ptr, .top_compensation = 0, .bottom_compensation = 0 };

vision_ui_driver_bind(&driver);

vision_ui_font_set_title(title_font);
vision_ui_font_set_subtitle(subtitle_font);
vision_ui_font_set(body_font);

vision_ui_list_item_t* root = vision_ui_list_item_new(8, false, "VisionUI");
vision_ui_root_item_set(root);

vision_ui_list_push_item(root, vision_ui_list_title_item_new("VisionUI"));
vision_ui_list_push_item(root, vision_ui_list_switch_item_new("Invert Display", false, on_invert_changed));

vision_ui_core_init();
vision_ui_render_init();

while (!vision_ui_is_exited()) {
    vision_ui_driver_buffer_clear();
    vision_ui_step_render();
    vision_ui_driver_buffer_send();
}
```

## 核心类型

### `vision_ui_font_t`

配置字体时使用这个结构体：

```cpp
vision_ui_font_t font = {
    .font = your_font_pointer,
    .top_compensation = 0,
    .bottom_compensation = 0,
};
```

字段说明：

- `font`：后端字体对象，例如 u8g2 字体指针。
- `top_compensation`：当字体整体偏下时，用于向上修正文本度量。
- `bottom_compensation`：当字体裁切或整体偏上时，用于向下修正文本度量。

### `vision_ui_action_t`

这个枚举由驱动输入钩子返回：

- `UiActionNone`
- `UiActionGoPrev`
- `UiActionGoNext`
- `UiActionEnter`
- `UiActionExit`

## 生命周期

这些函数控制初始化和每帧执行。

| 函数                                                          | 作用                         | 何时调用             |
|---------------------------------------------------------------|------------------------------|----------------------|
| `vision_ui_render_init()`                                     | 标记 UI 已激活并初始化渲染器。 | 主渲染循环开始前一次。 |
| `vision_ui_core_init()`                                       | 初始化选择状态、相机状态和列表运行时状态。 | 根树构建完成后。 |
| `vision_ui_start_logo_set(const uint8_t* bmp, uint32_t span)` | 显示一张固定时长的启动位图。 | 可选，渲染开始前。 |
| `vision_ui_step_render()`                                     | 执行一帧 UI 逻辑与绘制。 | 每一帧都调用。 |
| `vision_ui_is_exited()`                                       | 返回 UI 是否已经关闭。 | 常用作主循环条件。 |
| `vision_ui_is_background_frozen()`                            | 返回后台交互是否应暂停。 | 可选，主要给自定义场景使用。 |

## 构建 UI 树

Vision UI 使用 `vision_ui_list_item_t` 节点树组织界面。每一个页面都是一个列表，列表中的每一行也是一个 item。

对于容器类 item，`capacity` 表示“这个 item 最多能容纳多少个直接子项”。

### 根节点与树辅助函数

| 函数                                                                                    | 作用           |
|-----------------------------------------------------------------------------------------|----------------|
| `vision_ui_root_item_set(vision_ui_list_item_t* item)`                                  | 设置顶层列表。 |
| `vision_ui_root_list_get()`                                                             | 返回当前根列表。 |
| `vision_ui_list_push_item(vision_ui_list_item_t* parent, vision_ui_list_item_t* child)` | 向父列表添加一个子项。 |

### Item 构造函数

| 函数                                                                                                                                          | 用途 |
|---------------------------------------------------------------------------------------------------------------------------------------------------|------|
| `vision_ui_list_item_new(size_t capacity, bool icon_mode, const char* content)`                                                                   | 创建普通列表容器，可持有子项。`icon_mode` 为 `true` 时表示图标视图列表。 |
| `vision_ui_list_title_item_new(const char* title)`                                                                                                | 创建不可交互的标题行。 |
| `vision_ui_list_icon_item_new(size_t capacity, const uint8_t* icon, const char* title, const char* description)`                                  | 创建可选中的图标卡片，也可以带子项。 |
| `vision_ui_list_switch_item_new(const char* content, bool default_value, void (*on_changed)(bool))`                                               | 创建开关行，值变化时会调用回调。 |
| `vision_ui_list_slider_item_new(const char* content, int16_t default_value, uint8_t step, int16_t min, int16_t max, void (*on_changed)(int16_t))` | 创建数值滑块行。 |
| `vision_ui_list_user_item_new(const char* content, void (*init_function)(), void (*loop_function)(), void (*exit_function)())`                    | 创建完全自定义的全屏场景，选中后接管绘制流程。 |

### User Item

`vision_ui_list_user_item_new(...)` 是完整自定义页面的出口。

- `init_function`：用户项第一次激活时调用。
- `loop_function`：用户项激活期间每帧调用。
- `exit_function`：离开用户项时调用。

在 user item 内部，你仍然使用普通渲染器所用的 `vision_ui_driver_*` 函数进行绘制。

### 类型辅助函数

这些函数用于把通用的 `vision_ui_list_item_t*` 转成更具体的 item 类型：

- `vision_ui_to_list_switch_item(...)`
- `vision_ui_to_list_slider_item(...)`
- `vision_ui_to_list_icon_item(...)`
- `vision_ui_to_list_user_item(...)`

除非你要在默认回调之外做自定义逻辑，否则通常不需要它们。

## 通知与警告

这些函数会在当前界面上方显示临时消息。

| 函数                                                              | 作用 |
|-------------------------------------------------------------------|------|
| `vision_ui_notification_push(const char* content, uint16_t span)` | 显示一条持续 `span` 毫秒的通知条。 |
| `vision_ui_alert_push(const char* content, uint16_t span)`        | 显示一条居中的警告，持续 `span` 毫秒。 |

`vision_ui_item.h` 中还有一些实例 getter：

- `vision_ui_notification_instance_get()`
- `vision_ui_notification_mutable_instance_get()`
- `vision_ui_alert_instance_get()`
- `vision_ui_alert_mutable_instance_get()`

大多数应用不需要直接使用这些 getter。只有在你要扩展渲染器行为时，它们才比较有用。

## 字体与内存

### 字体配置

| 函数                                                 | 作用 |
|------------------------------------------------------|------|
| `vision_ui_minifont_set(vision_ui_font_t font)`      | 设置小号辅助字体。 |
| `vision_ui_font_set(vision_ui_font_t font)`          | 设置主正文字体。 |
| `vision_ui_font_set_title(vision_ui_font_t font)`    | 设置标题字体。 |
| `vision_ui_font_set_subtitle(vision_ui_font_t font)` | 设置副标题字体。 |
| `vision_ui_minifont_get()`                           | 返回当前小号辅助字体。 |
| `vision_ui_font_get()`                               | 返回当前正文字体。 |
| `vision_ui_font_get_title()`                         | 返回当前标题字体。 |
| `vision_ui_font_get_subtitle()`                      | 返回当前副标题字体。 |

### 自定义分配器

`vision_ui_allocator_set(...)` 允许你替换默认内存分配钩子。

```cpp
void vision_ui_allocator_set(
    void* (*allocator)(vision_alloc_op_t op, size_t size, size_t count, void* ptr)
);
```

`vision_alloc_op_t` 用来告诉你的分配器当前要执行什么操作：

- `VisionAllocMalloc`
- `VisionAllocCalloc`
- `VisionAllocFree`

除非你需要在嵌入式环境里精确控制内存，否则通常可以忽略这部分。

## 驱动契约

要让 Vision UI 运行在新的平台上，你需要实现 `vision_ui_draw_driver.h` 中定义的函数。

### 输入与时间

| 函数                                  | 作用 |
|---------------------------------------|------|
| `vision_ui_driver_action_get()`       | 返回当前的高层用户动作。 |
| `vision_ui_driver_ticks_ms_get()`     | 返回单调递增的毫秒时间。 |
| `vision_ui_driver_delay(uint32_t ms)` | 休眠或让出约 `ms` 毫秒。 |
| `vision_ui_driver_bind(void* driver)` | 保存后端驱动句柄。 |

### 字体与文本

| 函数                                               | 作用 |
|----------------------------------------------------|------|
| `vision_ui_driver_font_set(vision_ui_font_t font)` | 在后端中激活字体。 |
| `vision_ui_driver_font_get()`                      | 返回当前激活的后端字体。 |
| `vision_ui_driver_str_draw(...)`                   | 绘制普通文本。 |
| `vision_ui_driver_str_utf8_draw(...)`              | 绘制 UTF-8 文本。 |
| `vision_ui_driver_str_width_get(...)`              | 返回普通文本宽度。 |
| `vision_ui_driver_str_utf8_width_get(...)`         | 返回 UTF-8 文本宽度。 |
| `vision_ui_driver_str_height_get()`                | 返回当前文本高度。 |
| `vision_ui_driver_font_mode_set(uint8_t mode)`     | 切换透明或实心文本绘制模式。 |
| `vision_ui_driver_font_direction_set(uint8_t dir)` | 切换文本方向。 |

### 基础图元

| 函数                                         | 作用 |
|----------------------------------------------|------|
| `vision_ui_driver_pixel_draw(...)`           | 画一个像素。 |
| `vision_ui_driver_circle_draw(...)`          | 画圆形轮廓。 |
| `vision_ui_driver_disc_draw(...)`            | 画实心圆。 |
| `vision_ui_driver_box_r_draw(...)`           | 画带圆角的实心矩形。 |
| `vision_ui_driver_box_draw(...)`             | 画实心矩形。 |
| `vision_ui_driver_frame_draw(...)`           | 画矩形边框。 |
| `vision_ui_driver_frame_r_draw(...)`         | 画圆角矩形边框。 |
| `vision_ui_driver_line_h_draw(...)`          | 画水平线。 |
| `vision_ui_driver_line_v_draw(...)`          | 画垂直线。 |
| `vision_ui_driver_line_draw(...)`            | 画任意直线。 |
| `vision_ui_driver_line_h_dotted_draw(...)`   | 画水平虚线。 |
| `vision_ui_driver_line_v_dotted_draw(...)`   | 画垂直虚线。 |
| `vision_ui_driver_bmp_draw(...)`             | 绘制位图。 |
| `vision_ui_driver_color_draw(uint8_t color)` | 修改当前绘制颜色。 |

### 裁剪与缓冲区

| 函数                                     | 作用 |
|------------------------------------------|------|
| `vision_ui_driver_clip_window_set(...)`  | 将绘制限制在一个矩形区域内。 |
| `vision_ui_driver_clip_window_reset()`   | 取消裁剪窗口。 |
| `vision_ui_driver_buffer_clear()`        | 清空帧缓冲区。 |
| `vision_ui_driver_buffer_send()`         | 提交当前帧缓冲区。 |
| `vision_ui_driver_buffer_area_send(...)` | 如果后端支持，只更新缓冲区的一部分。 |
| `vision_ui_driver_buffer_pointer_get()`  | 返回原始完整缓冲区指针。 |

`vision_ui_driver_buffer_pointer_get()` 主要用于过渡和模糊效果。如果你的后端要完整支持 Vision UI，它必须返回一个有效的整帧缓冲区指针。
