# Vision UI API 参考

本文用较直白的方式解释 Vision UI 对外导出的 API。第一次接入库时，建议按下面顺序阅读：

1. 生命周期
2. 构建 UI 树
3. 通知与警告
4. 驱动契约

如果你正在移植后端驱动，请先看 [`migration-zh-CN.md`](migration-zh-CN.md)。

头文件位置：

- [`include/vision/vision_ui.h`](../include/vision/vision_ui.h)
- [`include/vision/vision_ui_types.h`](../include/vision/vision_ui_types.h)
- [`include/vision/vision_ui_core.h`](../include/vision/vision_ui_core.h)
- [`include/vision/vision_ui_item.h`](../include/vision/vision_ui_item.h)
- [`include/vision/vision_ui_draw_driver.h`](../include/vision/vision_ui_draw_driver.h)
- [`include/vision/vision_ui_renderer.h`](../include/vision/vision_ui_renderer.h)

## 典型流程

大多数应用会按下面的顺序使用 Vision UI：

1. 创建并初始化一个 `vision_ui_t` 实例。
2. 通过 `vision_ui_driver_bind(...)` 绑定绘制驱动。
3. 通过 `vision_ui_font_set...(...)` 配置字体。
4. 创建根列表和子项。
5. 调用 `vision_ui_root_item_set(&ui, root)`。
6. 调用 `vision_ui_core_init(&ui)`。
7. 调用 `vision_ui_render_init(&ui)`。
8. 在主循环中清空缓冲区，调用 `vision_ui_step_render(&ui)`，然后发送缓冲区。

示例：

```cpp
vision_ui_font_t title_font = { .font = title_ptr, .top_compensation = 0, .bottom_compensation = 0 };
vision_ui_font_t subtitle_font = { .font = subtitle_ptr, .top_compensation = 0, .bottom_compensation = 0 };
vision_ui_font_t body_font = { .font = body_ptr, .top_compensation = 0, .bottom_compensation = 0 };

vision_ui_t ui;
vision_ui_init(&ui);

vision_ui_driver_bind(&ui, &driver);

vision_ui_font_set_title(&ui, title_font);
vision_ui_font_set_subtitle(&ui, subtitle_font);
vision_ui_font_set(&ui, body_font);

vision_ui_list_item_t* root = vision_ui_list_item_new(&ui, 8, false, "VisionUI");
vision_ui_root_item_set(&ui, root);

vision_ui_list_push_item(&ui, root, vision_ui_list_title_item_new(&ui, "VisionUI"));
vision_ui_list_push_item(&ui, root, vision_ui_list_switch_item_new(&ui, "Invert Display", false, on_invert_changed, NULL));

vision_ui_core_init(&ui);
vision_ui_render_init(&ui);

while (!vision_ui_is_exited(&ui)) {
    vision_ui_driver_buffer_clear(&ui);
    vision_ui_step_render(&ui);
    vision_ui_driver_buffer_send(&ui);
}
```

## 核心类型

`vision_ui_types.h` 里的公开类型层包括 `vision_ui_t`、`vision_ui_font_t`、`vision_ui_action_t` 和
`vision_ui_icon_t`。

### `vision_ui_t`

`vision_ui_t` 保存一个 UI 实例的运行时状态：字体、根列表、选择器、相机、通知、警告、启动 Logo、
动画状态以及绑定的绘制驱动。凡是操作某个 UI 实例的公开 API，都把 `vision_ui_t*` 作为第一个参数。
只读查询会尽量使用 `const vision_ui_t*`。

如果实例由调用方保存，使用 `vision_ui_init(&ui)` 初始化；如果希望由库分配，使用
`vision_ui_create()` / `vision_ui_destroy(ui)`。

当你使用 `vision_ui_list_item_new(...)` 这一类 item 构造辅助函数时，`vision_ui_destroy(...)`
也会释放为同一个 `vision_ui_t` 实例创建的库自有列表项。

### `vision_ui_font_t`

这是一个公开的配置类型，供字体 setter/getter API 使用。

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

### `vision_ui_icon_t`

这是一个公开的配置类型，供 `vision_ui_list_icon_set(...)` 使用。

