//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//

#ifndef VISION_UI_VISION_UI_RENDERER_H
#define VISION_UI_VISION_UI_RENDERER_H

#include <stddef.h>
#include <stdint.h>

extern void vision_ui_exit_animation_render(float delta_ms);

extern void vision_ui_enter_animation_render(float delta_ms);

extern void vision_ui_widget_render();

extern void vision_ui_list_render();

typedef struct vision_ui_list_icon_t {
    uint8_t* list_header;
    uint8_t* switch_header;
    uint8_t* slider_header;
    uint8_t* default_header;

    uint8_t* switch_on_footer;
    uint8_t* switch_off_footer;
    uint8_t* slider_footer;

    size_t header_width;
    size_t header_height;

    size_t footer_width;
    size_t footer_height;
} vision_ui_icon_t;

extern vision_ui_icon_t DEFAULT_LIST_ICON;

extern void vision_ui_list_icon_set(vision_ui_icon_t icon);

extern vision_ui_icon_t vision_ui_list_icon_get_current();

#endif // VISION_UI_VISION_UI_RENDERER_H
