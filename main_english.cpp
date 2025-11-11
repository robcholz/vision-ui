#include <tgmath.h>
#include <u8g2.h>

#include <vision/astra_ui_core.h>
#include <vision/astra_ui_item.h>

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

void animation(int16_t* pos, int16_t pos_trg, int16_t speed) {
    if (*pos != pos_trg) {
        if (fabs(*pos - pos_trg) <= 1.0f) *pos = pos_trg;
        else *pos += (pos_trg - *pos) / ((100 - speed) / 1.0f);
    }
}

void test_user_item_init_function() {
    TIME_START = get_ticks_ms();
}

void test_user_item_loop_function() {
    uint32_t time = get_ticks_ms();

    oled_set_draw_color(1);
    oled_draw_R_box(2, Y_BOX - 1, oled_get_UTF8_width("「astraLauncher」") + 4, oled_get_str_height() + 2, 1);
    oled_set_draw_color(2);
    oled_draw_UTF8(4, Y_LOGO - 2, "「astraLauncher」");

    oled_set_draw_color(1);
    oled_draw_str(106, Y_VERSION, "v1.0");
    oled_draw_UTF8(2, Y_NAME, "by Rational Works.");
    oled_draw_UTF8(2, Y_ASTRA, "Powered by Astra UI Lite v1.1");
    oled_draw_UTF8(2, Y_ASTRA + 14, "Engine.");
    oled_draw_frame(X_BOARD, 38, 28, 20);
    oled_draw_frame(X_BOARD + 2, 40, 24, 10);
    oled_draw_box(X_BOARD + 2, 40, 2, 10);
    oled_draw_pixel(X_BOARD + 25, 51);
    oled_draw_pixel(X_BOARD + 25, 53);
    oled_draw_pixel(X_BOARD + 25, 55);
    oled_draw_box(X_BOARD + 21, 51, 3, 2);
    oled_draw_box(X_BOARD + 21, 54, 3, 2);
    oled_draw_box(X_BOARD + 17, 53, 3, 3);

    oled_draw_box(X_BOARD + 12, 53, 4, 3);
    oled_draw_box(X_BOARD + 7, 53, 4, 3);
    oled_draw_box(X_BOARD + 2, 53, 4, 3);

    oled_draw_box(X_BOARD + 7, Y_WIRE_1, 4, 3);
    oled_draw_V_line(X_BOARD + 9, Y_WIRE_1 + 3, 3);
    oled_draw_V_line(X_BOARD + 8, Y_WIRE_1 + 6, 2);

    oled_draw_box(X_BOARD + 12, Y_WIRE_2, 4, 3);
    oled_draw_V_line(X_BOARD + 14, Y_WIRE_2 + 3, 3);
    oled_draw_V_line(X_BOARD + 15, Y_WIRE_2 + 6, 2);

    if (time - TIME_START > 300) animation(&Y_LOGO, 15, 94);
    if (time - TIME_START > 350) animation(&Y_VERSION, 14, 88);
    if (time - TIME_START > 400) animation(&Y_BOX, 2, 92);
    if (time - TIME_START > 450) animation(&Y_ASTRA, 36, 91);
    if (time - TIME_START > 500) animation(&Y_NAME, 62, 94);
    if (time - TIME_START > 550) animation(&X_BOARD, 102, 92);
    if (time - TIME_START > 620) animation(&Y_WIRE_1, 56, 86);
    if (time - TIME_START > 1400 && time - TIME_START < 1600) oled_draw_box(X_BOARD + 5, 42, 19, 6);
    if (time - TIME_START > 1800 && time - TIME_START < 1900) oled_draw_box(X_BOARD + 5, 42, 19, 6);
    if (time - TIME_START > 2200) oled_draw_box(X_BOARD + 5, 42, 19, 6);
    if (time - TIME_START > 2400) animation(&Y_WIRE_2, 56, 86);
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

    vision_ui_bind_driver(&U8G2);

    astra_init_core();

    astra_set_font((void*) u8g2_font_my_chinese);

    astra_list_item_t* launcher_setting_list_item = astra_new_list_item("Board Settings");

    astra_push_item_to_list(astra_get_root_list(), launcher_setting_list_item);
    astra_push_item_to_list(astra_get_root_list(), astra_new_switch_item("Switch Screen", true, [](bool b) {
    }));
    astra_push_item_to_list(astra_get_root_list(),
                            astra_new_user_item("Wiring Diagram...", test_user_item_init_function, test_user_item_loop_function,
                                                test_user_item_exit_function));
    astra_push_item_to_list(astra_get_root_list(),
                            astra_new_user_item("About the Board...", test_user_item_init_function, test_user_item_loop_function,
                                                test_user_item_exit_function));
    astra_push_item_to_list(astra_get_root_list(),
                            astra_new_switch_item("Test Alert", false, [](bool b) {
                                astra_push_pop_up("Hello", 5000);
                            }));

    astra_push_item_to_list(launcher_setting_list_item, astra_new_switch_item("Heartbeat LED", true, [](bool b) {
    }));
    astra_push_item_to_list(launcher_setting_list_item, astra_new_switch_item("Reverse Keys", false, [](bool b) {
    }));
    astra_push_item_to_list(launcher_setting_list_item, astra_new_slider_item("Display Style", 1600, 5, 1, 9999, [](int16_t value) {
    }));
    astra_push_item_to_list(launcher_setting_list_item, astra_new_switch_item("Invert Display", false, [](bool b) {
    }));
    astra_push_item_to_list(launcher_setting_list_item, astra_new_switch_item("MCU Serial Channel", false, [](bool b) {
    }));
    astra_push_item_to_list(launcher_setting_list_item, astra_new_switch_item("External Serial Channel", false, [](bool b) {
    }));

    vision_ui_render_init();

    float prev_ms = get_ticks_ms();
    float current_ms = prev_ms;
    float fps_timer = prev_ms;
    int frame_count = 0;

    constexpr float target_ms = 1000.0f / 60.0f;

    while (!vision_ui_is_exited()) {
        const float frame_begin = get_ticks_ms();

        // render
        oled_clear_buffer();
        vision_ui_render_loop();
        oled_send_buffer();

        // pace to 120 fps (include render time)
        float now_ms = get_ticks_ms();
        float elapsed_ms = now_ms - frame_begin; // full frame so far
        if (elapsed_ms < target_ms) {
            // sleep the remainder (rounded; replace delay() with a microsecond sleep if you have one)
            delay((uint32_t) lrintf(target_ms - elapsed_ms));
            now_ms = get_ticks_ms(); // re-sample after sleep
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