它包含列表 header/footer 使用的位图指针和共享尺寸：

- `list_header`、`switch_header`、`slider_header`、`default_header`：不同列表项样式的 header 位图。
- `switch_on_footer`、`switch_off_footer`、`slider_footer`：开关项和滑块项的 footer 位图。
- `header_width`、`header_height`：header 位图共用尺寸。
- `footer_width`、`footer_height`：footer 位图共用尺寸。

## 生命周期

这些函数控制初始化和每帧执行。

| 函数                                                                             | 作用                    | 何时调用           |
|--------------------------------------------------------------------------------|-----------------------|----------------|
| `vision_ui_init(vision_ui_t* ui)`                                              | 初始化调用方持有的 UI 实例。      | 使用实例前。         |
| `vision_ui_create()` / `vision_ui_destroy(vision_ui_t* ui)`                    | 分配或释放 UI 实例。          | 可选的堆分配生命周期。    |
| `vision_ui_render_init(vision_ui_t* ui)`                                       | 标记 UI 已激活并初始化渲染器。     | 主渲染循环开始前一次。    |
| `vision_ui_core_init(vision_ui_t* ui)`                                         | 初始化选择状态、相机状态和列表运行时状态。若还没有挂上 root，则返回 `VisionUiCoreInitRootItemNotSet`。 | 根树构建完成后。       |
| `vision_ui_start_logo_set(vision_ui_t* ui, const uint8_t* bmp, uint32_t span)` | 显示一张固定时长的启动位图。        | 可选，渲染开始前。      |
| `vision_ui_step_render(vision_ui_t* ui)`                                       | 执行一帧 UI 逻辑与绘制。        | 每一帧都调用。        |
| `vision_ui_is_exited(const vision_ui_t* ui)`                                   | 返回 UI 是否已经关闭。         | 常用作主循环条件。      |
| `vision_ui_is_background_frozen(const vision_ui_t* ui)`                        | 返回后台交互是否应暂停。          | 可选，主要给自定义场景使用。 |

## 构建 UI 树

Vision UI 使用 `vision_ui_list_item_t` 节点树组织界面。每一个页面都是一个列表，列表中的每一行也是一个 item。

对于容器类 item，`capacity` 表示“这个 item 最多能容纳多少个直接子项”。

### 根节点与树辅助函数

| 函数                                                                                                       | 作用          |
|----------------------------------------------------------------------------------------------------------|-------------|
| `vision_ui_root_item_set(vision_ui_t* ui, vision_ui_list_item_t* item)`                                  | 设置顶层列表。     |
| `vision_ui_root_list_get(const vision_ui_t* ui)`                                                         | 返回当前顶层列表；如果还没有设置 root，则返回 `NULL`。 |
| `vision_ui_list_push_item(vision_ui_t* ui, vision_ui_list_item_t* parent, vision_ui_list_item_t* child)` | 向父列表添加一个子项。 |

### Item 构造函数

| 函数                                                                                                                                                                                                                              | 用途                                             |
|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|------------------------------------------------|
| `vision_ui_list_item_new(const vision_ui_t* ui, size_t capacity, bool icon_mode, const char* content)`                                                                                                                          | 创建普通列表容器，可持有子项。`icon_mode` 为 `true` 时表示图标视图列表。 |
| `vision_ui_list_title_item_new(const vision_ui_t* ui, const char* title)`                                                                                                                                                       | 创建不可交互的标题行。                                    |
| `vision_ui_list_icon_item_new(const vision_ui_t* ui, size_t capacity, const uint8_t* icon, const char* title, const char* description)`                                                                                         | 创建可选中的图标卡片，也可以带子项。                             |
| `vision_ui_list_switch_item_new(const vision_ui_t* ui, const char* content, bool default_value, void (*on_changed)(vision_ui_t*, bool, void*), void* user_data)`                                                                | 创建开关行，值变化时会调用回调。                               |
| `vision_ui_list_slider_item_new(const vision_ui_t* ui, const char* content, int16_t default_value, uint8_t step, int16_t min, int16_t max, void (*on_changed)(vision_ui_t*, int16_t, void*), void* user_data)`                  | 创建数值滑块行。                                       |
| `vision_ui_list_user_item_new(const vision_ui_t* ui, const char* content, void (*init_function)(vision_ui_t*, void*), void (*loop_function)(vision_ui_t*, void*), void (*exit_function)(vision_ui_t*, void*), void* user_data)` | 创建完全自定义的全屏场景，选中后接管绘制流程。                        |

