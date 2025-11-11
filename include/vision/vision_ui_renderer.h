//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//

#ifndef VISION_UI_VISION_UI_RENDERER_H
#define VISION_UI_VISION_UI_RENDERER_H

extern void vision_ui_exit_animation_render();

struct vision_ui_page_t;

extern void vision_ui_widget_render();

extern void vision_ui_list_render();

extern void vision_ui_icon_render(const struct vision_ui_page_t *page);

#endif // VISION_UI_VISION_UI_RENDERER_H
