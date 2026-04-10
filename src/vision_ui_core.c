//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//
#include "vision_ui_core.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <tgmath.h>

#include "vision_ui_animation.h"
#include "vision_ui_config.h"
#include "vision_ui_draw_driver.h"
#include "vision_ui_item.h"
#include "vision_ui_renderer.h"

void vision_ui_init(vision_ui_t* ui) {
    assert(ui != NULL);

    memset(ui, 0, sizeof(*ui));
    ui->exit_animation_finished = true;
    ui->enter_animation_finished = true;
    ui->notification = (vision_ui_notification_t) {0,
                                                   1,
                                                   0 - 2 * VISION_UI_NOTIFICATION_HEIGHT,
                                                   0 - 2 * VISION_UI_NOTIFICATION_HEIGHT,
                                                   VISION_UI_NOTIFICATION_WIDTH,
                                                   VISION_UI_NOTIFICATION_WIDTH,
                                                   false,
                                                   0,
                                                   1,
                                                   false,
                                                   0,
                                                   NULL,
                                                   0,
                                                   false};
    ui->alert = (vision_ui_alert_t) {0,
                                     1,
                                     0 - 2 * VISION_UI_ALERT_HEIGHT,
                                     0 - 2 * VISION_UI_ALERT_HEIGHT,
                                     VISION_UI_ALERT_WIDTH,
                                     VISION_UI_ALERT_WIDTH,
                                     false,
                                     0,
                                     1};
    ui->camera = (vision_ui_camera_t) {0, 0, 0, 0, 0, 0, NULL};
    ui->list_icon = DEFAULT_LIST_ICON;
}

vision_ui_t* vision_ui_create() {
    vision_ui_t* ui = malloc(sizeof(vision_ui_t));
    if (ui == NULL) {
        return NULL;
    }
    vision_ui_init(ui);
    return ui;
}

void vision_ui_destroy(vision_ui_t* ui) {
    free(ui);
}

void vision_ui_allocator_set(vision_ui_t* ui, const vision_ui_allocator_t allocator) {
    assert(ui != NULL);
    ui->allocator = allocator;
}

void vision_ui_minifont_set(vision_ui_t* ui, const vision_ui_font_t font) {
    assert(ui != NULL);
    if (memcmp(&font, &ui->minifont, sizeof(vision_ui_font_t)) != 0) {
        ui->minifont = font;
    }
}

void vision_ui_font_set(vision_ui_t* ui, const vision_ui_font_t font) {
    assert(ui != NULL);
    if (memcmp(&font, &ui->font, sizeof(vision_ui_font_t)) != 0) {
        ui->font = font;
    }
}

void vision_ui_font_set_title(vision_ui_t* ui, const vision_ui_font_t font) {
    assert(ui != NULL);
    if (memcmp(&font, &ui->title_font, sizeof(vision_ui_font_t)) != 0) {
        ui->title_font = font;
    }
}

void vision_ui_font_set_subtitle(vision_ui_t* ui, const vision_ui_font_t font) {
    assert(ui != NULL);
    if (memcmp(&font, &ui->subtitle_font, sizeof(vision_ui_font_t)) != 0) {
        ui->subtitle_font = font;
    }
}

vision_ui_font_t vision_ui_minifont_get(const vision_ui_t* ui) {
    assert(ui != NULL);
    return ui->minifont;
}

vision_ui_font_t vision_ui_font_get(const vision_ui_t* ui) {
    assert(ui != NULL);
    return ui->font;
}

vision_ui_font_t vision_ui_font_get_title(const vision_ui_t* ui) {
    assert(ui != NULL);
    return ui->title_font;
}

vision_ui_font_t vision_ui_font_get_subtitle(const vision_ui_t* ui) {
    assert(ui != NULL);
    return ui->subtitle_font;
}

bool vision_ui_exit_animation_is_finished(const vision_ui_t* ui) {
    assert(ui != NULL);
    return ui->exit_animation_finished;
}

