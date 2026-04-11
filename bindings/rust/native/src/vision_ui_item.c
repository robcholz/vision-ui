//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//
#include "vision_ui_item.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "vision_ui_config.h"
#include "vision_ui_core.h"

static void* vision_ui_malloc(const vision_ui_t* ui, const size_t size) {
    assert(ui != NULL);
    if (ui->allocator == NULL) {
        return malloc(size);
    }
    return ui->allocator(VisionAllocMalloc, size, 0, NULL);
}

static void vision_ui_free(const vision_ui_t* ui, void* ptr) {
    assert(ui != NULL);
    if (ptr == NULL) {
        return;
    }
    if (ui->allocator == NULL) {
        free(ptr);
        return;
    }
    ui->allocator(VisionAllocFree, 0, 0, ptr);
}

static void vision_ui_owned_item_register(const vision_ui_t* ui, vision_ui_list_item_t* item) {
    assert(ui != NULL);
    assert(item != NULL);

    vision_ui_t* ui_mut = (vision_ui_t*) ui;
    item->owned_next = ui_mut->owned_item_head;
    ui_mut->owned_item_head = item;
}

static bool vision_ui_child_list_init(const vision_ui_t* ui, vision_ui_list_item_t* item, const size_t capacity) {
    assert(ui != NULL);
    assert(item != NULL);

    item->capacity = capacity;
    if (capacity == 0) {
        item->owns_child_list = false;
        item->child_list_item = NULL;
        return true;
    }

    item->owns_child_list = true;
    item->child_list_item = vision_ui_malloc(ui, capacity * sizeof(vision_ui_list_item_t*));
    return item->child_list_item != NULL;
}

// vision_ui_list_item_t vision_ui_list_item_root = {};

vision_ui_switch_item_t* vision_ui_to_list_switch_item(vision_ui_list_item_t* list_item) {
    assert(list_item != NULL && list_item->type == SwitchItem);
    return (vision_ui_switch_item_t*) list_item;
}

vision_ui_slider_item_t* vision_ui_to_list_slider_item(vision_ui_list_item_t* list_item) {
    assert(list_item != NULL && list_item->type == SliderItem);
    return (vision_ui_slider_item_t*) list_item;
}

vision_ui_icon_item_t* vision_ui_to_list_icon_item(vision_ui_list_item_t* list_item) {
    assert(list_item != NULL && list_item->type == IconItem);
    return (vision_ui_icon_item_t*) list_item;
}

vision_ui_user_item_t* vision_ui_to_list_user_item(vision_ui_list_item_t* list_item) {
    assert(list_item != NULL && list_item->type == UserItem);
    return (vision_ui_user_item_t*) list_item;
}

vision_ui_list_item_t* vision_ui_list_item_new(
        const vision_ui_t* ui,
        const size_t capacity,
        const bool icon_mode,
        const char* content
) {
    vision_ui_list_item_t* list_item = vision_ui_malloc(ui, sizeof(vision_ui_list_item_t));
    if (list_item == NULL) {
        return NULL;
    }
    memset(list_item, 0, sizeof(vision_ui_list_item_t));
    list_item->type = ListItem;
    list_item->icon_view_mode = icon_mode;
    list_item->content = content;
    if (!vision_ui_child_list_init(ui, list_item, capacity)) {
        vision_ui_free(ui, list_item);
        return NULL;
    }
    vision_ui_owned_item_register(ui, list_item);
    return list_item;
}

vision_ui_list_item_t* vision_ui_list_title_item_new(const vision_ui_t* ui, const char* title) {
    vision_ui_list_item_t* list_item = vision_ui_malloc(ui, sizeof(vision_ui_title_item_t));
    if (list_item == NULL) {
        return NULL;
    }
    memset(list_item, 0, sizeof(vision_ui_title_item_t));
    list_item->type = TitleItem;
    list_item->content = title;
    vision_ui_child_list_init(ui, list_item, 0);
    vision_ui_owned_item_register(ui, list_item);
    return list_item;
}

