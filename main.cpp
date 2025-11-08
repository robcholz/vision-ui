#include <u8g2.h>
#include <vision/astra_ui_core.h>
#include <vision/astra_ui_item.h>

u8g2_t u8g2;

int main() {
    // 1) 初始化 u8g2（SDL 128x64）
    u8g2_SetupBuffer_SDL_128x64(&u8g2, U8G2_R0);
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);

    vision_ui_bind_driver(&u8g2);

    bool test_bool = false; // 开关项的变量

    // Astra UI初始化
    astra_init_core(); // 初始化核心函数
    astra_list_item_t* setting_list_item = astra_new_list_item("列表"); // 定义列表项
    astra_push_item_to_list(astra_get_root_list(), setting_list_item); // 创建列表项
    astra_push_item_to_list(astra_get_root_list(),
                            astra_new_switch_item("开关", &test_bool, []() {
                                                  }, []() {
                                                  })); // 开关项
    while (true) {
        astra_ui_widget_core();
        astra_ui_main_core();
        /* ----------------------------------- 主循环 ---------------------------------- */
        oled_clear_buffer(); // 清空缓冲区
        ad_astra(); // 入口函数，主要用于将in_astra置true进入菜单，也可以用其他函数，只要能将in_astra置true即可，如in_astra = ture;
        astra_ui_widget_core(); // 弹窗处理函数
        astra_ui_main_core(); // 核心处理函数
        oled_send_buffer(); // 发送缓冲区
    }

    return 0;
}