### User Item

`vision_ui_list_user_item_new(...)` 是完整自定义页面的出口。

- `init_function`：用户项第一次激活时调用。
- `loop_function`：用户项激活期间每帧调用。
- `exit_function`：离开用户项时调用。
- `user_data`：由调用方拥有的上下文指针。Vision UI 会保存这个指针，并在回调触发时原样传回。只要该 item 还可能触发回调，这块数据就必须保持有效。

在 user item 内部，你仍然使用普通渲染器所用的 `vision_ui_driver_*` 函数进行绘制。当前
`vision_ui_t*` 和保存下来的 `user_data` 指针都会传给每个 user item 回调。

## 通知与警告

这些函数会在当前界面上方显示临时消息。

| 函数                                                                                 | 作用                      |
|------------------------------------------------------------------------------------|-------------------------|
| `vision_ui_notification_push(vision_ui_t* ui, const char* content, uint16_t span)` | 显示一条持续 `span` 毫秒的通知条。若 `content` 为 `NULL`，则返回 `VisionUiNotificationPushContentInvalid`。   |
| `vision_ui_alert_push(vision_ui_t* ui, const char* content, uint16_t span)`        | 显示一条居中的警告，持续 `span` 毫秒。 |

## 字体与内存

### 字体配置

| 函数                                                                    | 作用          |
|-----------------------------------------------------------------------|-------------|
| `vision_ui_minifont_set(vision_ui_t* ui, vision_ui_font_t font)`      | 设置小号辅助字体。   |
| `vision_ui_font_set(vision_ui_t* ui, vision_ui_font_t font)`          | 设置主正文字体。    |
| `vision_ui_font_set_title(vision_ui_t* ui, vision_ui_font_t font)`    | 设置标题字体。     |
| `vision_ui_font_set_subtitle(vision_ui_t* ui, vision_ui_font_t font)` | 设置副标题字体。    |
| `vision_ui_minifont_get(const vision_ui_t* ui)`                       | 返回当前小号辅助字体。 |
| `vision_ui_font_get(const vision_ui_t* ui)`                           | 返回当前正文字体。   |
| `vision_ui_font_get_title(const vision_ui_t* ui)`                     | 返回当前标题字体。   |
| `vision_ui_font_get_subtitle(const vision_ui_t* ui)`                  | 返回当前副标题字体。  |

### 自定义分配器

`vision_ui_allocator_set(...)` 允许你替换默认内存分配钩子。

```cpp
void vision_ui_allocator_set(
    vision_ui_t* ui,
    void* (*allocator)(vision_alloc_op_t op, size_t size, size_t count, void* ptr)
);
```

`vision_alloc_op_t` 用来告诉你的分配器当前要执行什么操作：

- `VisionAllocMalloc`
- `VisionAllocCalloc`
- `VisionAllocFree`

除非你需要在嵌入式环境里精确控制内存，否则通常可以忽略这部分。
安装自定义分配器后，Vision UI 会用它分配构造函数创建的列表项，并在随后销毁这些库自有 item 时继续使用它。

## 驱动契约

要让 Vision UI 运行在新的平台上，你需要实现 `vision_ui_draw_driver.h` 中定义的函数。

### 输入与时间

| 函数                                                           | 作用              |
|--------------------------------------------------------------|-----------------|
| `vision_ui_driver_action_get(const vision_ui_t* ui)`         | 返回当前的高层用户动作。    |
| `vision_ui_driver_ticks_ms_get(const vision_ui_t* ui)`       | 返回单调递增的毫秒时间。    |
| `vision_ui_driver_delay(const vision_ui_t* ui, uint32_t ms)` | 休眠或让出约 `ms` 毫秒。 |
| `vision_ui_driver_bind(vision_ui_t* ui, void* driver)`       | 保存后端驱动句柄。       |

### 字体与文本