void vision_ui_exit_animation_set_is_finished(vision_ui_t* ui) {
    assert(ui != NULL);
    ui->exit_animation_finished = true;
}

void vision_ui_exit_animation_start(vision_ui_t* ui) {
    assert(ui != NULL);
    ui->exit_animation_finished = false;
}

bool vision_ui_enter_animation_is_finished(const vision_ui_t* ui) {
    assert(ui != NULL);
    return ui->enter_animation_finished;
}

void vision_ui_enter_animation_set_is_finished(vision_ui_t* ui) {
    assert(ui != NULL);
    ui->enter_animation_finished = true;
}

void vision_ui_enter_animation_start(vision_ui_t* ui) {
    assert(ui != NULL);
    ui->enter_animation_finished = false;
}

void vision_ui_render_init(vision_ui_t* ui) {
    assert(ui != NULL);
    ui->is_in_vision_ui = true;
    vision_ui_driver_font_set(ui, vision_ui_font_get(ui));
}

static void vision_ui_notification_update(vision_ui_t* ui, const float delta_ms) {
    vision_ui_notification_t* notification = vision_ui_notification_mutable_instance_get(ui);
    vision_ui_animation_s_curve(&notification->y_notification, notification->y_notification_trg, 50, delta_ms);
    vision_ui_animation_s_curve(&notification->w_notification, notification->w_notification_trg, 52, delta_ms);
}

static void vision_ui_alert_update(vision_ui_t* ui, const float delta_ms) {
    vision_ui_alert_t* alert = vision_ui_alert_mutable_instance_get(ui);
    vision_ui_animation_s_curve(&alert->y_alert, alert->y_alert_trg, 94, delta_ms);
    vision_ui_animation_s_curve(&alert->w_alert, alert->w_alert_trg, 96, delta_ms);
}

static void vision_ui_camera_position_update(vision_ui_t* ui, const float delta_ms) {
    vision_ui_camera_t* camera = vision_ui_camera_mutable_instance_get(ui);
    const vision_ui_selector_t* selector = camera->selector;
    if (selector == NULL || selector->selected_item == NULL) {
        vision_ui_camera_instance_x_trg_set(ui, 0);
        vision_ui_camera_instance_y_trg_set(ui, 0);
        vision_ui_animation_2nd_ode_slight_overshoot(
                &camera->x_camera, &camera->x_camera_velocity, camera->x_camera_trg, 95, delta_ms
        );
        vision_ui_animation_2nd_ode_slight_overshoot(
                &camera->y_camera, &camera->y_camera_velocity, camera->y_camera_trg, 96, delta_ms
        );
        return;
    }

    const vision_ui_list_item_t* selected_item = selector->selected_item;
    bool icon_view_active = false;
    if (selected_item->parent != NULL && selected_item->parent->icon_view_mode) {
        icon_view_active = true;
    } else if (selected_item->parent != NULL && selected_item->parent->type == IconItem &&
               selected_item->parent->child_num == 1 && selected_item->parent->parent != NULL &&
               selected_item->parent->parent->icon_view_mode &&
               selected_item->parent->child_list_item[0]->type == UserItem) {
        icon_view_active = true;
    }
    if (icon_view_active) {
        vision_ui_camera_instance_x_trg_set(ui, 0);
        vision_ui_camera_instance_y_trg_set(ui, 0);
        vision_ui_animation_2nd_ode_slight_overshoot(
                &camera->x_camera, &camera->x_camera_velocity, camera->x_camera_trg, 95, delta_ms
        );
        vision_ui_animation_2nd_ode_slight_overshoot(
                &camera->y_camera, &camera->y_camera_velocity, camera->y_camera_trg, 96, delta_ms
        );
        return;
    }

    if (selector->y_selector_trg + VISION_UI_LIST_SELECTOR_FIXED_HEIGHT + camera->y_camera_trg >
        VISION_UI_SCREEN_HEIGHT) {
        // Move down when the selector would extend past the bottom of the screen.
        vision_ui_camera_instance_y_trg_set(
                ui, VISION_UI_SCREEN_HEIGHT - selector->y_selector_trg - VISION_UI_LIST_SELECTOR_FIXED_HEIGHT
        );
    }

    const float top_padding = VISION_UI_LIST_TITLE_TO_DISPLAY_TOP_PADDING;
    if (selector->y_selector_trg + camera->y_camera_trg < top_padding) {
        // Move up when the selector would extend past the top padding.
        vision_ui_camera_instance_y_trg_set(ui, top_padding - selector->y_selector_trg);
    }

    vision_ui_camera_instance_x_trg_set(ui, 0);
    vision_ui_animation_2nd_ode_slight_overshoot(
            &camera->x_camera, &camera->x_camera_velocity, camera->x_camera_trg, 95, delta_ms
    );
    vision_ui_animation_2nd_ode_slight_overshoot(
            &camera->y_camera, &camera->y_camera_velocity, camera->y_camera_trg, 96, delta_ms
    );
}

