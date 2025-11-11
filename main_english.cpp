#include <tgmath.h>
#include <u8g2.h>

#include <vision/vision_ui.hpp>

#include "driver/u8g2.h"

#include "font/chinese.h"

u8g2_t U8G2;

uint32_t TIME_START = 0;
static int16_t Y_LOGO = 200;
static int16_t Y_VERSION = 200;
static int16_t Y_NAME = -200;
static int16_t Y_ASTRA = -200;
static int16_t Y_BOX = 200;
static int16_t X_BOARD = -200;
static int16_t Y_WIRE_1 = 200;
static int16_t Y_WIRE_2 = 200;

void animation(int16_t *pos, int16_t pos_trg, int16_t speed) {
    if (*pos != pos_trg) {
        if (fabs(*pos - pos_trg) <= 1.0f) {

            *pos = pos_trg;
        } else {
            *pos += (pos_trg - *pos) / ((100 - speed) / 1.0f);
        }
    }
}

void test_user_item_init_function() {
    TIME_START = vision_ui_driver_ticks_ms_get();
}

void test_user_item_loop_function() {
    uint32_t time = vision_ui_driver_ticks_ms_get();

    vision_ui_driver_color_draw(1);
    vision_ui_driver_box_r_draw(2, Y_BOX - 1, vision_ui_driver_str_utf8_width_get("「astraLauncher」") + 4,
                                vision_ui_driver_str_height_get() + 2, 1);
    vision_ui_driver_color_draw(2);
    vision_ui_driver_str_utf8_draw(4, Y_LOGO - 2, "「astraLauncher」");

    vision_ui_driver_color_draw(1);
    vision_ui_driver_str_draw(106, Y_VERSION, "v1.0");
    vision_ui_driver_str_utf8_draw(2, Y_NAME, "by Rational Works.");
    vision_ui_driver_str_utf8_draw(2, Y_ASTRA, "Powered by Astra UI Lite v1.1");
    vision_ui_driver_str_utf8_draw(2, Y_ASTRA + 14, "Engine.");
    vision_ui_driver_frame_draw(X_BOARD, 38, 28, 20);
    vision_ui_driver_frame_draw(X_BOARD + 2, 40, 24, 10);
    vision_ui_driver_box_draw(X_BOARD + 2, 40, 2, 10);
    vision_ui_driver_pixel_draw(X_BOARD + 25, 51);
    vision_ui_driver_pixel_draw(X_BOARD + 25, 53);
    vision_ui_driver_pixel_draw(X_BOARD + 25, 55);
    vision_ui_driver_box_draw(X_BOARD + 21, 51, 3, 2);
    vision_ui_driver_box_draw(X_BOARD + 21, 54, 3, 2);
    vision_ui_driver_box_draw(X_BOARD + 17, 53, 3, 3);

    vision_ui_driver_box_draw(X_BOARD + 12, 53, 4, 3);
    vision_ui_driver_box_draw(X_BOARD + 7, 53, 4, 3);
    vision_ui_driver_box_draw(X_BOARD + 2, 53, 4, 3);

    vision_ui_driver_box_draw(X_BOARD + 7, Y_WIRE_1, 4, 3);
    vision_ui_driver_line_v_draw(X_BOARD + 9, Y_WIRE_1 + 3, 3);
    vision_ui_driver_line_v_draw(X_BOARD + 8, Y_WIRE_1 + 6, 2);

    vision_ui_driver_box_draw(X_BOARD + 12, Y_WIRE_2, 4, 3);
    vision_ui_driver_line_v_draw(X_BOARD + 14, Y_WIRE_2 + 3, 3);
    vision_ui_driver_line_v_draw(X_BOARD + 15, Y_WIRE_2 + 6, 2);

    if (time - TIME_START > 300) {
        animation(&Y_LOGO, 15, 94);
    }
    if (time - TIME_START > 350) {
        animation(&Y_VERSION, 14, 88);
    }
    if (time - TIME_START > 400) {
        animation(&Y_BOX, 2, 92);
    }
    if (time - TIME_START > 450) {
        animation(&Y_ASTRA, 36, 91);
    }
    if (time - TIME_START > 500) {

        animation(&Y_NAME, 62, 94);
    }
    if (time - TIME_START > 550) {
        animation(&X_BOARD, 102, 92);
    }
    if (time - TIME_START > 620) {
        animation(&Y_WIRE_1, 56, 86);
    }
    if (time - TIME_START > 1400 && time - TIME_START < 1600) {
        vision_ui_driver_box_draw(X_BOARD + 5, 42, 19, 6);
    }
    if (time - TIME_START > 1800 && time - TIME_START < 1900) {
        vision_ui_driver_box_draw(X_BOARD + 5, 42, 19, 6);
    }
    if (time - TIME_START > 2200) {
        vision_ui_driver_box_draw(X_BOARD + 5, 42, 19, 6);
    }
    if (time - TIME_START > 2400) {


        animation(&Y_WIRE_2, 56, 86);
    }
}

