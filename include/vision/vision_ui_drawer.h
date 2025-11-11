//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//

#ifndef VISION_UI_VISION_UI_DRAWER_H
#define VISION_UI_VISION_UI_DRAWER_H

#include "vision_ui_item.h"

#ifdef __cplusplus
extern "C" {
#endif

extern uint8_t ASTRA_EXIT_ANIMATION_STATUS;

extern void astra_draw_exit_animation();

extern void astra_draw_info_bar();

extern void astra_draw_pop_up();

extern void astra_draw_list_appearance();

extern void astra_draw_list_item();

extern void astra_draw_selector();

extern void astra_draw_widget();

extern void astra_draw_list();

#ifdef __cplusplus
}
#endif

#endif //VISION_UI_VISION_UI_DRAWER_H