| 函数                                                                        | 作用             |
|---------------------------------------------------------------------------|----------------|
| `vision_ui_driver_font_set(vision_ui_t* ui, vision_ui_font_t font)`       | 在后端中激活字体。      |
| `vision_ui_driver_font_get(const vision_ui_t* ui)`                        | 返回当前激活的后端字体。   |
| `vision_ui_driver_str_draw(...)`                                          | 绘制普通文本。        |
| `vision_ui_driver_str_utf8_draw(...)`                                     | 绘制 UTF-8 文本。   |
| `vision_ui_driver_str_width_get(...)`                                     | 返回普通文本宽度。      |
| `vision_ui_driver_str_utf8_width_get(...)`                                | 返回 UTF-8 文本宽度。 |
| `vision_ui_driver_str_height_get(const vision_ui_t* ui)`                  | 返回当前文本高度。      |
| `vision_ui_driver_font_mode_set(const vision_ui_t* ui, uint8_t mode)`     | 切换透明或实心文本绘制模式。 |
| `vision_ui_driver_font_direction_set(const vision_ui_t* ui, uint8_t dir)` | 切换文本方向。        |

### 基础图元

| 函数                                                                  | 作用         |
|---------------------------------------------------------------------|------------|
| `vision_ui_driver_pixel_draw(...)`                                  | 画一个像素。     |
| `vision_ui_driver_circle_draw(...)`                                 | 画圆形轮廓。     |
| `vision_ui_driver_disc_draw(...)`                                   | 画实心圆。      |
| `vision_ui_driver_box_r_draw(...)`                                  | 画带圆角的实心矩形。 |
| `vision_ui_driver_box_draw(...)`                                    | 画实心矩形。     |
| `vision_ui_driver_frame_draw(...)`                                  | 画矩形边框。     |
| `vision_ui_driver_frame_r_draw(...)`                                | 画圆角矩形边框。   |
| `vision_ui_driver_line_h_draw(...)`                                 | 画水平线。      |
| `vision_ui_driver_line_v_draw(...)`                                 | 画垂直线。      |
| `vision_ui_driver_line_draw(...)`                                   | 画任意直线。     |
| `vision_ui_driver_line_h_dotted_draw(...)`                          | 画水平虚线。     |
| `vision_ui_driver_line_v_dotted_draw(...)`                          | 画垂直虚线。     |
| `vision_ui_driver_bmp_draw(...)`                                    | 绘制位图。      |
| `vision_ui_driver_color_draw(const vision_ui_t* ui, uint8_t color)` | 修改当前绘制颜色。  |

### 裁剪与缓冲区

| 函数                                                           | 作用                 |
|--------------------------------------------------------------|--------------------|
| `vision_ui_driver_clip_window_set(...)`                      | 将绘制限制在一个矩形区域内。     |
| `vision_ui_driver_clip_window_reset(const vision_ui_t* ui)`  | 取消裁剪窗口。            |
| `vision_ui_driver_buffer_clear(const vision_ui_t* ui)`       | 清空帧缓冲区。            |
| `vision_ui_driver_buffer_send(const vision_ui_t* ui)`        | 提交当前帧缓冲区。          |
| `vision_ui_driver_buffer_area_send(...)`                     | 如果后端支持，只更新缓冲区的一部分。 |
| `vision_ui_driver_buffer_pointer_get(const vision_ui_t* ui)` | 返回原始完整缓冲区指针。       |

`vision_ui_driver_buffer_pointer_get(const vision_ui_t* ui)` 主要用于过渡和模糊效果。如果你的后端要完整支持 Vision
UI，它必须返回一个有效的整帧缓冲区指针。

## 列表图标

渲染器在 `vision_ui_renderer.h` 中提供了一个小型图标主题入口。

| 函数                                                                | 作用            |
|-------------------------------------------------------------------|---------------|
| `vision_ui_list_icon_set(vision_ui_t* ui, vision_ui_icon_t icon)` | 替换当前列表图标包。    |
| `DEFAULT_LIST_ICON`                                               | 示例程序使用的内置图标包。 |

`vision_ui_icon_t` 包含列表 header/footer 的位图指针和尺寸。
