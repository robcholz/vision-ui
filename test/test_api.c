#include "mock_driver.h"
#include "unity.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <vision_ui_config.h>

typedef struct callback_state_t {
    int allocator_mallocs;
    int allocator_frees;
    int switch_changed_calls;
    int slider_changed_calls;
    int user_init_calls;
    int user_loop_calls;
    int user_exit_calls;
    bool last_switch_value;
    int16_t last_slider_value;
} callback_state_t;

typedef struct alloc_header_t {
    size_t size;
} alloc_header_t;

static vision_ui_font_t test_font(const uintptr_t id, const int8_t top, const int8_t bottom) {
    vision_ui_font_t font = {(const void*) id, top, bottom};
    return font;
}

static void switch_changed_cb(vision_ui_t* ui, const bool value, void* user_data) {
    (void) ui;
    callback_state_t* state = (callback_state_t*) user_data;
    state->switch_changed_calls++;
    state->last_switch_value = value;
}

static void slider_changed_cb(vision_ui_t* ui, const int16_t value, void* user_data) {
    (void) ui;
    callback_state_t* state = (callback_state_t*) user_data;
    state->slider_changed_calls++;
    state->last_slider_value = value;
}

static void user_init_cb(vision_ui_t* ui, void* user_data) {
    (void) ui;
    ((callback_state_t*) user_data)->user_init_calls++;
}

static void user_loop_cb(vision_ui_t* ui, void* user_data) {
    (void) ui;
    ((callback_state_t*) user_data)->user_loop_calls++;
}

static void user_exit_cb(vision_ui_t* ui, void* user_data) {
    (void) ui;
    ((callback_state_t*) user_data)->user_exit_calls++;
}

static void* counting_allocator(const vision_alloc_op_t op, const size_t size, const size_t count, void* ptr) {
    static callback_state_t* state = NULL;
    if (op == VisionAllocFree && ptr == NULL) {
        state = (callback_state_t*) (uintptr_t) count;
        return NULL;
    }

    if (op == VisionAllocMalloc) {
        alloc_header_t* header = (alloc_header_t*) malloc(sizeof(*header) + size);
        if (header == NULL) {
            return NULL;
        }
        header->size = size;
        if (state != NULL) {
            state->allocator_mallocs++;
        }
        return header + 1;
    }

    if (op == VisionAllocCalloc) {
        const size_t total = size * count;
        alloc_header_t* header = (alloc_header_t*) calloc(1, sizeof(*header) + total);
        if (header == NULL) {
            return NULL;
        }
        header->size = total;
        if (state != NULL) {
            state->allocator_mallocs++;
        }
        return header + 1;
    }

    if (op == VisionAllocFree && ptr != NULL) {
        alloc_header_t* header = ((alloc_header_t*) ptr) - 1;
        if (state != NULL) {
            state->allocator_frees++;
        }
        free(header);
    }
    return NULL;
}

static void register_allocator_state(callback_state_t* state) {
    counting_allocator(VisionAllocFree, 0, (size_t) (uintptr_t) state, NULL);
}

static void init_ui_with_driver(vision_ui_t* ui, mock_driver_t* driver) {
    vision_ui_init(ui);
    mock_driver_init(driver);
    vision_ui_driver_t binding = mock_driver_descriptor(driver);
    vision_ui_init_driver(ui, &binding);
    vision_ui_font_set(ui, test_font(1u, 1, 0));
    vision_ui_font_set_title(ui, test_font(2u, 2, 0));
    vision_ui_font_set_subtitle(ui, test_font(3u, 0, 1));
    vision_ui_minifont_set(ui, test_font(4u, 0, 0));
}

static void prepare_basic_list_ui(vision_ui_t* ui, mock_driver_t* driver, callback_state_t* state) {
    init_ui_with_driver(ui, driver);

    vision_ui_list_item_t* root = vision_ui_list_item_new(ui, 3, false, "root");
    vision_ui_list_item_t* switch_item = vision_ui_list_switch_item_new(ui, "switch", false, switch_changed_cb, state);
    vision_ui_list_item_t* slider_item =
            vision_ui_list_slider_item_new(ui, "slider", 3, 2, 0, 5, slider_changed_cb, state);
    vision_ui_list_item_t* title_item = vision_ui_list_title_item_new(ui, "title");

    TEST_ASSERT_NOT_NULL(root);
    TEST_ASSERT_EQUAL_INT(VisionUiRootItemSetOk, vision_ui_root_item_set(ui, root));
    TEST_ASSERT_EQUAL_INT(VisionUiListPushItemOk, vision_ui_list_push_item(ui, root, switch_item));
    TEST_ASSERT_EQUAL_INT(VisionUiListPushItemOk, vision_ui_list_push_item(ui, root, slider_item));
    TEST_ASSERT_EQUAL_INT(VisionUiListPushItemOk, vision_ui_list_push_item(ui, root, title_item));
    TEST_ASSERT_EQUAL_INT(VisionUiCoreInitOk, vision_ui_core_init(ui));
    vision_ui_render_init(ui);
}

static void finish_pending_transition(vision_ui_t* ui, mock_driver_t* driver) {
    vision_ui_exit_animation_set_is_finished(ui);
    mock_driver_advance_time(driver, 16);
    vision_ui_step_render(ui);
}

