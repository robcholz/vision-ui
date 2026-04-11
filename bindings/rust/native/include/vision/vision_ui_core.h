//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//

#ifndef VISION_UI_VISION_UI_CORE_H
#define VISION_UI_VISION_UI_CORE_H

#include "vision_ui_types.h"

typedef enum vision_ui_core_init_result_t {
    /// The core runtime state was initialized successfully.
    VisionUiCoreInitOk = 0,
    /// Core initialization requires a root item to be attached first.
    VisionUiCoreInitRootItemNotSet,
} vision_ui_core_init_result_t;

/**
 * Initializes a caller-owned UI instance before first use.
 *
 * @param ui UI instance to initialize. Must not be `NULL`.
 */
extern void vision_ui_init(vision_ui_t* ui);

/**
 * Destroys a UI instance and any library-owned items created for it.
 *
 * @param ui UI instance to destroy, or `NULL` to do nothing.
 */
extern void vision_ui_destroy(vision_ui_t* ui);

/**
 * Marks the UI as active and prepares the renderer to start drawing.
 *
 * @param ui UI instance to activate. Must not be `NULL`.
 */
extern void vision_ui_render_init(vision_ui_t* ui);

/**
 * Initializes selector, camera, and list runtime state after the tree is built.
 *
 * The root tree should already be attached with `vision_ui_root_item_set()` before this is called.
 *
 * @param ui UI instance to initialize. Must not be `NULL`.
 * @return `VisionUiCoreInitOk` on success.
 * @return `VisionUiCoreInitRootItemNotSet` when no root item has been attached yet.
 */
extern vision_ui_core_init_result_t vision_ui_core_init(vision_ui_t* ui);

/**
 * Shows a startup bitmap for a fixed duration before the normal UI loop takes over.
 *
 * @param ui UI instance to configure. Must not be `NULL`.
 * @param bmp Monochrome bitmap pointer to display. The pointer is borrowed, not copied.
 * @param span Display duration in milliseconds.
 */
extern void vision_ui_start_logo_set(vision_ui_t* ui, const uint8_t* bmp, uint32_t span);

/**
 * Runs one frame of UI logic and rendering.
 *
 * @param ui UI instance to update and render. Must not be `NULL`.
 */
extern void vision_ui_step_render(vision_ui_t* ui);

/**
 * Returns whether the UI has completed its exit flow.
 *
 * @param ui UI instance to query. Must not be `NULL`.
 * @return `true` when the UI is no longer active; otherwise `false`.
 */
extern bool vision_ui_is_exited(const vision_ui_t* ui);

/**
 * Returns whether background interaction should be paused behind the UI.
 *
 * @param ui UI instance to query. Must not be `NULL`.
 * @return `true` when background interaction should be frozen; otherwise `false`.
 */
extern bool vision_ui_is_background_frozen(const vision_ui_t* ui);

/**
 * Installs a custom allocator used for subsequent library-managed allocations.
 *
 * Constructor-created items allocated after this call will use the supplied allocator, and `vision_ui_destroy()` will
 * free those library-owned items through the same allocator.
 *
 * @param ui UI instance to configure. Must not be `NULL`.
 * @param allocator Allocation callback to install, or `NULL` to use the default libc allocator.
 */
extern void vision_ui_allocator_set(vision_ui_t* ui, vision_ui_allocator_t allocator);

/**
 * Sets the small utility font.
 *
 * @param ui UI instance to configure. Must not be `NULL`.
 * @param font Font configuration to store.
 */
extern void vision_ui_minifont_set(vision_ui_t* ui, vision_ui_font_t font);

/**
 * Sets the main body font.
 *
 * @param ui UI instance to configure. Must not be `NULL`.
 * @param font Font configuration to store.
 */
extern void vision_ui_font_set(vision_ui_t* ui, vision_ui_font_t font);

/**
 * Sets the title font.
 *
 * @param ui UI instance to configure. Must not be `NULL`.
 * @param font Font configuration to store.
 */
extern void vision_ui_font_set_title(vision_ui_t* ui, vision_ui_font_t font);

/**
 * Sets the subtitle font.
 *
 * @param ui UI instance to configure. Must not be `NULL`.
 * @param font Font configuration to store.
 */
extern void vision_ui_font_set_subtitle(vision_ui_t* ui, vision_ui_font_t font);

/**
 * Returns the current small utility font.
 *
 * @param ui UI instance to query. Must not be `NULL`.
 * @return A copy of the currently configured minifont. Its `font` field may be `NULL` if no font was configured yet.
 */
extern vision_ui_font_t vision_ui_minifont_get(const vision_ui_t* ui);

/**
 * Returns the current main body font.
 *
 * @param ui UI instance to query. Must not be `NULL`.
 * @return A copy of the currently configured body font. Its `font` field may be `NULL` if no font was configured yet.
 */
extern vision_ui_font_t vision_ui_font_get(const vision_ui_t* ui);

/**
 * Returns the current title font.
 *
 * @param ui UI instance to query. Must not be `NULL`.
 * @return A copy of the currently configured title font. Its `font` field may be `NULL` if no font was configured yet.
 */
extern vision_ui_font_t vision_ui_font_get_title(const vision_ui_t* ui);

/**
 * Returns the current subtitle font.
 *
 * @param ui UI instance to query. Must not be `NULL`.
 * @return A copy of the currently configured subtitle font. Its `font` field may be `NULL` if no font was configured
 * yet.
 */
extern vision_ui_font_t vision_ui_font_get_subtitle(const vision_ui_t* ui);

extern bool vision_ui_exit_animation_is_finished(const vision_ui_t* ui);

extern void vision_ui_exit_animation_set_is_finished(vision_ui_t* ui);

extern void vision_ui_exit_animation_start(vision_ui_t* ui);

extern bool vision_ui_enter_animation_is_finished(const vision_ui_t* ui);

extern void vision_ui_enter_animation_set_is_finished(vision_ui_t* ui);

extern void vision_ui_enter_animation_start(vision_ui_t* ui);

#endif // VISION_UI_VISION_UI_CORE_H