vision_ui_list_item_t* vision_ui_list_icon_item_new(
        const vision_ui_t* ui,
        const size_t capacity,
        const uint8_t* icon,
        const char* title,
        const char* description
) {
    vision_ui_list_item_t* list_item = vision_ui_malloc(ui, sizeof(vision_ui_icon_item_t));
    if (list_item == NULL) {
        return NULL;
    }
    memset(list_item, 0, sizeof(vision_ui_icon_item_t));
    list_item->type = IconItem;
    list_item->content = title;
    if (!vision_ui_child_list_init(ui, list_item, capacity)) {
        vision_ui_free(ui, list_item);
        return NULL;
    }

    ((vision_ui_icon_item_t*) list_item)->icon = icon;
    ((vision_ui_icon_item_t*) list_item)->description = description;
    // start hidden so the first selection animates the slide-in
    ((vision_ui_icon_item_t*) list_item)->title_y = VISION_UI_ICON_VIEW_TITLE_AREA_HEIGHT;
    ((vision_ui_icon_item_t*) list_item)->title_y_velocity = 0;
    ((vision_ui_icon_item_t*) list_item)->title_y_trg = VISION_UI_ICON_VIEW_TITLE_AREA_HEIGHT;
    vision_ui_owned_item_register(ui, list_item);
    return list_item;
}

vision_ui_list_item_t* vision_ui_list_switch_item_new(
        const vision_ui_t* ui,
        const char* content,
        const bool default_value,
        vision_ui_switch_changed_cb on_changed,
        void* user_data
) {
    vision_ui_switch_item_t* switch_item = vision_ui_malloc(ui, sizeof(vision_ui_switch_item_t));
    if (switch_item == NULL) {
        return NULL;
    }
    memset(switch_item, 0, sizeof(vision_ui_switch_item_t));
    switch_item->base_item.type = SwitchItem;
    switch_item->base_item.content = content;
    switch_item->value = default_value;
    switch_item->on_changed = on_changed;
    switch_item->on_changed_user_data = user_data;
    vision_ui_child_list_init(ui, (vision_ui_list_item_t*) switch_item, 0);
    vision_ui_owned_item_register(ui, (vision_ui_list_item_t*) switch_item);
    return (vision_ui_list_item_t*) switch_item;
}

vision_ui_list_item_t* vision_ui_list_slider_item_new(
        const vision_ui_t* ui,
        const char* content,
        const int16_t default_value,
        const uint8_t step,
        const int16_t min,
        const int16_t max,
        vision_ui_slider_changed_cb on_changed,
        void* user_data
) {
    vision_ui_slider_item_t* slider_item = vision_ui_malloc(ui, sizeof(vision_ui_slider_item_t));
    if (slider_item == NULL) {
        return NULL;
    }
    memset(slider_item, 0, sizeof(vision_ui_slider_item_t));
    slider_item->base_item.type = SliderItem;
    slider_item->base_item.content = content;
    slider_item->value = default_value;
    slider_item->value_step = step;
    slider_item->value_min = min;
    slider_item->value_max = max;
    slider_item->on_changed = on_changed;
    slider_item->on_changed_user_data = user_data;
    vision_ui_child_list_init(ui, (vision_ui_list_item_t*) slider_item, 0);
    vision_ui_owned_item_register(ui, (vision_ui_list_item_t*) slider_item);
    return (vision_ui_list_item_t*) slider_item;
}

vision_ui_list_item_t* vision_ui_list_user_item_new(
        const vision_ui_t* ui,
        const char* content,
        vision_ui_scene_cb init_function,
        vision_ui_scene_cb loop_function,
        vision_ui_scene_cb exit_function,
        void* user_data
) {
    vision_ui_user_item_t* user_item = vision_ui_malloc(ui, sizeof(vision_ui_user_item_t));
    if (user_item == NULL) {
        return NULL;
    }
    memset(user_item, 0, sizeof(vision_ui_user_item_t));
    user_item->base_item.type = UserItem;
    user_item->base_item.content = content;
    user_item->init_function = init_function;
    user_item->loop_function = loop_function;
    user_item->exit_function = exit_function;
    user_item->user_data = user_data;
    vision_ui_child_list_init(ui, (vision_ui_list_item_t*) user_item, 0);
    vision_ui_owned_item_register(ui, (vision_ui_list_item_t*) user_item);
    return (vision_ui_list_item_t*) user_item;
}

const vision_ui_selector_t* vision_ui_selector_instance_get(const vision_ui_t* ui) {
    assert(ui != NULL);
    return &ui->selector;
}

vision_ui_selector_t* vision_ui_selector_mutable_instance_get(vision_ui_t* ui) {
    assert(ui != NULL);
    return &ui->selector;
}