static void test_lifecycle_fonts_and_allocator(void) {
    vision_ui_t ui;
    mock_driver_t driver;
    callback_state_t state = {0};

    init_ui_with_driver(&ui, &driver);
    TEST_ASSERT_TRUE(vision_ui_is_exited(&ui));
    TEST_ASSERT_FALSE(vision_ui_is_background_frozen(&ui));
    TEST_ASSERT_TRUE(vision_ui_exit_animation_is_finished(&ui));
    TEST_ASSERT_TRUE(vision_ui_enter_animation_is_finished(&ui));

    TEST_ASSERT_EQUAL_PTR(DEFAULT_LIST_ICON.list_header, vision_ui_list_icon_get_current(&ui).list_header);
    TEST_ASSERT_EQUAL_PTR((const void*) 4u, vision_ui_minifont_get(&ui).font);
    TEST_ASSERT_EQUAL_PTR((const void*) 1u, vision_ui_font_get(&ui).font);
    TEST_ASSERT_EQUAL_PTR((const void*) 2u, vision_ui_font_get_title(&ui).font);
    TEST_ASSERT_EQUAL_PTR((const void*) 3u, vision_ui_font_get_subtitle(&ui).font);

    vision_ui_render_init(&ui);
    TEST_ASSERT_FALSE(vision_ui_is_exited(&ui));
    TEST_ASSERT_EQUAL_PTR((const void*) 1u, vision_ui_driver_font_get(&ui).font);

    vision_ui_start_logo_set(&ui, (const uint8_t*) "logo", 25);
    TEST_ASSERT_EQUAL_PTR((const uint8_t*) "logo", ui.logo);
    TEST_ASSERT_EQUAL_UINT32(25, ui.logo_span);
    TEST_ASSERT_FALSE(ui.logo_finished);

    register_allocator_state(&state);
    vision_ui_allocator_set(&ui, counting_allocator);
    vision_ui_list_item_t* item = vision_ui_list_item_new(&ui, 2, false, "owned");
    TEST_ASSERT_NOT_NULL(item);
    vision_ui_destroy(&ui);
    TEST_ASSERT_EQUAL_INT(2, state.allocator_mallocs);
    TEST_ASSERT_EQUAL_INT(2, state.allocator_frees);

    vision_ui_t* heap_ui = vision_ui_create();
    TEST_ASSERT_NOT_NULL(heap_ui);
    vision_ui_destroy(heap_ui);
}

static void test_empty_and_zero_capacity_corner_cases(void) {
    vision_ui_t ui;
    mock_driver_t driver;

    init_ui_with_driver(&ui, &driver);

    vision_ui_list_item_t* empty_root = vision_ui_list_item_new(&ui, 0, false, "empty");
    vision_ui_list_item_t* zero_icon = vision_ui_list_icon_item_new(&ui, 0, (const uint8_t*) "i", "icon", "desc");
    TEST_ASSERT_NOT_NULL(empty_root);
    TEST_ASSERT_NOT_NULL(zero_icon);
    TEST_ASSERT_FALSE(empty_root->owns_child_list);
    TEST_ASSERT_NULL(empty_root->child_list_item);
    TEST_ASSERT_FALSE(zero_icon->owns_child_list);
    TEST_ASSERT_NULL(zero_icon->child_list_item);

    TEST_ASSERT_EQUAL_INT(VisionUiRootItemSetOk, vision_ui_root_item_set(&ui, empty_root));
    TEST_ASSERT_EQUAL_INT(VisionUiCoreInitOk, vision_ui_core_init(&ui));
    TEST_ASSERT_NULL(vision_ui_selector_instance_get(&ui)->selected_item);

    vision_ui_render_init(&ui);
    mock_driver_advance_time(&driver, 16);
    vision_ui_step_render(&ui);
    TEST_ASSERT_EQUAL_UINT32(1, driver.action_get_count);
    TEST_ASSERT_FALSE(vision_ui_is_exited(&ui));

    TEST_ASSERT_EQUAL_INT(VisionUiListPushItemParentFull, vision_ui_list_push_item(&ui, empty_root, zero_icon));

    vision_ui_destroy(&ui);
}

