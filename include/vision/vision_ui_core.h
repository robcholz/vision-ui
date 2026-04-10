//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//

#ifndef VISION_UI_VISION_UI_CORE_H
#define VISION_UI_VISION_UI_CORE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct vision_ui_t vision_ui_t;

typedef struct vision_ui_font_t {
    const void* font;
    int8_t top_compensation;
    int8_t bottom_compensation;
} vision_ui_font_t;

typedef struct vision_ui_list_icon_t {
    const uint8_t* list_header;
    const uint8_t* switch_header;
    const uint8_t* slider_header;
    const uint8_t* default_header;

    const uint8_t* switch_on_footer;
    const uint8_t* switch_off_footer;
    const uint8_t* slider_footer;

    size_t header_width;
    size_t header_height;

    size_t footer_width;
    size_t footer_height;
} vision_ui_icon_t;

typedef enum vision_ui_action_t {
    UiActionNone,
    UiActionGoPrev,
    UiActionGoNext,
    UiActionEnter,
    UiActionExit,
} vision_ui_action_t;

extern void vision_ui_init(vision_ui_t* ui);

extern vision_ui_t* vision_ui_create();

extern void vision_ui_destroy(vision_ui_t* ui);

extern void vision_ui_render_init(vision_ui_t* ui);

extern void vision_ui_core_init(vision_ui_t* ui);

extern void vision_ui_start_logo_set(vision_ui_t* ui, const uint8_t* bmp, uint32_t span);

extern void vision_ui_step_render(vision_ui_t* ui);

extern bool vision_ui_is_exited(const vision_ui_t* ui);

extern bool vision_ui_is_background_frozen(const vision_ui_t* ui);

#endif // VISION_UI_VISION_UI_CORE_H