static void vision_ui_widget_core_position_update(vision_ui_t* ui, const float delta_ms) {
    // Run updates for all widgets.
    vision_ui_notification_update(ui, delta_ms);
    vision_ui_alert_update(ui, delta_ms);
}

static void vision_ui_list_init(vision_ui_t* ui) {
    const vision_ui_list_item_t* root = vision_ui_root_list_get(ui);
    vision_ui_selector_t* selector = vision_ui_selector_mutable_instance_get(ui);

    // Initialize animation-related state.
    for (uint8_t i = 0; i < root->child_num; i++) {
        vision_ui_list_item_t* list = root->child_list_item[i];
        list->y_list_item = 0;
        list->scroll_bar_top = 0;
        list->scroll_bar_top_trg = 0;
        list->scroll_bar_top_velocity = 0;
        list->scroll_bar_height = 0;
        list->scroll_bar_height_velocity = 0;
        list->scroll_bar_height_trg = 0;
        list->scroll_bar_scale_part = 0;
        list->scroll_bar_scale_part_trg = 0;
        list->scroll_bar_top_px = 0;
        list->scroll_bar_height_px = VISION_UI_SCREEN_HEIGHT;
    }
    selector->scroll_bar_scale_parent = NULL;
    selector->scroll_bar_scale_part_shared = 0.f;
    selector->selected_index = 0;
    selector->selected_item = root->child_num > 0 ? root->child_list_item[0] : NULL;

    selector->w_selector_velocity = 0;

    selector->y_selector = VISION_UI_SCREEN_HEIGHT;
    selector->y_selector_velocity = 0;

    selector->h_selector = VISION_UI_SCREEN_HEIGHT;
    selector->h_selector_velocity = 0;
}

void vision_ui_core_init(vision_ui_t* ui) {
    assert(ui != NULL);

    vision_ui_list_init(ui);
    const vision_ui_list_item_t* root = vision_ui_root_list_get(ui);
    if (root->child_num > 0) {
        vision_ui_selector_t_selector_bind_item(ui, root->child_list_item[0]);
    }
    vision_ui_camera_bind_selector(ui, vision_ui_selector_mutable_instance_get(ui));
}

void vision_ui_start_logo_set(vision_ui_t* ui, const uint8_t* bmp, const uint32_t span) {
    assert(ui != NULL);

    ui->logo = bmp;
    ui->logo_span = span;
    ui->logo_finished = false;
    ui->logo_started = false;
    ui->logo_start_time = 0;
}

