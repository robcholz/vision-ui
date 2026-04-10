//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//

#ifndef VISION_UI_VISION_UI_CORE_H
#define VISION_UI_VISION_UI_CORE_H

#include "vision_ui_types.h"

extern void vision_ui_init(vision_ui_t* ui);

extern vision_ui_t* vision_ui_create();

extern void vision_ui_destroy(vision_ui_t* ui);

extern void vision_ui_render_init(vision_ui_t* ui);

extern void vision_ui_core_init(vision_ui_t* ui);

extern void vision_ui_start_logo_set(vision_ui_t* ui, const uint8_t* bmp, uint32_t span);

extern void vision_ui_step_render(vision_ui_t* ui);

extern bool vision_ui_is_exited(const vision_ui_t* ui);

extern bool vision_ui_is_background_frozen(const vision_ui_t* ui);

extern void vision_ui_allocator_set(vision_ui_t* ui, vision_ui_allocator_t allocator);

extern void vision_ui_minifont_set(vision_ui_t* ui, vision_ui_font_t font);

extern void vision_ui_font_set(vision_ui_t* ui, vision_ui_font_t font);

extern void vision_ui_font_set_title(vision_ui_t* ui, vision_ui_font_t font);

extern void vision_ui_font_set_subtitle(vision_ui_t* ui, vision_ui_font_t font);

extern vision_ui_font_t vision_ui_minifont_get(const vision_ui_t* ui);

extern vision_ui_font_t vision_ui_font_get(const vision_ui_t* ui);

extern vision_ui_font_t vision_ui_font_get_title(const vision_ui_t* ui);

extern vision_ui_font_t vision_ui_font_get_subtitle(const vision_ui_t* ui);

extern bool vision_ui_exit_animation_is_finished(const vision_ui_t* ui);

extern void vision_ui_exit_animation_set_is_finished(vision_ui_t* ui);

extern void vision_ui_exit_animation_start(vision_ui_t* ui);

extern bool vision_ui_enter_animation_is_finished(const vision_ui_t* ui);

extern void vision_ui_enter_animation_set_is_finished(vision_ui_t* ui);

extern void vision_ui_enter_animation_start(vision_ui_t* ui);

#endif // VISION_UI_VISION_UI_CORE_H