static void test_item_constructors_and_push_validations(void) {
    vision_ui_t ui;
    mock_driver_t driver;
    callback_state_t state = {0};

    init_ui_with_driver(&ui, &driver);

    TEST_ASSERT_EQUAL_INT(VisionUiCoreInitRootItemNotSet, vision_ui_core_init(&ui));
    TEST_ASSERT_EQUAL_INT(VisionUiRootItemSetItemInvalid, vision_ui_root_item_set(&ui, NULL));
    TEST_ASSERT_NULL(vision_ui_root_list_get(&ui));
    TEST_ASSERT_FALSE(vision_ui_selector_t_selector_bind_item(&ui, NULL));

    vision_ui_list_item_t* root = vision_ui_list_item_new(&ui, 2, false, "root");
    vision_ui_list_item_t* title = vision_ui_list_title_item_new(&ui, "title");
    vision_ui_list_item_t* switch_item = vision_ui_list_switch_item_new(&ui, "switch", true, switch_changed_cb, &state);
    vision_ui_list_item_t* slider_item =
            vision_ui_list_slider_item_new(&ui, "slider", 7, 3, 1, 9, slider_changed_cb, &state);
    vision_ui_list_item_t* icon_item = vision_ui_list_icon_item_new(&ui, 1, (const uint8_t*) "i", "icon", "desc");
    vision_ui_list_item_t* user_item =
            vision_ui_list_user_item_new(&ui, "user", user_init_cb, user_loop_cb, user_exit_cb, &state);
    TEST_ASSERT_NOT_NULL(root);
    TEST_ASSERT_NOT_NULL(title);
    TEST_ASSERT_NOT_NULL(switch_item);
    TEST_ASSERT_NOT_NULL(slider_item);
    TEST_ASSERT_NOT_NULL(icon_item);
    TEST_ASSERT_NOT_NULL(user_item);

    TEST_ASSERT_EQUAL_INT(VisionUiRootItemSetOk, vision_ui_root_item_set(&ui, root));
    TEST_ASSERT_EQUAL_PTR(root, vision_ui_root_list_get(&ui));
    TEST_ASSERT_EQUAL_INT(VisionUiListPushItemItemInvalid, vision_ui_list_push_item(&ui, NULL, title));
    TEST_ASSERT_EQUAL_INT(VisionUiListPushItemOk, vision_ui_list_push_item(&ui, root, title));
    TEST_ASSERT_EQUAL_INT(VisionUiListPushItemOk, vision_ui_list_push_item(&ui, root, switch_item));
    TEST_ASSERT_EQUAL_INT(VisionUiListPushItemParentFull, vision_ui_list_push_item(&ui, root, slider_item));
    TEST_ASSERT_EQUAL_SIZE_T(0, title->y_list_item_trg);
    TEST_ASSERT_EQUAL_SIZE_T(
            VISION_UI_LIST_FRAME_FIXED_HEIGHT + VISION_UI_LIST_TITLE_TO_FRAME_PADDING,
            (size_t) switch_item->y_list_item_trg
    );
    TEST_ASSERT_EQUAL_PTR(root, title->parent);

    vision_ui_list_item_t* icon_root = vision_ui_list_item_new(&ui, 1, true, "icons");
    TEST_ASSERT_EQUAL_INT(VisionUiListPushItemIconViewChildMismatch, vision_ui_list_push_item(&ui, icon_root, title));
    TEST_ASSERT_EQUAL_INT(VisionUiListPushItemOk, vision_ui_list_push_item(&ui, icon_root, icon_item));

    vision_ui_list_item_t artificial_parent = {0};
    artificial_parent.capacity = 1;
    artificial_parent.layer = VISION_UI_MAX_LIST_LAYER;
    artificial_parent.child_list_item = &title;
    TEST_ASSERT_EQUAL_INT(
            VisionUiListPushItemMaxLayerExceeded, vision_ui_list_push_item(&ui, &artificial_parent, title)
    );

    TEST_ASSERT_EQUAL_PTR((vision_ui_switch_item_t*) switch_item, vision_ui_to_list_switch_item(switch_item));
    TEST_ASSERT_EQUAL_PTR((vision_ui_slider_item_t*) slider_item, vision_ui_to_list_slider_item(slider_item));
    TEST_ASSERT_EQUAL_PTR((vision_ui_icon_item_t*) icon_item, vision_ui_to_list_icon_item(icon_item));
    TEST_ASSERT_EQUAL_PTR((vision_ui_user_item_t*) user_item, vision_ui_to_list_user_item(user_item));

    TEST_ASSERT_EQUAL_PTR(&ui.selector, vision_ui_selector_mutable_instance_get(&ui));
    TEST_ASSERT_EQUAL_PTR(&ui.selector, vision_ui_selector_instance_get(&ui));
    TEST_ASSERT_EQUAL_PTR(&ui.camera, vision_ui_camera_mutable_instance_get(&ui));
    TEST_ASSERT_EQUAL_PTR(&ui.camera, vision_ui_camera_instance_get(&ui));
    vision_ui_camera_instance_x_trg_set(&ui, 12.5f);
    vision_ui_camera_instance_y_trg_set(&ui, -3.5f);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 12.5f, ui.camera.x_camera_trg);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, -3.5f, ui.camera.y_camera_trg);
    vision_ui_camera_bind_selector(&ui, &ui.selector);
    TEST_ASSERT_EQUAL_PTR(&ui.selector, ui.camera.selector);

    vision_ui_destroy(&ui);
}

static void test_root_first_push_auto_binds_selector(void) {
    vision_ui_t ui;
    mock_driver_t driver;

    init_ui_with_driver(&ui, &driver);

    vision_ui_list_item_t* root = vision_ui_list_item_new(&ui, 2, false, "root");
    vision_ui_list_item_t* child_a = vision_ui_list_switch_item_new(&ui, "a", false, NULL, NULL);
    vision_ui_list_item_t* child_b = vision_ui_list_switch_item_new(&ui, "b", true, NULL, NULL);

    TEST_ASSERT_EQUAL_INT(VisionUiRootItemSetOk, vision_ui_root_item_set(&ui, root));
    TEST_ASSERT_EQUAL_INT(VisionUiListPushItemOk, vision_ui_list_push_item(&ui, root, child_a));
    TEST_ASSERT_EQUAL_PTR(child_a, vision_ui_selector_instance_get(&ui)->selected_item);
    TEST_ASSERT_EQUAL_PTR(&ui.selector, vision_ui_camera_instance_get(&ui)->selector);
    TEST_ASSERT_EQUAL_INT(VisionUiListPushItemOk, vision_ui_list_push_item(&ui, root, child_b));
    TEST_ASSERT_EQUAL_PTR(child_a, vision_ui_selector_instance_get(&ui)->selected_item);

    vision_ui_destroy(&ui);
}

