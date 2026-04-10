//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//

#ifndef VISION_UI_VISION_UI_RENDERER_H
#define VISION_UI_VISION_UI_RENDERER_H

#include <stddef.h>
#include <stdint.h>

#include "vision_ui_core.h"

extern void vision_ui_exit_animation_render(vision_ui_t* ui, float delta_ms);

extern void vision_ui_enter_animation_render(vision_ui_t* ui, float delta_ms);

extern void vision_ui_widget_render(vision_ui_t* ui);

extern void vision_ui_list_render(vision_ui_t* ui);

extern vision_ui_icon_t DEFAULT_LIST_ICON;

extern void vision_ui_list_icon_set(vision_ui_t* ui, vision_ui_icon_t icon);

extern vision_ui_icon_t vision_ui_list_icon_get_current(const vision_ui_t* ui);

#endif // VISION_UI_VISION_UI_RENDERER_H
