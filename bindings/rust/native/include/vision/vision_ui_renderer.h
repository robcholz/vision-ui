//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//

#ifndef VISION_UI_VISION_UI_RENDERER_H
#define VISION_UI_VISION_UI_RENDERER_H

#include "vision_ui_types.h"

typedef enum vision_ui_notification_push_result_t {
    /// The notification was accepted successfully.
    VisionUiNotificationPushOk = 0,
    /// The notification content pointer was invalid.
    VisionUiNotificationPushContentInvalid,
} vision_ui_notification_push_result_t;

extern void vision_ui_exit_animation_render(vision_ui_t* ui, float delta_ms);

extern void vision_ui_enter_animation_render(vision_ui_t* ui, float delta_ms);

extern void vision_ui_widget_render(vision_ui_t* ui);

extern void vision_ui_list_render(vision_ui_t* ui);

extern const vision_ui_notification_t* vision_ui_notification_instance_get(const vision_ui_t* ui);

extern vision_ui_notification_t* vision_ui_notification_mutable_instance_get(vision_ui_t* ui);

/**
 * Shows a transient notification bar for the given duration in milliseconds.
 *
 * If another notification is already running, the new message is queued as the pending replacement.
 *
 * @param ui UI instance that owns the notification state. Must not be `NULL`.
 * @param content Borrowed NUL-terminated message string used for layout and rendering. The pointed-to string should
 * remain valid while the notification is visible or pending.
 * @param span Display duration in milliseconds.
 * @return `VisionUiNotificationPushOk` on success.
 * @return `VisionUiNotificationPushContentInvalid` when `content` is `NULL`.
 */
extern vision_ui_notification_push_result_t vision_ui_notification_push(
        vision_ui_t* ui,
        const char* content,
        uint16_t span
);

extern const vision_ui_alert_t* vision_ui_alert_instance_get(const vision_ui_t* ui);

extern vision_ui_alert_t* vision_ui_alert_mutable_instance_get(vision_ui_t* ui);

/**
 * Shows a centered alert for the given duration in milliseconds.
 *
 * @param ui UI instance that owns the alert state. Must not be `NULL`.
 * @param content Borrowed NUL-terminated message string used for layout and rendering. The pointed-to string should
 * remain valid while the alert is visible.
 * @param span Display duration in milliseconds.
 */
extern void vision_ui_alert_push(vision_ui_t* ui, const char* content, uint16_t span);

/// Built-in list icon pack used by the simulator and default renderer setup.
extern vision_ui_icon_t DEFAULT_LIST_ICON;

/**
 * Replaces the active list icon pack.
 *
 * @param ui UI instance to configure. Must not be `NULL`.
 * @param icon Icon bitmap pack to store by value.
 *
 * @note The struct is copied by value, but any bitmap pointers inside it remain borrowed and should stay valid while
 * the icon pack is active.
 */
extern void vision_ui_list_icon_set(vision_ui_t* ui, vision_ui_icon_t icon);

extern vision_ui_icon_t vision_ui_list_icon_get_current(const vision_ui_t* ui);

#endif // VISION_UI_VISION_UI_RENDERER_H