static void vision_ui_list_item_position_update(vision_ui_t* ui, const float delta_ms) {
    vision_ui_selector_t* selector_mut = vision_ui_selector_mutable_instance_get(ui);
    const vision_ui_selector_t* selector = vision_ui_selector_instance_get(ui);

    if (selector->selected_item == NULL) {
        return;
    }

    vision_ui_list_item_t* parent = selector->selected_item->parent;
    const vision_ui_list_item_t* selected_icon_item = selector->selected_item;
    if (parent != NULL && parent->type == IconItem && parent->child_num == 1 && parent->parent != NULL &&
        parent->parent->icon_view_mode && parent->child_list_item[0]->type == UserItem) {
        selected_icon_item = parent;
        parent = parent->parent;
    }
    if (parent == NULL) {
        return;
    }

    if (parent->icon_view_mode) {
        for (uint8_t i = 0; i < parent->child_num; i++) {
            vision_ui_icon_item_t* item = vision_ui_to_list_icon_item(parent->child_list_item[i]);
            item->base_item.y_list_item = 0;
            item->base_item.y_list_item_trg = 0;

            item->title_y_trg = VISION_UI_ICON_VIEW_TITLE_AREA_HEIGHT;

            const bool is_selected = parent->child_list_item[i] == selected_icon_item;
            item->title_y_trg = is_selected ? 0.f : VISION_UI_ICON_VIEW_TITLE_AREA_HEIGHT;
            vision_ui_animation_2nd_ode_slight_overshoot(
                    &item->title_y,
                    &item->title_y_velocity,
                    item->title_y_trg,
                    VISION_UI_ICON_VIEW_SCROLL_SPEED,
                    delta_ms
            );
        }

        const float icon_item_span = (float) (VISION_UI_ICON_VIEW_ICON_SIZE + VISION_UI_ICON_VIEW_ITEM_SPACING);
        parent->icon_scroll_offset_trg = -icon_item_span * selector->selected_index;
        vision_ui_animation_2nd_ode_slight_overshoot(
                &parent->icon_scroll_offset,
                &parent->icon_scroll_offset_velocity,
                parent->icon_scroll_offset_trg,
                VISION_UI_ICON_VIEW_SCROLL_SPEED,
                delta_ms
        );

        parent->scroll_bar_top = 0;
        parent->scroll_bar_top_velocity = 0;
        parent->scroll_bar_top_trg = 0;
        parent->scroll_bar_height = 0;
        parent->scroll_bar_height_velocity = 0;
        parent->scroll_bar_height_trg = 0;
        parent->scroll_bar_scale_part = 0;
        parent->scroll_bar_scale_part_trg = 0;
        parent->scroll_bar_top_px = 0;
        parent->scroll_bar_height_px = 0;
        selector_mut->scroll_bar_scale_parent = NULL;
        selector_mut->scroll_bar_scale_part_shared = 0.f;
        return;
    }

    for (uint8_t i = 0; i < parent->child_num; i++) {
#if VISION_UI_LIST_ENTRY_ANIMATION
        vision_ui_animation_s_curve(
                &parent->child_list_item[i]->y_list_item, parent->child_list_item[i]->y_list_item_trg, 84, delta_ms
        );
#else
        parent->child_list_item[i]->y_list_item = parent->child_list_item[i]->y_list_item_trg;
#endif
    }

    const uint8_t child_cnt = parent->child_num > 0 ? parent->child_num : 1;
    const float part = (float) VISION_UI_SCREEN_HEIGHT / child_cnt;
    const float slider_top_trg = part * selector->selected_index;
    const float slider_h_trg = fmaxf((float) VISION_UI_LIST_FRAME_FIXED_HEIGHT / child_cnt, part);

    parent->scroll_bar_top_trg = slider_top_trg;
    parent->scroll_bar_height_trg = slider_h_trg;
    parent->scroll_bar_scale_part_trg = part;

    const vision_ui_list_item_t* const prev_parent = selector_mut->scroll_bar_scale_parent;
    if (parent != prev_parent) {
        if (prev_parent != NULL) {
            parent->scroll_bar_top = prev_parent->scroll_bar_top;
            parent->scroll_bar_top_velocity = prev_parent->scroll_bar_top_velocity;
            parent->scroll_bar_height = prev_parent->scroll_bar_height;
            parent->scroll_bar_height_velocity = prev_parent->scroll_bar_height_velocity;
        }
        parent->scroll_bar_scale_part = selector_mut->scroll_bar_scale_part_shared;
        selector_mut->scroll_bar_scale_parent = parent;
    }

    const bool scroll_bar_uninitialized = parent->scroll_bar_height == 0.f && parent->scroll_bar_height_trg == 0.f;
    if (scroll_bar_uninitialized) {
        parent->scroll_bar_top = parent->scroll_bar_top_trg;
        parent->scroll_bar_height_velocity = 0;
        parent->scroll_bar_height = parent->scroll_bar_height_trg;
    } else {
        vision_ui_animation_2nd_ode_no_overshoot(
                &parent->scroll_bar_top,
                &parent->scroll_bar_top_velocity,
                parent->scroll_bar_top_trg,
                VISION_UI_LIST_SCROLL_BAR_ANIMATION_SPEED,
                delta_ms
        );
        vision_ui_animation_s_curve(&parent->scroll_bar_height, parent->scroll_bar_height_trg, 92, delta_ms);
    }

    const bool scroll_bar_scale_uninitialized =
            parent->scroll_bar_scale_part == 0.f && parent->scroll_bar_scale_part_trg == 0.f;
    if (scroll_bar_scale_uninitialized) {
        parent->scroll_bar_scale_part = parent->scroll_bar_scale_part_trg;
    } else {
        vision_ui_animation_s_curve(&parent->scroll_bar_scale_part, parent->scroll_bar_scale_part_trg, 94, delta_ms);
    }

    selector_mut->scroll_bar_scale_part_shared = parent->scroll_bar_scale_part;

    int16_t slider_top_px = (int16_t) lrintf(parent->scroll_bar_top);
    int16_t slider_h_px = (int16_t) lrintf(parent->scroll_bar_height);
    if (slider_h_px < 1) {
        slider_h_px = 1;
    }
    if (slider_h_px > VISION_UI_SCREEN_HEIGHT) {
        slider_h_px = VISION_UI_SCREEN_HEIGHT;
    }
    if (slider_top_px < 0) {
        slider_top_px = 0;
    }
    if (slider_top_px > VISION_UI_SCREEN_HEIGHT - 1) {
        slider_top_px = VISION_UI_SCREEN_HEIGHT - 1;
    }
    if (slider_top_px + slider_h_px > VISION_UI_SCREEN_HEIGHT) {
        slider_top_px = VISION_UI_SCREEN_HEIGHT - slider_h_px;
    }

    parent->scroll_bar_top_px = slider_top_px;
    parent->scroll_bar_height_px = slider_h_px;
}