static void test_selector_navigation_switch_slider_and_wrap(void) {
    vision_ui_t ui;
    mock_driver_t driver;
    callback_state_t state = {0};

    prepare_basic_list_ui(&ui, &driver, &state);
    const vision_ui_selector_t* selector = vision_ui_selector_instance_get(&ui);

    TEST_ASSERT_EQUAL_STRING("switch", selector->selected_item->content);
    vision_ui_selector_go_next_item(&ui);
    TEST_ASSERT_EQUAL_STRING("slider", selector->selected_item->content);
    vision_ui_selector_go_next_item(&ui);
    TEST_ASSERT_EQUAL_STRING("title", selector->selected_item->content);
    vision_ui_selector_go_next_item(&ui);
    TEST_ASSERT_EQUAL_STRING("switch", selector->selected_item->content);
    vision_ui_selector_go_prev_item(&ui);
    TEST_ASSERT_EQUAL_STRING("title", selector->selected_item->content);

    vision_ui_selector_go_next_item(&ui);
    TEST_ASSERT_EQUAL_STRING("switch", selector->selected_item->content);
    vision_ui_selector_jump_to_selected_item(&ui);
    TEST_ASSERT_EQUAL_INT(1, state.switch_changed_calls);
    TEST_ASSERT_TRUE(state.last_switch_value);

    vision_ui_selector_go_next_item(&ui);
    TEST_ASSERT_EQUAL_STRING("slider", selector->selected_item->content);
    vision_ui_selector_jump_to_selected_item(&ui);
    TEST_ASSERT_TRUE(vision_ui_to_list_slider_item(selector->selected_item)->is_confirmed);
    vision_ui_selector_go_next_item(&ui);
    TEST_ASSERT_EQUAL_INT(1, state.slider_changed_calls);
    TEST_ASSERT_EQUAL_INT(5, state.last_slider_value);
    vision_ui_selector_go_next_item(&ui);
    TEST_ASSERT_EQUAL_INT(2, state.slider_changed_calls);
    TEST_ASSERT_EQUAL_INT(5, state.last_slider_value);
    vision_ui_selector_go_prev_item(&ui);
    TEST_ASSERT_EQUAL_INT(3, state.slider_changed_calls);
    TEST_ASSERT_EQUAL_INT(3, state.last_slider_value);
    vision_ui_selector_jump_to_selected_item(&ui);
    TEST_ASSERT_FALSE(vision_ui_to_list_slider_item(selector->selected_item)->is_confirmed);

    vision_ui_destroy(&ui);
}

static void test_slider_boundaries_and_title_exit_behavior(void) {
    vision_ui_t ui;
    mock_driver_t driver;
    callback_state_t state = {0};

    init_ui_with_driver(&ui, &driver);
    vision_ui_list_item_t* root = vision_ui_list_item_new(&ui, 2, false, "root");
    vision_ui_list_item_t* slider =
            vision_ui_list_slider_item_new(&ui, "slider", 0, 4, 0, 10, slider_changed_cb, &state);
    vision_ui_list_item_t* title = vision_ui_list_title_item_new(&ui, "title");

    TEST_ASSERT_EQUAL_INT(VisionUiRootItemSetOk, vision_ui_root_item_set(&ui, root));
    TEST_ASSERT_EQUAL_INT(VisionUiListPushItemOk, vision_ui_list_push_item(&ui, root, slider));
    TEST_ASSERT_EQUAL_INT(VisionUiListPushItemOk, vision_ui_list_push_item(&ui, root, title));
    TEST_ASSERT_EQUAL_INT(VisionUiCoreInitOk, vision_ui_core_init(&ui));
    vision_ui_render_init(&ui);

    vision_ui_selector_jump_to_selected_item(&ui);
    TEST_ASSERT_TRUE(vision_ui_to_list_slider_item(slider)->is_confirmed);
    vision_ui_selector_go_prev_item(&ui);
    TEST_ASSERT_EQUAL_INT(1, state.slider_changed_calls);
    TEST_ASSERT_EQUAL_INT(0, state.last_slider_value);
    vision_ui_selector_go_next_item(&ui);
    vision_ui_selector_go_next_item(&ui);
    vision_ui_selector_go_next_item(&ui);
    TEST_ASSERT_EQUAL_INT(10, state.last_slider_value);
    TEST_ASSERT_EQUAL_INT(4, state.slider_changed_calls);
    vision_ui_selector_exit_current_item(&ui);
    TEST_ASSERT_FALSE(vision_ui_to_list_slider_item(slider)->is_confirmed);

    vision_ui_selector_go_next_item(&ui);
    TEST_ASSERT_EQUAL_PTR(title, vision_ui_selector_instance_get(&ui)->selected_item);
    vision_ui_selector_jump_to_selected_item(&ui);
    TEST_ASSERT_FALSE(vision_ui_is_exited(&ui));

    vision_ui_destroy(&ui);
}

