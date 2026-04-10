//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//

#ifndef VISION_UI_VISION_UI_CORE_H
#define VISION_UI_VISION_UI_CORE_H

#include "vision_ui_types.h"

/// Initializes a caller-owned UI instance before first use.
extern void vision_ui_init(vision_ui_t* ui);

/// Allocates and initializes a heap-owned UI instance.
extern vision_ui_t* vision_ui_create();

/// Destroys a UI instance and any library-owned items created for it.
extern void vision_ui_destroy(vision_ui_t* ui);

/// Marks the UI as active and prepares the renderer to start drawing.
extern void vision_ui_render_init(vision_ui_t* ui);

/// Initializes selector, camera, and list runtime state after the tree is built.
extern void vision_ui_core_init(vision_ui_t* ui);

/// Shows a startup bitmap for a fixed duration before the normal UI loop takes over.
extern void vision_ui_start_logo_set(vision_ui_t* ui, const uint8_t* bmp, uint32_t span);

/// Runs one frame of UI logic and rendering.
extern void vision_ui_step_render(vision_ui_t* ui);

/// Returns whether the UI has completed its exit flow.
extern bool vision_ui_is_exited(const vision_ui_t* ui);

/// Returns whether background interaction should be paused behind the UI.
extern bool vision_ui_is_background_frozen(const vision_ui_t* ui);

/// Installs a custom allocator used for subsequent library-managed allocations.
extern void vision_ui_allocator_set(vision_ui_t* ui, vision_ui_allocator_t allocator);

/// Sets the small utility font.
extern void vision_ui_minifont_set(vision_ui_t* ui, vision_ui_font_t font);

/// Sets the main body font.
extern void vision_ui_font_set(vision_ui_t* ui, vision_ui_font_t font);

/// Sets the title font.
extern void vision_ui_font_set_title(vision_ui_t* ui, vision_ui_font_t font);

/// Sets the subtitle font.
extern void vision_ui_font_set_subtitle(vision_ui_t* ui, vision_ui_font_t font);

/// Returns the current small utility font.
extern vision_ui_font_t vision_ui_minifont_get(const vision_ui_t* ui);

/// Returns the current main body font.
extern vision_ui_font_t vision_ui_font_get(const vision_ui_t* ui);

/// Returns the current title font.
extern vision_ui_font_t vision_ui_font_get_title(const vision_ui_t* ui);

/// Returns the current subtitle font.
extern vision_ui_font_t vision_ui_font_get_subtitle(const vision_ui_t* ui);

extern bool vision_ui_exit_animation_is_finished(const vision_ui_t* ui);

extern void vision_ui_exit_animation_set_is_finished(vision_ui_t* ui);

extern void vision_ui_exit_animation_start(vision_ui_t* ui);

extern bool vision_ui_enter_animation_is_finished(const vision_ui_t* ui);

extern void vision_ui_enter_animation_set_is_finished(vision_ui_t* ui);

extern void vision_ui_enter_animation_start(vision_ui_t* ui);

#endif // VISION_UI_VISION_UI_CORE_H
