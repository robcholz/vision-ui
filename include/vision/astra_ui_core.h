//
// Created by forpaindream on 25-1-20.
//

#ifndef ASTRA_UI_CORE_H
#define ASTRA_UI_CORE_H
#include <stdbool.h>

#define ALLOW_EXIT_ASTRA_UI_BY_USER 1 //允许用户在最浅层级退出astra ui lite

#ifdef __cplusplus
extern "C" {

#endif

extern bool in_astra;

extern void vision_ui_render_init();

extern void astra_refresh_info_bar();

extern void astra_refresh_pop_up();

extern void astra_refresh_camera_position();

extern void astra_refresh_widget_core_position();

extern void astra_init_list();

extern void astra_init_core();

extern void astra_refresh_list_item_position();

extern void astra_refresh_selector_position();

extern void astra_refresh_main_core_position();

extern void vision_ui_render_loop();

extern void astra_ui_widget_core();

extern void astra_ui_main_core();

extern bool vision_ui_is_exited();

#ifdef __cplusplus
}
#endif

#endif //ASTRA_UI_CORE_H