static void test_nested_navigation_and_user_item_transitions(void) {
    vision_ui_t ui;
    mock_driver_t driver;
    callback_state_t state = {0};

    init_ui_with_driver(&ui, &driver);
    vision_ui_list_item_t* root = vision_ui_list_item_new(&ui, 2, false, "root");
    vision_ui_list_item_t* submenu = vision_ui_list_item_new(&ui, 1, false, "submenu");
    vision_ui_list_item_t* sub_switch =
            vision_ui_list_switch_item_new(&ui, "sub-switch", false, switch_changed_cb, &state);
    vision_ui_list_item_t* user =
            vision_ui_list_user_item_new(&ui, "user", user_init_cb, user_loop_cb, user_exit_cb, &state);

    TEST_ASSERT_EQUAL_INT(VisionUiRootItemSetOk, vision_ui_root_item_set(&ui, root));
    TEST_ASSERT_EQUAL_INT(VisionUiListPushItemOk, vision_ui_list_push_item(&ui, submenu, sub_switch));
    TEST_ASSERT_EQUAL_INT(VisionUiListPushItemOk, vision_ui_list_push_item(&ui, root, submenu));
    TEST_ASSERT_EQUAL_INT(VisionUiListPushItemOk, vision_ui_list_push_item(&ui, root, user));
    TEST_ASSERT_EQUAL_INT(VisionUiCoreInitOk, vision_ui_core_init(&ui));
    vision_ui_render_init(&ui);

    vision_ui_selector_jump_to_selected_item(&ui);
    TEST_ASSERT_FALSE(vision_ui_exit_animation_is_finished(&ui));
    finish_pending_transition(&ui, &driver);
    TEST_ASSERT_EQUAL_STRING("sub-switch", vision_ui_selector_instance_get(&ui)->selected_item->content);

    vision_ui_selector_exit_current_item(&ui);
    finish_pending_transition(&ui, &driver);
    TEST_ASSERT_EQUAL_STRING("submenu", vision_ui_selector_instance_get(&ui)->selected_item->content);

    vision_ui_selector_go_next_item(&ui);
    TEST_ASSERT_EQUAL_STRING("user", vision_ui_selector_instance_get(&ui)->selected_item->content);
    vision_ui_selector_jump_to_selected_item(&ui);
    finish_pending_transition(&ui, &driver);
    TEST_ASSERT_EQUAL_INT(1, state.user_init_calls);
    TEST_ASSERT_EQUAL_INT(1, state.user_loop_calls);
    TEST_ASSERT_TRUE(vision_ui_to_list_user_item(vision_ui_selector_instance_get(&ui)->selected_item)->in_user_item);

    vision_ui_selector_exit_current_item(&ui);
    finish_pending_transition(&ui, &driver);
    TEST_ASSERT_EQUAL_INT(1, state.user_exit_calls);
    TEST_ASSERT_FALSE(vision_ui_to_list_user_item(user)->in_user_item);

    vision_ui_destroy(&ui);
}

static void test_icon_view_direct_user_item_flow(void) {
    vision_ui_t ui;
    mock_driver_t driver;
    callback_state_t state = {0};

    init_ui_with_driver(&ui, &driver);
    vision_ui_list_item_t* root = vision_ui_list_item_new(&ui, 1, true, "icons");
    vision_ui_list_item_t* icon = vision_ui_list_icon_item_new(&ui, 1, (const uint8_t*) "bmp", "Card", "Detail");
    vision_ui_list_item_t* user =
            vision_ui_list_user_item_new(&ui, "detail", user_init_cb, user_loop_cb, user_exit_cb, &state);

    TEST_ASSERT_EQUAL_INT(VisionUiRootItemSetOk, vision_ui_root_item_set(&ui, root));
    TEST_ASSERT_EQUAL_INT(VisionUiListPushItemOk, vision_ui_list_push_item(&ui, icon, user));
    TEST_ASSERT_EQUAL_INT(VisionUiListPushItemOk, vision_ui_list_push_item(&ui, root, icon));
    TEST_ASSERT_EQUAL_INT(VisionUiCoreInitOk, vision_ui_core_init(&ui));
    vision_ui_render_init(&ui);

    vision_ui_selector_jump_to_selected_item(&ui);
    finish_pending_transition(&ui, &driver);
    TEST_ASSERT_EQUAL_STRING("detail", vision_ui_selector_instance_get(&ui)->selected_item->content);
    TEST_ASSERT_EQUAL_INT(1, state.user_init_calls);
    TEST_ASSERT_EQUAL_INT(1, state.user_loop_calls);

    vision_ui_selector_exit_current_item(&ui);
    finish_pending_transition(&ui, &driver);
    TEST_ASSERT_EQUAL_STRING("Card", vision_ui_selector_instance_get(&ui)->selected_item->content);
    TEST_ASSERT_EQUAL_INT(1, state.user_exit_calls);

    vision_ui_destroy(&ui);
}

