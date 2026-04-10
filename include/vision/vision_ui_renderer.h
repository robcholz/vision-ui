//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//

#ifndef VISION_UI_VISION_UI_RENDERER_H
#define VISION_UI_VISION_UI_RENDERER_H

#include "vision_ui_types.h"

extern void vision_ui_exit_animation_render(vision_ui_t* ui, float delta_ms);

extern void vision_ui_enter_animation_render(vision_ui_t* ui, float delta_ms);

extern void vision_ui_widget_render(vision_ui_t* ui);

extern void vision_ui_list_render(vision_ui_t* ui);

extern const vision_ui_notification_t* vision_ui_notification_instance_get(const vision_ui_t* ui);

extern vision_ui_notification_t* vision_ui_notification_mutable_instance_get(vision_ui_t* ui);

/// Shows a transient notification bar for the given duration in milliseconds.
extern void vision_ui_notification_push(vision_ui_t* ui, const char* content, uint16_t span);

extern const vision_ui_alert_t* vision_ui_alert_instance_get(const vision_ui_t* ui);

extern vision_ui_alert_t* vision_ui_alert_mutable_instance_get(vision_ui_t* ui);

/// Shows a centered alert for the given duration in milliseconds.
extern void vision_ui_alert_push(vision_ui_t* ui, const char* content, uint16_t span);

/// Built-in list icon pack used by the simulator and default renderer setup.
extern vision_ui_icon_t DEFAULT_LIST_ICON;

/// Replaces the active list icon pack.
extern void vision_ui_list_icon_set(vision_ui_t* ui, vision_ui_icon_t icon);

extern vision_ui_icon_t vision_ui_list_icon_get_current(const vision_ui_t* ui);

#endif // VISION_UI_VISION_UI_RENDERER_H