bool vision_ui_selector_t_selector_bind_item(vision_ui_t* ui, vision_ui_list_item_t* item) {
    assert(ui != NULL);
    if (item == NULL) {
        return false;
    }

    vision_ui_selector_t* selector = &ui->selector;

    // Find the item's index inside its parent.
    uint8_t temp_index = 0;
    if (item->parent != NULL) {
        for (uint8_t i = 0; i < item->parent->child_num; i++) {
            if (item->parent->child_list_item[i] == item) {
                temp_index = i;
                break;
            }
        }
    }

    // Coordinates are updated during refresh.
    if (selector->selected_item == NULL) {
        selector->y_selector = 2 * VISION_UI_SCREEN_HEIGHT; // Start off-screen so the first animation has motion.
        selector->h_selector = 160;
    }
    selector->selected_index = temp_index;
    selector->selected_item = item;

    return true;
}

static void vision_ui_selector_schedule_selection(
        vision_ui_t* ui,
        vision_ui_list_item_t* target_item,
        const uint8_t target_index
) {
    ui->selector.pending_selected_item = target_item;
    ui->selector.pending_selected_index = target_index;
    ui->selector.has_pending_selection = true;
}

void vision_ui_selector_go_next_item(vision_ui_t* ui) {
    assert(ui != NULL);
    if (ui->selector.selected_item == NULL) {
        return;
    }

    vision_ui_selector_t* selector = &ui->selector;
    if (selector->selected_item->type == SliderItem &&
        vision_ui_to_list_slider_item(selector->selected_item)->is_confirmed) {
        vision_ui_slider_item_t* selected_slider_item = vision_ui_to_list_slider_item(selector->selected_item);
        selected_slider_item->value += selected_slider_item->value_step;
        if (selected_slider_item->value >= selected_slider_item->value_max) {
            selected_slider_item->value = selected_slider_item->value_max;
        }
        if (selected_slider_item->on_changed != NULL) {
            selected_slider_item->on_changed(ui, selected_slider_item->value, selected_slider_item->on_changed_user_data);
        }
        return;
    }

    if (selector->selected_item->type == UserItem &&
        vision_ui_to_list_user_item(selector->selected_item)->in_user_item) {
        return;
    }

    // Wrap around at the end of the list.
    if (selector->selected_index == selector->selected_item->parent->child_num - 1) {
        selector->selected_item = selector->selected_item->parent->child_list_item[0];
        selector->selected_index = 0;
        return;
    }

    selector->selected_item = selector->selected_item->parent->child_list_item[++selector->selected_index];
}

void vision_ui_selector_go_prev_item(vision_ui_t* ui) {
    assert(ui != NULL);
    if (ui->selector.selected_item == NULL) {
        return;
    }

    vision_ui_selector_t* selector = &ui->selector;
    if (selector->selected_item->type == SliderItem &&
        vision_ui_to_list_slider_item(selector->selected_item)->is_confirmed) {
        vision_ui_slider_item_t* selected_slider_item = vision_ui_to_list_slider_item(selector->selected_item);
        selected_slider_item->value -= selected_slider_item->value_step;
        if (selected_slider_item->value <= selected_slider_item->value_min) {
            selected_slider_item->value = selected_slider_item->value_min;
        }
        if (selected_slider_item->on_changed != NULL) {
            selected_slider_item->on_changed(ui, selected_slider_item->value, selected_slider_item->on_changed_user_data);
        }
        return;
    }

    if (selector->selected_item->type == UserItem &&
        vision_ui_to_list_user_item(selector->selected_item)->in_user_item) {
        return;
    }

    // Wrap around at the start of the list.
    if (selector->selected_index == 0) {
        selector->selected_item =
                selector->selected_item->parent->child_list_item[selector->selected_item->parent->child_num - 1];
        selector->selected_index = selector->selected_item->parent->child_num - 1;
        return;
    }

    selector->selected_item = selector->selected_item->parent->child_list_item[--selector->selected_index];
}

/** @brief Confirm the currently selected item.
 * @note If a list is selected, enter that list.
 * @note If a special item is selected, toggle or adjust its value.
 */