static void test_notifications_alerts_and_widget_render(void) {
    vision_ui_t ui;
    mock_driver_t driver;
    callback_state_t state = {0};

    prepare_basic_list_ui(&ui, &driver, &state);
    TEST_ASSERT_EQUAL_INT(VisionUiNotificationPushContentInvalid, vision_ui_notification_push(&ui, NULL, 10));
    TEST_ASSERT_EQUAL_INT(VisionUiNotificationPushOk, vision_ui_notification_push(&ui, "A", 20));
    TEST_ASSERT_TRUE(vision_ui_notification_instance_get(&ui)->is_running);
    TEST_ASSERT_EQUAL_INT(VisionUiNotificationPushOk, vision_ui_notification_push(&ui, "B", 30));
    TEST_ASSERT_TRUE(vision_ui_notification_instance_get(&ui)->is_dismissing);
    TEST_ASSERT_EQUAL_STRING("B", vision_ui_notification_instance_get(&ui)->pending_content);

    vision_ui_notification_mutable_instance_get(&ui)->y_notification =
            vision_ui_notification_instance_get(&ui)->y_notification_trg;
    mock_driver_advance_time(&driver, VISION_UI_NOTIFICATION_DISMISS_DURATION_MS + 1);
    vision_ui_widget_render(&ui);
    TEST_ASSERT_EQUAL_STRING("B", vision_ui_notification_instance_get(&ui)->content);
    TEST_ASSERT_FALSE(vision_ui_notification_instance_get(&ui)->is_dismissing);

    vision_ui_notification_mutable_instance_get(&ui)->y_notification =
            vision_ui_notification_instance_get(&ui)->y_notification_trg;
    mock_driver_advance_time(&driver, 31);
    vision_ui_widget_render(&ui);
    TEST_ASSERT_TRUE(vision_ui_notification_instance_get(&ui)->is_dismissing);

    vision_ui_alert_push(&ui, "Alert", 10);
    TEST_ASSERT_TRUE(vision_ui_alert_instance_get(&ui)->is_running);
    vision_ui_alert_mutable_instance_get(&ui)->y_alert = vision_ui_alert_instance_get(&ui)->y_alert_trg;
    mock_driver_advance_time(&driver, 11);
    vision_ui_widget_render(&ui);
    TEST_ASSERT_TRUE(vision_ui_alert_instance_get(&ui)->y_alert_trg < 0);
    vision_ui_alert_mutable_instance_get(&ui)->y_alert = vision_ui_alert_instance_get(&ui)->y_alert_trg;
    vision_ui_widget_render(&ui);
    TEST_ASSERT_FALSE(vision_ui_alert_instance_get(&ui)->is_running);
    TEST_ASSERT_TRUE(driver.box_r_draw_count > 0);
    TEST_ASSERT_TRUE(driver.str_utf8_draw_count > 0);

    vision_ui_destroy(&ui);
}

static void test_notification_queue_replacement_corner_cases(void) {
    vision_ui_t ui;
    mock_driver_t driver;
    callback_state_t state = {0};

    prepare_basic_list_ui(&ui, &driver, &state);

    TEST_ASSERT_EQUAL_INT(VisionUiNotificationPushOk, vision_ui_notification_push(&ui, "first", 50));
    TEST_ASSERT_EQUAL_INT(VisionUiNotificationPushOk, vision_ui_notification_push(&ui, "second", 60));
    TEST_ASSERT_EQUAL_INT(VisionUiNotificationPushOk, vision_ui_notification_push(&ui, "third", 70));
    TEST_ASSERT_TRUE(vision_ui_notification_instance_get(&ui)->has_pending);
    TEST_ASSERT_EQUAL_STRING("third", vision_ui_notification_instance_get(&ui)->pending_content);
    TEST_ASSERT_EQUAL_UINT16(70, vision_ui_notification_instance_get(&ui)->pending_span);

    vision_ui_notification_mutable_instance_get(&ui)->y_notification =
            vision_ui_notification_instance_get(&ui)->y_notification_trg;
    mock_driver_advance_time(&driver, VISION_UI_NOTIFICATION_DISMISS_DURATION_MS + 1);
    vision_ui_widget_render(&ui);
    TEST_ASSERT_EQUAL_STRING("third", vision_ui_notification_instance_get(&ui)->content);
    TEST_ASSERT_FALSE(vision_ui_notification_instance_get(&ui)->has_pending);

    vision_ui_notification_mutable_instance_get(&ui)->y_notification =
            vision_ui_notification_instance_get(&ui)->y_notification_trg;
    mock_driver_advance_time(&driver, 71);
    vision_ui_widget_render(&ui);
    TEST_ASSERT_TRUE(vision_ui_notification_instance_get(&ui)->is_dismissing);
    vision_ui_notification_mutable_instance_get(&ui)->y_notification =
            vision_ui_notification_instance_get(&ui)->y_notification_trg;
    mock_driver_advance_time(&driver, VISION_UI_NOTIFICATION_DISMISS_DURATION_MS + 1);
    vision_ui_widget_render(&ui);
    TEST_ASSERT_FALSE(vision_ui_notification_instance_get(&ui)->is_running);
    TEST_ASSERT_NULL(vision_ui_notification_instance_get(&ui)->pending_content);

    vision_ui_destroy(&ui);
}

