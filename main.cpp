#include <u8g2.h>

#include <vision/astra_ui_core.h>
#include <vision/astra_ui_item.h>

#include "font/chinese.h"

u8g2_t u8g2;

int main() {
    u8g2_SetupBuffer_SDL_128x64(&u8g2, U8G2_R0);
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);

    vision_ui_bind_driver(&u8g2);

    bool test_bool = false; // 开关项的变量

    // Astra UI初始化
    astra_init_core();

    astra_set_font((void*) u8g2_font_my_chinese);

    astra_list_item_t* launcher_setting_list_item = astra_new_list_item("开发板设置");

    astra_push_item_to_list(astra_get_root_list(), launcher_setting_list_item);
    bool screen_switch;
    astra_push_item_to_list(astra_get_root_list(), astra_new_switch_item("切换屏幕", &screen_switch));
    astra_push_item_to_list(astra_get_root_list(), astra_new_user_item("接线图...", []() {
                                                                       }, []() {
                                                                       }, []() {
                                                                       }));
    astra_push_item_to_list(astra_get_root_list(), astra_new_user_item("关于开发板...", []() {
                                                                       }, []() {
                                                                       }, []() {
                                                                       }));
    bool pulse_light;
    astra_push_item_to_list(launcher_setting_list_item, astra_new_switch_item("心跳灯开关", &pulse_light));
    //astra_push_item_to_list(launcher_setting_list_item, astra_new_switch_item("反转按键", &key_flip));
    //astra_push_item_to_list(launcher_setting_list_item, astra_new_slider_item("数据显示样式", &p_mode, 1, 1, 3));
    //astra_push_item_to_list(launcher_setting_list_item, astra_new_switch_item("反转显示", &dark_mode));
    //astra_push_item_to_list(launcher_setting_list_item, astra_new_switch_item("MCU串口通道", &mcu_serial_channel));
    //astra_push_item_to_list(launcher_setting_list_item, astra_new_switch_item("外部串口通道", &external_serial_channel));

    //launcher_set_terminal_area(4, 26, 124, 62);

    // launcher_push_str_to_terminal(info, "astraLauncher \nRev1.0 made by \nforpaindream.");

    //astra_selector_go_next_item();
    //astra_selector_go_prev_item();
    //astra_selector_exit_current_item();
    // astra_selector_jump_to_selected_item();

    vision_ui_render_init();

    while (true) {
        /* ----------------------------------- 主循环 ---------------------------------- */
        oled_clear_buffer(); // 清空缓冲区
        vision_ui_render_loop();
        oled_send_buffer(); // 发送缓冲区
    }

    return 0;
}
