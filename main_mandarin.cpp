#include <tgmath.h>
#include <u8g2.h>

#include <vision/vision_ui_core.h>
#include <vision/vision_ui_item.h>

#include "font/chinese.h"

u8g2_t U8G2;

uint32_t TIME_START = 0;
static int16_t y_logo = 200;
static int16_t y_version = 200;
static int16_t y_name = -200;
static int16_t y_astra = -200;
static int16_t y_box = 200;
static int16_t x_board = -200;
static int16_t y_wire_1 = 200;
static int16_t y_wire_2 = 200;

void animation(int16_t* pos, int16_t target, int16_t speed) {
    if (*pos != target) {
        if (fabs(*pos - target) <= 1.0f)
            *pos = target;
        else
            *pos += (target - *pos) / ((100 - speed) / 1.0f);
    }
}

void board_info_init(void) {
    TIME_START = get_ticks_ms();
}

void board_info_loop(void) {
    uint32_t t = get_ticks_ms();

    oled_set_draw_color(1);
    oled_draw_R_box(2, y_box - 1, oled_get_UTF8_width("「开发板启动器」") + 4, oled_get_str_height() + 2, 1);
    oled_set_draw_color(2);
    oled_draw_UTF8(4, y_logo - 2, "「开发板启动器」");

    oled_set_draw_color(1);
    oled_draw_str(106, y_version, "v1.0");
    oled_draw_UTF8(2, y_name, "由 无理造物 制作.");
    oled_draw_UTF8(2, y_astra, "基于「Astra UI Lite」v1.1");
    oled_draw_UTF8(2, y_astra + 14, "驱动引擎.");
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

    if (t - TIME_START > 300) animation(&y_logo, 15, 94);
    if (t - TIME_START > 350) animation(&y_version, 14, 88);
    if (t - TIME_START > 400) animation(&y_box, 2, 92);
    if (t - TIME_START > 450) animation(&y_astra, 36, 91);
    if (t - TIME_START > 500) animation(&y_name, 62, 94);
    if (t - TIME_START > 550) animation(&x_board, 102, 92);
    if (t - TIME_START > 620) animation(&y_wire_1, 56, 86);
    if (t - TIME_START > 1400 && t - TIME_START < 1600) oled_draw_box(x_board + 5, 42, 19, 6);
    if (t - TIME_START > 1800 && t - TIME_START < 1900) oled_draw_box(x_board + 5, 42, 19, 6);
    if (t - TIME_START > 2200) oled_draw_box(x_board + 5, 42, 19, 6);
    if (t - TIME_START > 2400) animation(&y_wire_2, 56, 86);
}

void board_info_exit(void) {
    TIME_START = 0;
    y_logo = 200;
    y_version = 200;
    y_name = -200;
    y_astra = -200;
    y_box = 200;
    x_board = -200;
    y_wire_1 = 200;
    y_wire_2 = 200;
}

int main(void) {
    u8g2_SetupBuffer_SDL_128x64(&U8G2, U8G2_R0);
    u8g2_InitDisplay(&U8G2);
    u8g2_SetPowerSave(&U8G2, 0);

    vision_ui_bind_driver(&U8G2);

    // 初始化 Astra UI
    astra_init_core();
    astra_set_font((void*) u8g2_font_my_chinese);

    // 主菜单
    astra_list_item_t* board_setting_item = astra_new_list_item("开发板设置");
    astra_push_item_to_list(astra_get_root_list(), board_setting_item);

    astra_push_item_to_list(astra_get_root_list(), astra_new_switch_item("切换屏幕", true, [](bool state) {
    }));

    astra_push_item_to_list(astra_get_root_list(),
                            astra_new_user_item("接线图...", board_info_init, board_info_loop, board_info_exit));

    astra_push_item_to_list(astra_get_root_list(),
                            astra_new_user_item("关于开发板...", board_info_init, board_info_loop, board_info_exit));

    astra_push_item_to_list(board_setting_item, astra_new_switch_item("心跳灯开关", true, [](bool state) {
    }));

    astra_push_item_to_list(board_setting_item, astra_new_switch_item("反转按键", false, [](bool state) {
    }));

    astra_push_item_to_list(board_setting_item,
                            astra_new_slider_item("数据显示样式", 1600, 5, 1, 9999, [](int16_t value) {
                            }));

    bool invert_display = false;
    astra_push_item_to_list(board_setting_item, astra_new_switch_item("反相显示", false, [](bool state) {
    }));

    bool mcu_serial = false;
    astra_push_item_to_list(board_setting_item, astra_new_switch_item("MCU 串口通道", true, [](bool state) {
    }));

    bool external_serial = false;
    astra_push_item_to_list(board_setting_item, astra_new_switch_item("外部串口通道", false, [](bool state) {
    }));

    // 渲染循环
    vision_ui_render_init();

    float prev_ms = get_ticks_ms();
    float current_ms = prev_ms;
    float fps_timer = prev_ms;
    int frame_count = 0;

    while (!vision_ui_is_exited()) {
        prev_ms = current_ms;
        current_ms = get_ticks_ms();

        oled_clear_buffer();
        vision_ui_render_loop();
        oled_send_buffer();

        frame_count++;
        if (current_ms - fps_timer >= 1000.0f) {
            const float fps = (float) frame_count * 1000.0f / (current_ms - fps_timer);
            printf("FPS: %.1f\n", fps);
            fps_timer = current_ms;
            frame_count = 0;
        }
    }

    return 0;
}