static void test_list_icon_and_renderer_entry_points(void) {
    vision_ui_t ui;
    mock_driver_t driver;
    callback_state_t state = {0};
    vision_ui_icon_t icons = DEFAULT_LIST_ICON;

    prepare_basic_list_ui(&ui, &driver, &state);
    icons.header_width = 9;
    vision_ui_list_icon_set(&ui, icons);
    TEST_ASSERT_EQUAL_SIZE_T(9, vision_ui_list_icon_get_current(&ui).header_width);

    vision_ui_list_render(&ui);
    TEST_ASSERT_TRUE(driver.line_v_draw_count > 0);
    TEST_ASSERT_TRUE(driver.box_draw_count > 0);
    TEST_ASSERT_TRUE(driver.str_utf8_draw_count > 0);

    mock_driver_seed_checkerboard(&driver);
    vision_ui_exit_animation_start(&ui);
    vision_ui_exit_animation_render(&ui, VISION_UI_EXIT_ANIMATION_DURATION_MS);
    TEST_ASSERT_TRUE(vision_ui_exit_animation_is_finished(&ui));
    TEST_ASSERT_FALSE(vision_ui_enter_animation_is_finished(&ui));
    vision_ui_enter_animation_render(&ui, VISION_UI_ENTER_ANIMATION_DURATION_MS);
    TEST_ASSERT_TRUE(vision_ui_enter_animation_is_finished(&ui));
    TEST_ASSERT_TRUE(driver.buffer_pointer_get_count > 0);

    vision_ui_destroy(&ui);
}

static void test_step_render_logo_actions_and_freeze(void) {
    vision_ui_t ui;
    mock_driver_t driver;
    callback_state_t state = {0};
    static const uint8_t logo_bitmap[8] = {0};

    prepare_basic_list_ui(&ui, &driver, &state);
    vision_ui_start_logo_set(&ui, logo_bitmap, 10);
    vision_ui_step_render(&ui);
    TEST_ASSERT_EQUAL_INT(1, driver.bmp_draw_count);

    mock_driver_advance_time(&driver, 20);
    vision_ui_step_render(&ui);
    TEST_ASSERT_EQUAL_INT(1, driver.buffer_clear_count);

    mock_driver_set_action(&driver, UiActionGoNext);
    mock_driver_advance_time(&driver, 16);
    vision_ui_step_render(&ui);
    TEST_ASSERT_EQUAL_STRING("slider", vision_ui_selector_instance_get(&ui)->selected_item->content);

    vision_ui_alert_push(&ui, "freeze", 100);
    mock_driver_advance_time(&driver, 16);
    vision_ui_step_render(&ui);
    TEST_ASSERT_TRUE(vision_ui_is_background_frozen(&ui));

    mock_driver_set_action(&driver, UiActionGoNext);
    mock_driver_advance_time(&driver, 16);
    vision_ui_step_render(&ui);
    TEST_ASSERT_EQUAL_STRING("slider", vision_ui_selector_instance_get(&ui)->selected_item->content);

    mock_driver_set_action(&driver, UiActionExit);
    mock_driver_advance_time(&driver, 16);
    vision_ui_step_render(&ui);
    TEST_ASSERT_FALSE(vision_ui_is_exited(&ui));

    vision_ui_destroy(&ui);
}