static void vision_ui_selector_position_update(vision_ui_t* ui, const float delta_ms) {
    vision_ui_selector_t* selector_mut = vision_ui_selector_mutable_instance_get(ui);
    const vision_ui_selector_t* selector = vision_ui_selector_instance_get(ui);
    if (selector == NULL || selector->selected_item == NULL) {
        return;
    }

    vision_ui_font_set(ui, vision_ui_font_get(ui));
    selector_mut->h_selector_trg = VISION_UI_LIST_FRAME_FIXED_HEIGHT;
    selector_mut->y_selector_trg = selector->selected_item->y_list_item_trg;
    const uint16_t selector_max_width = VISION_UI_LIST_HEADER_MAX_WIDTH + VISION_UI_LIST_HEADER_TO_TEXT_PADDING +
                                        VISION_UI_LIST_TEXT_MAX_WIDTH(selector->selected_item) +
                                        VISION_UI_LIST_SELECTOR_TO_INNER_WIDGET_PADDING +
                                        VISION_UI_LIST_SELECTOR_TO_INNER_WIDGET_PADDING;
    const uint16_t selector_current_width =
            selector->selected_item->type == TitleItem
                    ? +vision_ui_driver_str_utf8_width_get(ui, selector->selected_item->content) +
                              VISION_UI_LIST_SELECTOR_TO_INNER_WIDGET_PADDING +
                              VISION_UI_LIST_SELECTOR_TO_INNER_WIDGET_PADDING
                    : +VISION_UI_LIST_HEADER_MAX_WIDTH + VISION_UI_LIST_HEADER_TO_TEXT_PADDING +
                              vision_ui_driver_str_utf8_width_get(ui, selector->selected_item->content) +
                              VISION_UI_LIST_SELECTOR_TO_INNER_WIDGET_PADDING +
                              VISION_UI_LIST_SELECTOR_TO_INNER_WIDGET_PADDING;
    selector_mut->w_selector_trg =
            selector_current_width > selector_max_width ? selector_max_width : selector_current_width;
    vision_ui_animation_2nd_ode_slight_overshoot(
            &selector_mut->y_selector, &selector_mut->y_selector_velocity, selector->y_selector_trg, 91, delta_ms
    );
    vision_ui_animation_2nd_ode_slight_overshoot(
            &selector_mut->w_selector, &selector_mut->w_selector_velocity, selector->w_selector_trg, 92, delta_ms
    );
    vision_ui_animation_2nd_ode_slight_overshoot(
            &selector_mut->h_selector, &selector_mut->h_selector_velocity, selector->h_selector_trg, 95, delta_ms
    );
}