void test_user_item_exit_function() {
    TIME_START = 0;
    Y_LOGO = 200;
    Y_VERSION = 200;
    Y_NAME = -200;
    Y_ASTRA = -200;
    Y_BOX = 200;
    X_BOARD = -200;
    Y_WIRE_1 = 200;
    Y_WIRE_2 = 200;
}

int main() {
    u8x8_Setup_SDL_128x64(u8g2_GetU8x8(&U8G2));
    u8g2_SetupBuffer(&U8G2, U8G2_BUFFER, 8, u8g2_ll_hvline_vertical_top_lsb, U8G2_R0);
    u8g2_InitDisplay(&U8G2);
    u8g2_SetPowerSave(&U8G2, 0);

    vision_ui_driver_bind(&U8G2);

    vision_ui_core_init();

    vision_ui_font_set((void *) u8g2_font_my_chinese);

    vision_ui_list_item_t *launcher_setting_list_item = vision_ui_list_item_new(10, "Board Settings");

    vision_ui_list_push_item(vision_ui_root_list_get(), launcher_setting_list_item);
    vision_ui_list_push_item(vision_ui_root_list_get(),
                             vision_ui_list_user_item_new(1, "About the Board...", test_user_item_init_function,
                                                          test_user_item_loop_function, test_user_item_exit_function));
    vision_ui_list_push_item(vision_ui_root_list_get(), vision_ui_list_switch_item_new(1, "Test Notification", false, [](bool b) {
                                 vision_ui_notification_push("Notification Test", 5000);
                             }));
    vision_ui_list_push_item(vision_ui_root_list_get(), vision_ui_list_switch_item_new(1, "Test Alert", false, [](bool b) {
                                 vision_ui_alert_push("Alert Test", 5000);
                             }));

    vision_ui_list_push_item(launcher_setting_list_item, vision_ui_list_switch_item_new(1, "Heartbeat LED", true, [](bool b) {}));
    vision_ui_list_push_item(launcher_setting_list_item, vision_ui_list_switch_item_new(1, "Reverse Keys", false, [](bool b) {}));
    vision_ui_list_push_item(launcher_setting_list_item,
                             vision_ui_list_slider_item_new(1, "Display Style", 1600, 5, 1, 9999, [](int16_t value) {}));
    vision_ui_list_push_item(launcher_setting_list_item, vision_ui_list_switch_item_new(1, "Invert Display", false, [](bool b) {}));
    vision_ui_list_push_item(launcher_setting_list_item, vision_ui_list_switch_item_new(1, "MCU Serial Channel", false, [](bool b) {}));
    vision_ui_list_push_item(launcher_setting_list_item,
                             vision_ui_list_switch_item_new(1, "External Serial Channel", false, [](bool b) {}));

    vision_ui_render_init();

    float prev_ms = vision_ui_driver_ticks_ms_get();
    float fps_timer = prev_ms;
    int frame_count = 0;

    constexpr float target_ms = 1000.0f / 60.0f;

    while (!vision_ui_is_exited()) {
        const float frame_begin = vision_ui_driver_ticks_ms_get();

        // render
        vision_ui_driver_buffer_clear();
        vision_ui_step_render();
        vision_ui_driver_buffer_send();

        // pace to 120 fps (include render time)
        float now_ms = vision_ui_driver_ticks_ms_get();
        float elapsed_ms = now_ms - frame_begin; // full frame so far
        if (elapsed_ms < target_ms) {
            // sleep the remainder (rounded; replace delay() with a microsecond sleep if you have one)
            vision_ui_driver_delay((uint32_t) lrintf(target_ms - elapsed_ms));
            now_ms = vision_ui_driver_ticks_ms_get(); // re-sample after sleep
        }

        // fps
        frame_count++;
        if (now_ms - fps_timer >= 1000.0f) {
            const float fps = (float) frame_count * 1000.0f / (now_ms - fps_timer);
            printf("FPS: %.1f\n", fps);
            fps_timer = now_ms;
            frame_count = 0;
        }

        prev_ms = frame_begin; // (optional) if you need per-frame dt later
    }

    return 0;
}