void vision_ui_selector_jump_to_selected_item(vision_ui_t* ui) {
    assert(ui != NULL);
    if (!ui->is_in_vision_ui || ui->selector.selected_item == NULL) {
        return;
    }

    vision_ui_selector_t* selector = &ui->selector;

    if (selector->selected_item->type == UserItem) {
        vision_ui_user_item_t* selected_user_item = vision_ui_to_list_user_item(selector->selected_item);

        // Already inside the user item and enter pressed again: no-op to avoid refresh/reset.
        if (selected_user_item->in_user_item) {
            return;
        }

        vision_ui_exit_animation_start(ui);
        // vision_ui_selector.selected_item->in_user_item = true;
        selected_user_item->entering_user_item = true;
        selected_user_item->exiting_user_item = false;
        selected_user_item->user_item_inited = false;
        selected_user_item->user_item_looping = false;
        return;
    }

    if (selector->selected_item->type == TitleItem) {
        vision_ui_selector_exit_current_item(ui);
        return;
    }

    if (selector->selected_item->type == SwitchItem) {
        vision_ui_switch_item_t* selected_switch_item = vision_ui_to_list_switch_item(selector->selected_item);
        selected_switch_item->value = !selected_switch_item->value;
        if (selected_switch_item->on_changed != NULL) {
            selected_switch_item->on_changed(ui, selected_switch_item->value, selected_switch_item->on_changed_user_data);
        }
        return;
    }

    if (selector->selected_item->type == SliderItem) {
        vision_ui_slider_item_t* selected_slider_item = vision_ui_to_list_slider_item(selector->selected_item);
        if (!selected_slider_item->is_confirmed) {
            selected_slider_item->is_confirmed = true; // Confirm the slider if it is not already confirmed.
            return;
        }
        if (selected_slider_item->is_confirmed) {
            selected_slider_item->is_confirmed = false;
            return;
        }
    }

    if (selector->selected_item->type == IconItem && selector->selected_item->child_num == 1) {
        vision_ui_list_item_t* only_child = selector->selected_item->child_list_item[0];
        if (only_child->type == UserItem) {
            vision_ui_exit_animation_start(ui);

            // jump directly into the user item instead of showing its list wrapper
            only_child->y_list_item = 0;
            uint8_t icon_index = 0;
            if (selector->selected_item->parent != NULL) {
                for (uint8_t i = 0; i < selector->selected_item->parent->child_num; ++i) {
                    if (selector->selected_item->parent->child_list_item[i] == selector->selected_item) {
                        icon_index = i;
                        break;
                    }
                }
            }
            selector->selected_index = icon_index;
            vision_ui_selector_schedule_selection(ui, only_child, icon_index);

            vision_ui_user_item_t* user_item = vision_ui_to_list_user_item(only_child);
            user_item->entering_user_item = true;
            user_item->exiting_user_item = false;
            user_item->user_item_inited = false;
            user_item->user_item_looping = false;
            return;
        }
    }

    if (selector->selected_item->child_num == 0) {
        return;
    }

    vision_ui_exit_animation_start(ui);

    // Reset child item positions so the enter animation can replay.
    for (uint8_t i = 0; i < selector->selected_item->child_num; i++) {
        selector->selected_item->child_list_item[i]->y_list_item = 0;
    }

    vision_ui_selector_schedule_selection(ui, selector->selected_item->child_list_item[0], 0);
}

void vision_ui_selector_exit_current_item(vision_ui_t* ui) {
    assert(ui != NULL);
    if (ui->selector.selected_item == NULL) {
        return;
    }

    const vision_ui_selector_t* selector = &ui->selector;
    if (selector->selected_item->type == SliderItem &&
        vision_ui_to_list_slider_item(selector->selected_item)->is_confirmed) {
        // Exit slider adjustment mode when the slider is already confirmed.
        vision_ui_slider_item_t* selected_slider_item = vision_ui_to_list_slider_item(selector->selected_item);

        if (selected_slider_item->is_confirmed) {
            selected_slider_item->is_confirmed = false;
            return;
        }
    }

    if (selector->selected_item->type == UserItem &&
        vision_ui_to_list_user_item(selector->selected_item)->in_user_item) {
        vision_ui_exit_animation_start(ui); // Restart the exit animation for the user item.
        // vision_ui_selector.selected_item->in_user_item = false;
        vision_ui_user_item_t* selected_user_item = vision_ui_to_list_user_item(selector->selected_item);
        selected_user_item->entering_user_item = false;
        selected_user_item->exiting_user_item = true;
        selected_user_item->user_item_inited = false;
        selected_user_item->user_item_looping = false;
        return;
    }

    if (selector->selected_item->parent->layer == 0 && ui->is_in_vision_ui) {
        if (VISION_UI_ALLOW_EXIT_BY_USER) {
            ui->is_in_vision_ui = false;
        }
        return;
    }

    vision_ui_exit_animation_start(ui);

    // Reset the grandparent's child item positions so the exit animation can replay.
    for (uint8_t i = 0; i < selector->selected_item->parent->parent->child_num; i++) {
        selector->selected_item->parent->parent->child_list_item[i]->y_list_item = 0;
    }

    // Find the parent item's index inside its own parent.
    uint8_t temp_index = 0;
    for (uint8_t i = 0; i < selector->selected_item->parent->parent->child_num; i++) {
        if (selector->selected_item->parent->parent->child_list_item[i] == selector->selected_item->parent) {
            temp_index = i;
            break;
        }
    }
    vision_ui_selector_schedule_selection(ui, selector->selected_item->parent, temp_index);
}

