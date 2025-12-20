//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//

#ifndef VISION_UI_VISION_UI_CORE_H
#define VISION_UI_VISION_UI_CORE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct vision_ui_font_t {
    const void* font;
    int8_t top_compensation;
    int8_t bottom_compensation;
} vision_ui_font_t;

typedef enum vision_ui_action_t {
    UiActionNone,
    UiActionGoPrev,
    UiActionGoNext,
    UiActionEnter,
    UiActionExit,
} vision_ui_action_t;

extern bool IS_IN_VISION_UI;

extern void vision_ui_render_init();

extern void vision_ui_core_init();

extern void vision_ui_start_logo_set(const uint8_t* bmp, uint32_t span);

extern void vision_ui_step_render();

extern bool vision_ui_is_exited();

extern bool vision_ui_is_background_frozen();

#endif // VISION_UI_VISION_UI_CORE_H
