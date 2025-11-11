//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//

#ifndef VISION_UI_VISION_UI_CORE_H
#define VISION_UI_VISION_UI_CORE_H

#include <stdbool.h>

typedef enum vision_ui_action_t {
    UI_ACTION_NONE,
    UI_ACTION_GO_PREV,
    UI_ACTION_GO_NEXT,
    UI_ACTION_ENTER,
    UI_ACTION_EXIT,
} vision_ui_action_t;

extern bool IS_IN_VISION_UI;

extern void vision_ui_render_init();

extern void vision_ui_info_bar_update();

extern void vision_ui_pop_up_update();

extern void vision_ui_camera_position_update();

extern void vision_ui_widget_core_position_update();

extern void vision_ui_list_init();

extern void vision_ui_core_init();

extern void vision_ui_list_item_position_update();

extern void vision_ui_selector_position_update();

extern void vision_ui_main_core_position_update();

extern void vision_ui_step_render();

extern void vision_ui_widget_core_step();

extern void vision_ui_main_core_step();

extern bool vision_ui_is_exited();

extern bool vision_ui_is_background_frozen();

#endif // VISION_UI_VISION_UI_CORE_H