static void vision_ui_main_core_position_update(vision_ui_t* ui, const float delta_ms) {
    vision_ui_list_item_position_update(ui, delta_ms);
}

static void vision_ui_widget_core_step(vision_ui_t* ui, const float delta_ms) {
    vision_ui_widget_core_position_update(ui, delta_ms);
    vision_ui_widget_render(ui);
}

static void vision_ui_main_core_step(vision_ui_t* ui, const float delta_ms) {
    assert(ui != NULL);
    if (vision_ui_is_exited(ui)) {
        return;
    }

    const bool exit_animation_finished = vision_ui_exit_animation_is_finished(ui);

    vision_ui_selector_t* selector_mut = vision_ui_selector_mutable_instance_get(ui);
    if (selector_mut == NULL || selector_mut->selected_item == NULL) {
        return;
    }
    if (exit_animation_finished && selector_mut->has_pending_selection) {
        selector_mut->selected_item = selector_mut->pending_selected_item;
        selector_mut->selected_index = selector_mut->pending_selected_index;
        selector_mut->has_pending_selection = false;
    }

    // Handle enter/exit transitions for user items.
    if (exit_animation_finished) {
        if (selector_mut->selected_item->type == UserItem) {
            vision_ui_user_item_t* selected_user_item = vision_ui_to_list_user_item(selector_mut->selected_item);
            if (selected_user_item->entering_user_item) {
                selected_user_item->in_user_item = 1;
                selected_user_item->entering_user_item = false;
            } else if (selected_user_item->exiting_user_item) {
                if (selected_user_item->user_item_inited && selected_user_item->user_item_looping) {
                    if (selected_user_item->exit_function != NULL) {
                        selected_user_item->exit_function(ui);
                    }
                }
                selected_user_item->in_user_item = 0;

                // when the user item was entered directly from an icon item with a single child,
                // exit back to that icon instead of showing the intermediate list
                vision_ui_list_item_t* icon_item = selector_mut->selected_item->parent;
                const bool is_single_user_child = icon_item != NULL && icon_item->type == IconItem &&
                                                  icon_item->child_num == 1 &&
                                                  icon_item->child_list_item[0]->type == UserItem;
                if (is_single_user_child && icon_item != NULL && icon_item->parent != NULL &&
                    icon_item->parent->icon_view_mode) {
                    const vision_ui_list_item_t* icon_list = icon_item->parent;
                    uint8_t icon_index = 0;
                    for (uint8_t i = 0; i < icon_list->child_num; ++i) {
                        if (icon_list->child_list_item[i] == icon_item) {
                            icon_index = i;
                            break;
                        }
                    }
                    selector_mut->selected_item = icon_item;
                    selector_mut->selected_index = icon_index;
                }
                selected_user_item->exiting_user_item = false;
            }
        }
    }

    // Main render flow.
    const vision_ui_selector_t* selector = vision_ui_selector_instance_get(ui);
    if (selector->selected_item->type == UserItem &&
        vision_ui_to_list_user_item(selector->selected_item)->in_user_item) {
        vision_ui_user_item_t* selected_user_item = vision_ui_to_list_user_item(selector->selected_item);
        // Initialize on first entry.
        if (!selected_user_item->user_item_inited) {
            if (selected_user_item->init_function != NULL) {
                selected_user_item->init_function(ui);
            }
            selected_user_item->user_item_inited = true;
        }

        if (selected_user_item->loop_function != NULL) {
            selected_user_item->user_item_looping = true;
            selected_user_item->loop_function(ui);
        }
    } else {
        vision_ui_main_core_position_update(ui, delta_ms);
        vision_ui_selector_position_update(ui, delta_ms);
        vision_ui_camera_position_update(ui, delta_ms);
        vision_ui_list_render(ui);
    }

    ui->is_background_frozen = vision_ui_alert_instance_get(ui)->is_running;

    // Exit animation.
    // Everything above is the normal frame; the exit pass only needs to add the overlay.
    if (!vision_ui_exit_animation_is_finished(ui)) {
        vision_ui_exit_animation_render(ui, delta_ms);
        return;
    }

    if (!vision_ui_enter_animation_is_finished(ui)) {
        vision_ui_enter_animation_render(ui, delta_ms);
        return;
    }
}