bool vision_ui_root_item_set(vision_ui_t* ui, vision_ui_list_item_t* item) {
    assert(ui != NULL);
    if (item == NULL) {
        return false;
    }
    ui->root_item = item;
    return true;
}

vision_ui_list_item_t* vision_ui_root_list_get(const vision_ui_t* ui) {
    assert(ui != NULL);
    assert(ui->root_item != NULL);
    return ui->root_item;
}

bool vision_ui_list_push_item(vision_ui_t* ui, vision_ui_list_item_t* parent, vision_ui_list_item_t* child) {
    assert(ui != NULL);
    if (parent == NULL || child == NULL) {
        return false;
    }
    if (parent->child_num >= parent->capacity) {
        return false;
    }
    if (parent->layer >= VISION_UI_MAX_LIST_LAYER) {
        return false;
    }
    if (parent->icon_view_mode && child->type != IconItem) {
        return false;
    }

    child->layer = parent->layer + 1;

    float next_y = VISION_UI_LIST_TITLE_TO_DISPLAY_TOP_PADDING;
    if (parent->child_num > 0) {
        const vision_ui_list_item_t* last_child = parent->child_list_item[parent->child_num - 1];
        const uint8_t gap_after_last = (last_child->type == TitleItem) ? VISION_UI_LIST_TITLE_TO_FRAME_PADDING
                                                                       : VISION_UI_LIST_FRAME_BETWEEN_PADDING;
        next_y = last_child->y_list_item_trg + VISION_UI_LIST_FRAME_FIXED_HEIGHT + gap_after_last;
    } else if (child->type != TitleItem) {
        // Keep the top padding even when there is no title.
        next_y = VISION_UI_LIST_TITLE_TO_DISPLAY_TOP_PADDING;
    }
    child->y_list_item_trg = next_y;

    const vision_ui_list_item_t* selected_item = vision_ui_selector_instance_get(ui)->selected_item;
    const bool selector_needs_root_binding = selected_item == NULL || selected_item->parent == NULL;
    if (parent == vision_ui_root_list_get(ui) && selector_needs_root_binding) {
        vision_ui_list_item_t* first_root_item = parent->child_num > 0 ? parent->child_list_item[0] : child;
        vision_ui_selector_t_selector_bind_item(ui, first_root_item); // Initialize and bind the selector.
        vision_ui_camera_bind_selector(ui, &ui->selector); // Initialize and bind the camera.
    }

    parent->child_list_item[parent->child_num++] = child;
    child->parent = parent;

    return true;
}

const vision_ui_camera_t* vision_ui_camera_instance_get(const vision_ui_t* ui) {
    assert(ui != NULL);
    return &ui->camera;
}

vision_ui_camera_t* vision_ui_camera_mutable_instance_get(vision_ui_t* ui) {
    assert(ui != NULL);
    return &ui->camera;
}

void vision_ui_camera_instance_x_trg_set(vision_ui_t* ui, const float x_trg) {
    assert(ui != NULL);
    ui->camera.x_camera_trg = x_trg;
}

void vision_ui_camera_instance_y_trg_set(vision_ui_t* ui, const float y_trg) {
    assert(ui != NULL);
    ui->camera.y_camera_trg = y_trg;
}

void vision_ui_camera_bind_selector(vision_ui_t* ui, vision_ui_selector_t* selector) {
    assert(ui != NULL);
    if (selector == NULL) {
        return;
    }

    ui->camera.selector = selector; // Coordinates are updated during refresh.
}
