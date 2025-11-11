#include <tgmath.h>
#include <u8g2.h>

#include <vision/astra_ui_core.h>
#include <vision/astra_ui_item.h>

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

void animation(int16_t *_pos, int16_t _posTrg, int16_t _speed)
{
    if (*_pos != _posTrg)
    {
        if (fabs(*_pos - _posTrg) <= 1.0f) *_pos = _posTrg;
        else *_pos += (_posTrg - *_pos) / ((100 - _speed) / 1.0f);
    }
}

void test_user_item_init_function()
{
  time_start = get_ticks_ms();
}

void test_user_item_loop_function()
{
  uint32_t _time = get_ticks_ms();

  oled_set_draw_color(1);
  oled_draw_R_box(2, y_box - 1, oled_get_UTF8_width("「astraLauncher」") + 4, oled_get_str_height() + 2, 1);
  oled_set_draw_color(2);
  oled_draw_UTF8(4, y_logo - 2, "「astraLauncher」");

  oled_set_draw_color(1);
  oled_draw_str(106, y_version, "v1.0");
  oled_draw_UTF8(2, y_name, "by 无理造物.");
  oled_draw_UTF8(2, y_astra, "由「astra UI Lite」v1.1");
  oled_draw_UTF8(2, y_astra + 14, "驱动.");
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

void test_user_item_exit_function()
{
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
    u8g2_SetupBuffer_SDL_128x64(&u8g2, U8G2_R0);
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);

    vision_ui_bind_driver(&u8g2);

    // Astra UI初始化
    astra_init_core();

    astra_set_font((void*) u8g2_font_my_chinese);

    astra_list_item_t* launcher_setting_list_item = astra_new_list_item("开发板设置");

    astra_push_item_to_list(astra_get_root_list(), launcher_setting_list_item);
    bool screen_switch;
    astra_push_item_to_list(astra_get_root_list(), astra_new_switch_item("切换屏幕", &screen_switch));
    astra_push_item_to_list(astra_get_root_list(), astra_new_user_item("接线图...", test_user_item_init_function, test_user_item_loop_function, test_user_item_exit_function));
    astra_push_item_to_list(astra_get_root_list(), astra_new_user_item("关于开发板...", test_user_item_init_function, test_user_item_loop_function, test_user_item_exit_function));

    bool pulse_light=true;
    astra_push_item_to_list(launcher_setting_list_item, astra_new_switch_item("心跳灯开关心跳灯开关", &pulse_light));
    bool key_flip;
    astra_push_item_to_list(launcher_setting_list_item, astra_new_switch_item("反转按键", &key_flip));
    int16_t p_mode=1000;
    astra_push_item_to_list(launcher_setting_list_item, astra_new_slider_item("数据显示样式", &p_mode, 5, 1, 9999));
    bool dark_mode;
    astra_push_item_to_list(launcher_setting_list_item, astra_new_switch_item("Invert Display", &dark_mode));
    bool mcu_serial_channel;
    astra_push_item_to_list(launcher_setting_list_item, astra_new_switch_item("MCU串口通道", &mcu_serial_channel));
    bool external_serial_channel;
    astra_push_item_to_list(launcher_setting_list_item, astra_new_switch_item("外部串口通道", &external_serial_channel));

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
            const float fps = static_cast<float>(frame_count) * 1000.0f / (current_ms - fps_timer);
            printf("FPS: %.1f\n", fps);
            fps_timer = current_ms;
            frame_count = 0;
        }
    }

    return 0;
}