void vision_ui_step_render(vision_ui_t* ui) {
    assert(ui != NULL);

    const uint32_t now = vision_ui_driver_ticks_ms_get(ui);

    if (!ui->logo_finished) {
        if (ui->logo_started == false) {
            ui->logo_started = true;
            ui->logo_start_time = now;
        }

        if (ui->logo != NULL && now - ui->logo_start_time < ui->logo_span) {
            vision_ui_driver_bmp_draw(ui, 0, 0, VISION_UI_SCREEN_WIDTH, VISION_UI_SCREEN_HEIGHT, ui->logo);
            return;
        }

        if (ui->logo == NULL || now - ui->logo_start_time > ui->logo_span) {
            ui->logo_finished = true;
            vision_ui_driver_buffer_clear(ui);
        }
    }

    float delta_ms = 16.6667f;
    if (ui->last_tick != 0 && now >= ui->last_tick) {
        delta_ms = (float) (now - ui->last_tick);
        if (delta_ms < 1.0f) {
            delta_ms = 1.0f;
        }
    }
    ui->last_tick = now;

    switch (vision_ui_driver_action_get(ui)) {
        case UiActionGoPrev:
            if (!vision_ui_is_background_frozen(ui)) {
                vision_ui_selector_go_prev_item(ui);
            }
            break;
        case UiActionGoNext:
            if (!vision_ui_is_background_frozen(ui)) {
                vision_ui_selector_go_next_item(ui);
            }
            break;
        case UiActionExit:
            vision_ui_selector_exit_current_item(ui);
            break;
        case UiActionEnter:
            if (!vision_ui_is_background_frozen(ui)) {
                vision_ui_selector_jump_to_selected_item(ui);
            }
            break;
        default:
            break;
    }
    vision_ui_main_core_step(ui, delta_ms); // Main UI update/render path.
    vision_ui_widget_core_step(ui, delta_ms); // Notification/alert update path.
}

bool vision_ui_is_exited(const vision_ui_t* ui) {
    return ui == NULL || !ui->is_in_vision_ui;
}

bool vision_ui_is_background_frozen(const vision_ui_t* ui) {
    assert(ui != NULL);
    return ui->is_background_frozen;
}