static void test_animation_helpers_and_driver_contract(void) {
    vision_ui_t ui;
    mock_driver_t driver;

    init_ui_with_driver(&ui, &driver);
    vision_ui_driver_font_set(&ui, test_font(1u, 1, 0));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, vision_ui_smoothstep(-1.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, vision_ui_smoothstep(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.5f, vision_ui_smoothstep(0.5f));
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, vision_ui_smoothstep(2.0f));

    float pos = 0.0f;
    vision_ui_animation_s_curve(&pos, 0.4f, 90.0f, 0.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.4f, pos);
    pos = 0.0f;
    for (int i = 0; i < 32; ++i) {
        vision_ui_animation_s_curve(&pos, 10.0f, 90.0f, 16.0f);
    }
    TEST_ASSERT_FLOAT_WITHIN(0.6f, 10.0f, pos);

    pos = 0.0f;
    float velocity = 0.0f;
    for (int i = 0; i < 64; ++i) {
        vision_ui_animation_2nd_ode_no_overshoot(&pos, &velocity, 5.0f, 90.0f, 16.0f);
    }
    TEST_ASSERT_FLOAT_WITHIN(0.6f, 5.0f, pos);

    pos = 0.0f;
    velocity = 0.0f;
    for (int i = 0; i < 64; ++i) {
        vision_ui_animation_2nd_ode_slight_overshoot(&pos, &velocity, -4.0f, 80.0f, 16.0f);
    }
    TEST_ASSERT_FLOAT_WITHIN(0.6f, -4.0f, pos);

    vision_ui_driver_font_mode_set(&ui, 1);
    vision_ui_driver_font_direction_set(&ui, 2);
    vision_ui_driver_delay(&ui, 7);
    TEST_ASSERT_EQUAL_UINT32(7, driver.total_delay_ms);

    TEST_ASSERT_EQUAL_UINT16(18, vision_ui_driver_str_width_get(&ui, "abc"));
    TEST_ASSERT_EQUAL_UINT16(18, vision_ui_driver_str_utf8_width_get(&ui, "abc"));
    TEST_ASSERT_EQUAL_UINT16(9, vision_ui_driver_str_height_get(&ui));

    vision_ui_driver_str_draw(&ui, 1, 2, "abc");
    vision_ui_driver_str_utf8_draw(&ui, 3, 4, "xyz");
    vision_ui_driver_pixel_draw(&ui, 1, 1);
    vision_ui_driver_circle_draw(&ui, 2, 2, 1);
    vision_ui_driver_disc_draw(&ui, 3, 3, 1);
    vision_ui_driver_box_r_draw(&ui, 4, 4, 2, 2, 1);
    vision_ui_driver_box_draw(&ui, 6, 6, 2, 2);
    vision_ui_driver_frame_draw(&ui, 8, 8, 3, 3);
    vision_ui_driver_frame_r_draw(&ui, 10, 10, 3, 3, 1);
    vision_ui_driver_line_h_draw(&ui, 0, 12, 5);
    vision_ui_driver_line_v_draw(&ui, 12, 0, 5);
    vision_ui_driver_line_draw(&ui, 0, 0, 5, 5);
    vision_ui_driver_line_h_dotted_draw(&ui, 0, 14, 6);
    vision_ui_driver_line_v_dotted_draw(&ui, 14, 0, 6);
    vision_ui_driver_bmp_draw(&ui, 0, 0, 2, 2, (const uint8_t*) "bm");
    vision_ui_driver_color_draw(&ui, 2);
    vision_ui_driver_buffer_clear(&ui);
    vision_ui_driver_color_draw(&ui, 1);
    vision_ui_driver_clip_window_set(&ui, 0, 0, 10, 10);
    vision_ui_driver_pixel_draw(&ui, 200, 200);
    vision_ui_driver_pixel_draw(&ui, 5, 5);
    TEST_ASSERT_TRUE((driver.buffer[((5u >> 3u) * VISION_UI_SCREEN_WIDTH) + 5u] & (1u << (5u & 0x7u))) != 0u);
    TEST_ASSERT_TRUE((driver.buffer[((200u >> 3u) * VISION_UI_SCREEN_WIDTH) + 200u] & (1u << (200u & 0x7u))) == 0u);
    vision_ui_driver_clip_window_reset(&ui);
    vision_ui_driver_buffer_area_send(&ui, 0, 0, 10, 10);
    vision_ui_driver_buffer_send(&ui);
    TEST_ASSERT_NOT_NULL(vision_ui_driver_buffer_pointer_get(&ui));
    vision_ui_driver_buffer_clear(&ui);
    TEST_ASSERT_TRUE((driver.buffer[((5u >> 3u) * VISION_UI_SCREEN_WIDTH) + 5u] & (1u << (5u & 0x7u))) == 0u);

    TEST_ASSERT_EQUAL_UINT32(1, driver.font_mode_set_count);
    TEST_ASSERT_EQUAL_UINT32(1, driver.font_direction_set_count);
    TEST_ASSERT_EQUAL_UINT32(1, driver.str_draw_count);
    TEST_ASSERT_EQUAL_UINT32(1, driver.str_utf8_draw_count);
    TEST_ASSERT_TRUE(driver.pixel_draw_count > 0);
    TEST_ASSERT_EQUAL_UINT32(1, driver.circle_draw_count);
    TEST_ASSERT_EQUAL_UINT32(1, driver.disc_draw_count);
    TEST_ASSERT_TRUE(driver.box_r_draw_count > 0);
    TEST_ASSERT_TRUE(driver.box_draw_count > 0);
    TEST_ASSERT_TRUE(driver.frame_draw_count > 0);
    TEST_ASSERT_TRUE(driver.frame_r_draw_count > 0);
    TEST_ASSERT_EQUAL_UINT32(1, driver.line_h_dotted_draw_count);
    TEST_ASSERT_EQUAL_UINT32(1, driver.line_v_dotted_draw_count);
    TEST_ASSERT_EQUAL_UINT32(1, driver.bmp_draw_count);
    TEST_ASSERT_EQUAL_UINT32(1, driver.buffer_area_send_count);
    TEST_ASSERT_EQUAL_UINT32(1, driver.buffer_send_count);
    TEST_ASSERT_EQUAL_UINT32(2, driver.buffer_clear_count);

    vision_ui_destroy(&ui);
}

extern void run_all_tests(void) {
    RUN_TEST(test_lifecycle_fonts_and_allocator);
    RUN_TEST(test_empty_and_zero_capacity_corner_cases);
    RUN_TEST(test_item_constructors_and_push_validations);
    RUN_TEST(test_root_first_push_auto_binds_selector);
    RUN_TEST(test_selector_navigation_switch_slider_and_wrap);
    RUN_TEST(test_slider_boundaries_and_title_exit_behavior);
    RUN_TEST(test_nested_navigation_and_user_item_transitions);
    RUN_TEST(test_icon_view_direct_user_item_flow);
    RUN_TEST(test_notifications_alerts_and_widget_render);
    RUN_TEST(test_notification_queue_replacement_corner_cases);
    RUN_TEST(test_list_icon_and_renderer_entry_points);
    RUN_TEST(test_step_render_logo_actions_and_freeze);
    RUN_TEST(test_animation_helpers_and_driver_contract);
}
