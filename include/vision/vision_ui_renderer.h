//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//

#ifndef VISION_UI_VISION_UI_RENDERER_H
#define VISION_UI_VISION_UI_RENDERER_H

// 完成完整的退场动画 VISION_UI_EXIT_ANIMATION_STATUS 的取值依次如下
typedef enum {
    EXIT_MASK_FALL = 0, // 0 触发退场动画 遮罩开始落下
    EXIT_MASK_FALL_COMPLETE = 1, // 1 遮罩落下完成 此时屏幕被遮罩填满 开始变更背景内容
    EXIT_MASK_RISE = 2, // 2 遮罩开始抬升
    // 0 遮罩抬升完成 退场动画完成
} vision_ui_exit_animation_status_t;

extern vision_ui_exit_animation_status_t vision_ui_exit_animation_status_get();

extern void vision_ui_exit_animation_render();

struct vision_ui_page_t;

extern void vision_ui_widget_render();

extern void vision_ui_list_render();

extern void vision_ui_icon_render(const struct vision_ui_page_t *page);

#endif // VISION_UI_VISION_UI_RENDERER_H
