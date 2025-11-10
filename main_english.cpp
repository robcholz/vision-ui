#include <tgmath.h>
#include <u8g2.h>

#include <vision/astra_ui_core.h>
#include <vision/astra_ui_item.h>

#include "driver/u8g2.h"

#include "font/chinese.h"

u8g2_t u8g2;

uint32_t time_start = 0;
static int16_t y_logo = 200;
static int16_t y_version = 200;
static int16_t y_name = -200;
static int16_t y_astra = -200;
static int16_t y_box = 200;
static int16_t x_board = -200;
static int16_t y_wire_1 = 200;
static int16_t y_wire_2 = 200;

void animation(int16_t* _pos, int16_t _posTrg, int16_t _speed) {
    if (*_pos != _posTrg) {
        if (fabs(*_pos - _posTrg) <= 1.0f) *_pos = _posTrg;
        else *_pos += (_posTrg - *_pos) / ((100 - _speed) / 1.0f);
    }
}

void test_user_item_init_function() {
    time_start = get_ticks_ms();
}

void test_user_item_loop_function() {
    uint32_t _time = get_ticks_ms();

    oled_set_draw_color(1);
    oled_draw_R_box(2, y_box - 1, oled_get_UTF8_width("「astraLauncher」") + 4, oled_get_str_height() + 2, 1);
    oled_set_draw_color(2);
    oled_draw_UTF8(4, y_logo - 2, "「astraLauncher」");

    oled_set_draw_color(1);
    oled_draw_str(106, y_version, "v1.0");
    oled_draw_UTF8(2, y_name, "by Rational Works.");
    oled_draw_UTF8(2, y_astra, "Powered by Astra UI Lite v1.1");
    oled_draw_UTF8(2, y_astra + 14, "Engine.");
    oled_draw_frame(x_board, 38, 28, 20);
    oled_draw_frame(x_board + 2, 40, 24, 10);
    oled_draw_box(x_board + 2, 40, 2, 10);
    oled_draw_pixel(x_board + 25, 51);
    oled_draw_pixel(x_board + 25, 53);
    oled_draw_pixel(x_board + 25, 55);
    oled_draw_box(x_board + 21, 51, 3, 2);
    oled_draw_box(x_board + 21, 54, 3, 2);
    oled_draw_box(x_board + 17, 53, 3, 3);

    oled_draw_box(x_board + 12, 53, 4, 3);
    oled_draw_box(x_board + 7, 53, 4, 3);
    oled_draw_box(x_board + 2, 53, 4, 3);

    oled_draw_box(x_board + 7, y_wire_1, 4, 3);
    oled_draw_V_line(x_board + 9, y_wire_1 + 3, 3);
    oled_draw_V_line(x_board + 8, y_wire_1 + 6, 2);

    oled_draw_box(x_board + 12, y_wire_2, 4, 3);
    oled_draw_V_line(x_board + 14, y_wire_2 + 3, 3);
    oled_draw_V_line(x_board + 15, y_wire_2 + 6, 2);

    if (_time - time_start > 300) animation(&y_logo, 15, 94);
    if (_time - time_start > 350) animation(&y_version, 14, 88);
    if (_time - time_start > 400) animation(&y_box, 2, 92);
    if (_time - time_start > 450) animation(&y_astra, 36, 91);
    if (_time - time_start > 500) animation(&y_name, 62, 94);
    if (_time - time_start > 550) animation(&x_board, 102, 92);
    if (_time - time_start > 620) animation(&y_wire_1, 56, 86);
    if (_time - time_start > 1400 && _time - time_start < 1600) oled_draw_box(x_board + 5, 42, 19, 6);
    if (_time - time_start > 1800 && _time - time_start < 1900) oled_draw_box(x_board + 5, 42, 19, 6);
    if (_time - time_start > 2200) oled_draw_box(x_board + 5, 42, 19, 6);
    if (_time - time_start > 2400) animation(&y_wire_2, 56, 86);
}

void test_user_item_exit_function() {
    time_start = 0;
    y_logo = 200;
    y_version = 200;
    y_name = -200;
    y_astra = -200;
    y_box = 200;
    x_board = -200;
    y_wire_1 = 200;
    y_wire_2 = 200;
}

int main() {
    u8x8_Setup_SDL_128x64(u8g2_GetU8x8(&u8g2));
    u8g2_SetupBuffer(&u8g2, buffer, 8, u8g2_ll_hvline_vertical_top_lsb, U8G2_R0);
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);

    vision_ui_bind_driver(&u8g2);

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
