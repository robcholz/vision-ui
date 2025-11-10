//
// Created by forpaindream on 25-1-20.
//

#include "astra_ui_drawer.h"

#include <math.h>
#include <stdio.h>

#include "astra_ui_core.h"

void astra_exit_animation(float* _pos, float _posTrg, float _speed) {
    if (*_pos != _posTrg) {
        if (fabs(*_pos - _posTrg) <= 1.0f) *_pos = _posTrg;
        else *_pos += (_posTrg - *_pos) / (100.0f - _speed) / 1.0f;
    }
}

uint8_t astra_exit_animation_status = 0;

void astra_draw_exit_animation() {
    //完成完整的退场动画 astra_exit_animation_status的取值依次如下
    //0 触发退场动画 遮罩开始落下
    //1 遮罩落下完成 此时屏幕被遮罩填满 开始变更背景内容
    //2 遮罩开始抬升
    //0 遮罩抬升完成 退场动画完成
    static float _temp_h = -8;
    static float _temp_h_trg = SCREEN_HEIGHT + 8;

    oled_set_draw_color(0);
    oled_draw_box(0, 0, SCREEN_WIDTH, _temp_h); //遮罩
    oled_set_draw_color(1);

    // 沙漏
    uint8_t _x_hourglass_offset = SCREEN_WIDTH / 2 - 8;
    int8_t _y_hourglass = _temp_h - SCREEN_HEIGHT / 2 - 18;
    if (_y_hourglass + 20 >= 0) {
        // 绘制顶部和底部矩形及中间擦除
        oled_draw_box(_x_hourglass_offset, _y_hourglass + 2, 13, 3);
        oled_set_draw_color(0);
        oled_draw_H_line(_x_hourglass_offset + 2, _y_hourglass + 3, 9);
        oled_set_draw_color(1);

        // 主体结构
        oled_draw_V_line(_x_hourglass_offset + 1, _y_hourglass + 4, 5);
        oled_draw_V_line(_x_hourglass_offset + 11, _y_hourglass + 4, 5);

        // 斜线部分循环绘制
        for (uint8_t i = 0; i < 5; ++i) {
            int8_t _current_y = _y_hourglass + 8 + i;
            int8_t _left_x = (i < 3) ? (_x_hourglass_offset + 1 + i) : (_x_hourglass_offset + 4);
            int8_t _right_x = (i < 3) ? (_x_hourglass_offset + 10 - i) : (_x_hourglass_offset + 7);
            oled_draw_H_line(_left_x, _current_y, 2);
            oled_draw_H_line(_right_x, _current_y, 2);
        }

        // 中间收口部分
        for (uint8_t i = 0; i < 3; ++i) {
            int8_t _current_y = _y_hourglass + 13 + i;
            oled_draw_H_line(_x_hourglass_offset + 3 - i, _current_y, 2);
            oled_draw_H_line(_x_hourglass_offset + 8 + i, _current_y, 2);
        }

        // 底部竖线
        oled_draw_V_line(_x_hourglass_offset + 1, _y_hourglass + 16, 3);
        oled_draw_V_line(_x_hourglass_offset + 11, _y_hourglass + 16, 3);

        // 底部矩形
        oled_draw_box(_x_hourglass_offset, _y_hourglass + 19, 13, 3);
        oled_set_draw_color(0);
        oled_draw_H_line(_x_hourglass_offset + 2, _y_hourglass + 20, 9);
        oled_set_draw_color(1);

        // 散点像素数组化绘制
        const uint8_t _points[][2] = {
            {5, 7}, {7, 7}, {6, 8}, {6, 10}, {6, 14}, {6, 16},
            {5, 17}, {7, 17}, {4, 18}, {6, 18}, {8, 18}
        };
        for (uint8_t i = 0; i < sizeof(_points) / sizeof(_points[0]); ++i)
            oled_draw_pixel(_x_hourglass_offset + _points[i][0], _y_hourglass + _points[i][1]);
    }

    if (_temp_h + 3 >= 0)
        //下面是遮罩下方横线
        for (uint8_t i = 0; i <= 3; ++i)
            oled_draw_H_line(0, _temp_h + i, SCREEN_WIDTH);

    //棋盘格过渡
    for (int16_t i = 0; i <= SCREEN_WIDTH; i += 2)
        for (int16_t j = _temp_h - 5; j <= _temp_h - 1; j++) {
            if (j % 2 == 0)
                oled_draw_pixel(i + 1, j);
            if (j % 2 == 1)
                oled_draw_pixel(i, j);
        }

    astra_exit_animation(&_temp_h, _temp_h_trg, 94);

    //下落过程
    if (astra_exit_animation_status == 0 && _temp_h == _temp_h_trg && _temp_h == SCREEN_HEIGHT + 8) {
        astra_exit_animation_status = 1; //落下来了
        return;
    }

    //上面astra_exit_animation_status=1之后 return了 进到core里刷新了背景显示内容 下一次进到本函数就可以把标志位置为2
    if (astra_exit_animation_status == 1) {
        // _temp_h_trg = OLED_HEIGHT + 8;
        _temp_h_trg = -8; //使其开始上升
        astra_exit_animation_status = 2; //开始抬起
        return;
    }

    if (astra_exit_animation_status == 2 && _temp_h == _temp_h_trg && _temp_h == -8) {
        astra_exit_animation_finished = true;
        astra_exit_animation_status = 0; //退场动画完成
        _temp_h = -8;
        _temp_h_trg = SCREEN_HEIGHT + 8;
        return;
    }
}

void astra_draw_info_bar() {
    if (!astra_info_bar.is_running) return;

    //弹窗到位后才开始计算时间
    if (astra_info_bar.y_info_bar == astra_info_bar.y_info_bar_trg) astra_info_bar.time = get_ticks_ms();

    //时间到了就收回
    if (astra_info_bar.time - astra_info_bar.time_start >= astra_info_bar.span) {
        astra_info_bar.y_info_bar_trg = 0 - 2 * INFO_BAR_HEIGHT; //收回
        if (astra_info_bar.y_info_bar == astra_info_bar.y_info_bar_trg) astra_info_bar.is_running = false; //等归位后结束生命周期
    }

    int16_t _x_info_bar = SCREEN_WIDTH / 2 - astra_info_bar.w_info_bar / 2;
    int16_t _y_info_bar_1 = astra_info_bar.y_info_bar - 4;
    int16_t _y_info_bar_2 = astra_info_bar.y_info_bar + INFO_BAR_HEIGHT;

    astra_set_font(astra_font);
    oled_set_draw_color(1);
    oled_draw_R_box(_x_info_bar + 3, _y_info_bar_1 + 3,
                    (int16_t) astra_info_bar.w_info_bar, INFO_BAR_HEIGHT + 4, 4);

    oled_set_draw_color(0); //黑遮罩打底
    oled_draw_R_box((int16_t) (SCREEN_WIDTH / 2 - (astra_info_bar.w_info_bar + 4) / 2), _y_info_bar_1,
                    (int16_t) (astra_info_bar.w_info_bar + 4), INFO_BAR_HEIGHT + 6, 4);

    oled_set_draw_color(1);
    oled_draw_R_box(_x_info_bar, _y_info_bar_1,
                    (int16_t) astra_info_bar.w_info_bar, INFO_BAR_HEIGHT + 4, 3);
    //向上移动四个像素 同时向下多画四个像素 只用下半部分圆角

    oled_set_draw_color(2);
    oled_draw_H_line(_x_info_bar + 2, _y_info_bar_2 - 2, (int16_t) (astra_info_bar.w_info_bar - 4));
    oled_draw_pixel(_x_info_bar + 1, _y_info_bar_2 - 3);
    oled_draw_pixel(_x_info_bar - 2, _y_info_bar_2 - 3);

    oled_draw_UTF8(_x_info_bar + 6,
                   (int16_t) (astra_info_bar.y_info_bar + oled_get_str_height() - 2),
                   astra_info_bar.content);
}

void astra_draw_pop_up() {
    if (!astra_pop_up.is_running) return;

    //弹窗到位后才开始计算时间
    if (astra_pop_up.y_pop_up == astra_pop_up.y_pop_up_trg) astra_pop_up.time = get_ticks_ms();

    //时间到了就收回
    if (astra_pop_up.time - astra_pop_up.time_start >= astra_pop_up.span) {
        astra_pop_up.y_pop_up_trg = 0 - 2 * INFO_BAR_HEIGHT; //收回
        if (astra_pop_up.y_pop_up == astra_pop_up.y_pop_up_trg) astra_pop_up.is_running = false; //等归位后结束生命周期
    }

    int16_t _x_pop_up = SCREEN_WIDTH / 2 - astra_pop_up.w_pop_up / 2;
    int16_t _y_pop_up = astra_pop_up.y_pop_up + POP_UP_HEIGHT;

    astra_set_font(astra_font);
    oled_set_draw_color(1); //阴影打底
    oled_draw_R_box(_x_pop_up + 1, (int16_t) astra_pop_up.y_pop_up + 3,
                    (int16_t) (astra_pop_up.w_pop_up + 4),
                    POP_UP_HEIGHT, 4);

    oled_set_draw_color(0); //黑遮罩
    oled_draw_R_box((int16_t) (SCREEN_WIDTH / 2 - (astra_pop_up.w_pop_up + 4) / 2 - 2), (int16_t) (astra_pop_up.y_pop_up - 2),
                    (int16_t) (astra_pop_up.w_pop_up + 8), POP_UP_HEIGHT + 4, 5);

    oled_set_draw_color(1);
    oled_draw_R_box(_x_pop_up - 2, (int16_t) astra_pop_up.y_pop_up,
                    (int16_t) (astra_pop_up.w_pop_up + 4),
                    POP_UP_HEIGHT, 3);

    oled_set_draw_color(2);
    oled_draw_H_line(_x_pop_up, _y_pop_up - 2, (int16_t) astra_pop_up.w_pop_up);
    oled_draw_pixel(_x_pop_up - 1, _y_pop_up - 3);
    oled_draw_pixel((int16_t) (SCREEN_WIDTH / 2 + astra_pop_up.w_pop_up / 2), _y_pop_up - 3);

    oled_draw_UTF8(_x_pop_up + 3,
                   (int16_t) (astra_pop_up.y_pop_up + oled_get_str_height() + 1),
                   astra_pop_up.content);
}

void astra_draw_list_appearance() {
    oled_set_draw_color(1);

    oled_draw_V_line(SCREEN_WIDTH - 2, 0, SCREEN_HEIGHT);

    const astra_list_item_t* parent = astra_selector.selected_item->parent;

    const int16_t slider_top_px = parent ? parent->scroll_bar_top_px : 0;
    const int16_t slider_h_px = parent ? parent->scroll_bar_height_px : SCREEN_HEIGHT;

    oled_draw_box(SCREEN_WIDTH - LIST_SCROLL_BAR_WIDTH,
                  slider_top_px,
                  LIST_SCROLL_BAR_WIDTH,
                  slider_h_px);

    const uint8_t child_cnt = parent ? parent->child_num : 1;
    const float part = child_cnt > 0 ? (float) SCREEN_HEIGHT / child_cnt : SCREEN_HEIGHT;

    const int16_t track_x = SCREEN_WIDTH - LIST_SCROLL_BAR_WIDTH;
    for (uint8_t i = 0; i <= child_cnt; ++i) {
        const int16_t y = lrintf(part * i);
        if (y >= 0 && y < SCREEN_HEIGHT) {
            if (i % 2 == 0) {
                oled_draw_H_line(track_x, y, LIST_SCROLL_BAR_WIDTH);
            } else {
                oled_draw_H_line(track_x, y, LIST_SCROLL_BAR_WIDTH - 1);
            }
        }
    }
}

static void vision_ui_draw_text(const char* text,
                                uint32_t* text_scroll_anchor,
                                const int16_t x0, const int16_t y0,
                                const int16_t x1, const int16_t y1,
                                const uint16_t scroll_speed_s,
                                const uint16_t scroll_pause_ms) {
    // 当前字体行高
    const int16_t line_h = oled_get_str_height();
    const int16_t text_x = x0;
    const int16_t text_y = y0 + (y1 - y0) / 2 + line_h / 2; // 垂直居中到rect内
    const int16_t visible_width = x1 - x0;

    // 文本总宽度
    const int16_t text_width = oled_get_UTF8_width(text);

    const int16_t clip_x0 = x0 < 0 ? 0 : x0;
    const int16_t clip_y0 = y0 < 0 ? 0 : y0;
    const int16_t clip_x1 = x1 > SCREEN_WIDTH ? SCREEN_WIDTH : x1;
    const int16_t clip_y1 = y1 > SCREEN_HEIGHT ? SCREEN_HEIGHT : y1;
    const bool has_visible_area = clip_x0 < clip_x1 && clip_y0 < clip_y1;

    // 如果文本太长，需要滚动
    if (text_width > visible_width && visible_width > 0) {
        int16_t scroll_offset = 0;
        const int16_t overflow = text_width - visible_width;
        const float speed_px_s = scroll_speed_s; // px/s
        const uint32_t forward_ms = (uint32_t) (1000.f * overflow / speed_px_s + 0.5f);

        const uint32_t now = get_ticks_ms();
        if (*text_scroll_anchor == 0)
            *text_scroll_anchor = now;

        const uint32_t elapsed = now - *text_scroll_anchor;

        if (elapsed > scroll_pause_ms && forward_ms > 0) {
            // 生成往返三角波（0..overflow..0）
            const uint32_t t = (elapsed - scroll_pause_ms) % (forward_ms * 2u);
            const float p = (float) t / (float) forward_ms; // 0..2
            const float tri = (p <= 1.f) ? p : (2.f - p); // 0..1..0
            scroll_offset = (int16_t) lrintf((float) overflow * tri);
        } else {
            scroll_offset = 0;
        }

        if (has_visible_area) {
            oled_set_clip_window(clip_x0, clip_y0, clip_x1, clip_y1);
            oled_draw_UTF8(text_x - scroll_offset, text_y, text);
            oled_reset_clip_window();
        }
    } else {
        // 不滚动，直接居中画
        *text_scroll_anchor = 0;
        if (has_visible_area) {
            oled_set_clip_window(clip_x0, clip_y0, clip_x1, clip_y1);
            oled_draw_UTF8(text_x, text_y, text);
            oled_reset_clip_window();
        }
    }
}

static void vision_ui_draw_list_item_text(astra_list_item_t* list,
                                          const int16_t x0, const int16_t y0,
                                          const int16_t x1, const int16_t y1) {
    vision_ui_draw_text(list->content, &list->text_scroll_anchor, x0, y0, x1, y1,LIST_TEXT_SCROLL_SPEED_PX_S,LIST_TEXT_SCROLL_PAUSE_MS);
}

static void vision_ui_draw_list_header() {
    static const uint8_t header_list_item[LIST_HEADER_MAX_HEIGHT] = {
        0b0000000,
        0b0011110,
        0b0000000,
        0b0111110,
        0b0000000,
        0b0001110,
        0b0000000,
    };

    static const uint8_t header_switch_item[LIST_HEADER_MAX_HEIGHT] = {
        0b0011100,
        0b0100010,
        0b1001001,
        0b1001001,
        0b1001001,
        0b0100010,
        0b0011100,
    };

    static const uint8_t header_slider_item[LIST_HEADER_MAX_HEIGHT] = {
        0b0001110,
        0b0101110,
        0b0101110,
        0b0100100,
        0b1110100,
        0b1110100,
        0b1110000,
    };

    static const uint8_t header_other_item[LIST_HEADER_MAX_HEIGHT] = {
        0b0000000,
        0b0000000,
        0b0000000,
        0b0011100,
        0b0000000,
        0b0000000,
        0b0000000,
    };

    for (uint8_t i = 0; i < astra_selector.selected_item->parent->child_num; i++) {
        astra_list_item_t* current_list_item = astra_selector.selected_item->parent->child_list_item[i];
        int16_t x_list_item = astra_camera.x_camera + LIST_HEADER_TO_LEFT_DISPLAY_PADDING;
        int16_t y_list_item = current_list_item->y_list_item + astra_camera.y_camera + (
                                  LIST_FRAME_FIXED_HEIGHT - LIST_HEADER_MAX_HEIGHT) / 2;
        // draw header
        oled_set_draw_color(1);
        const uint16_t header_base_x = x_list_item;
        if (current_list_item->type == list_item) {
            oled_draw_bMP(header_base_x, y_list_item, LIST_HEADER_MAX_WIDTH, LIST_HEADER_MAX_HEIGHT, header_list_item);
        } else if (current_list_item->type == switch_item) {
            oled_draw_bMP(header_base_x, y_list_item, LIST_HEADER_MAX_WIDTH, LIST_HEADER_MAX_HEIGHT, header_switch_item);
        } else if (current_list_item->type == slider_item) {
            oled_draw_bMP(header_base_x, y_list_item, LIST_HEADER_MAX_WIDTH, LIST_HEADER_MAX_HEIGHT, header_slider_item);
        } else if (current_list_item->type == title_item) {
            // do nothing
        } else {
            oled_draw_bMP(header_base_x, y_list_item, LIST_HEADER_MAX_WIDTH, LIST_HEADER_MAX_HEIGHT, header_other_item);
        }
    }
}

static void vision_ui_draw_list_footer() {
    static const uint8_t footer_switch_off[LIST_FOOTER_MAX_HEIGHT][3] = {
        {0b00000000, 0b00000000, 0b00000000},
        {0b11100000, 0b00111111, 0b00000000},
        {0b00100000, 0b00100000, 0b00000000},
        {0b00100000, 0b00100000, 0b00000000},
        {0b00100000, 0b00100000, 0b00000000},
        {0b00100000, 0b00100000, 0b00000000},
        {0b00100000, 0b00100000, 0b00000000},
        {0b00100000, 0b00100000, 0b00000000},
        {0b00100000, 0b00100000, 0b00000000},
        {0b11100000, 0b00111111, 0b00000000},
        {0b00000000, 0b00000000, 0b00000000},
    };

    static const uint8_t footer_switch_on[LIST_FOOTER_MAX_HEIGHT][3] = {
        {0b00000000, 0b00000000, 0b00000000},
        {0b11100000, 0b00111111, 0b00000000},
        {0b00100000, 0b00100000, 0b00000000},
        {0b10100000, 0b00101111, 0b00000000},
        {0b10100000, 0b00101111, 0b00000000},
        {0b10100000, 0b00101111, 0b00000000},
        {0b10100000, 0b00101111, 0b00000000},
        {0b10100000, 0b00101111, 0b00000000},
        {0b00100000, 0b00100000, 0b00000000},
        {0b11100000, 0b00111111, 0b00000000},
        {0b00000000, 0b00000000, 0b00000000},
    };

    static const uint8_t footer_slider[LIST_FOOTER_MAX_HEIGHT][3] = {
        {0b00000000, 0b00000000, 0b00000000},
        {0b11111110, 0b11111111, 0b00000011},
        {0b11111111, 0b11111111, 0b00000111},
        {0b11111111, 0b11111111, 0b00000111},
        {0b11111111, 0b11111111, 0b00000111},
        {0b11111111, 0b11111111, 0b00000111},
        {0b11111111, 0b11111111, 0b00000111},
        {0b11111111, 0b11111111, 0b00000111},
        {0b11111111, 0b11111111, 0b00000111},
        {0b11111111, 0b11111111, 0b00000111},
        {0b11111110, 0b11111111, 0b00000011},
    };

    for (uint8_t i = 0; i < astra_selector.selected_item->parent->child_num; i++) {
        astra_list_item_t* current_list_item = astra_selector.selected_item->parent->child_list_item[i];
        int16_t y_list_item = current_list_item->y_list_item + astra_camera.y_camera;

        // draw header
        const int16_t frame_x = SCREEN_WIDTH - LIST_SCROLL_BAR_WIDTH - LIST_FOOTER_TO_SCROLL_BAR_PADDING - LIST_FOOTER_MAX_WIDTH;
        const int16_t frame_y = y_list_item + (LIST_FRAME_FIXED_HEIGHT - LIST_FOOTER_MAX_HEIGHT) / 2;
        if (current_list_item->type == list_item) {
        } else if (current_list_item->type == switch_item) {
            if (*astra_to_switch_item(current_list_item)->value == true) {
                oled_set_draw_color(1);
                oled_draw_bMP(frame_x, frame_y, LIST_FOOTER_MAX_WIDTH, LIST_FOOTER_MAX_HEIGHT, footer_switch_on);
            } else {
                oled_set_draw_color(1);
                oled_draw_bMP(frame_x, frame_y, LIST_FOOTER_MAX_WIDTH, LIST_FOOTER_MAX_HEIGHT, footer_switch_off);
            }
        } else if (current_list_item->type == slider_item) {
            const uint16_t SHRINK_WIDTH = LIST_FOOTER_MAX_WIDTH - 4;
            const int16_t footer_x0 = frame_x + (LIST_FOOTER_MAX_WIDTH - SHRINK_WIDTH) / 2;
            const int16_t footer_y0 = frame_y;
            const int16_t footer_x1 = footer_x0 + SHRINK_WIDTH;
            const int16_t footer_y1 = footer_y0 + LIST_FOOTER_MAX_HEIGHT;

            char value_str[10] = {};
            const int16_t value = *astra_to_slider_item(current_list_item)->value;
            sprintf(value_str, "%d", value);

            oled_set_draw_color(1);
            vision_ui_draw_text(value_str,
                                &astra_to_slider_item(current_list_item)->text_scroll_anchor,
                                footer_x0,
                                footer_y0,
                                footer_x1,
                                footer_y1,
                                LIST_SLIDER_VALUE_SCROLL_SPEED_PX_S,
                                LIST_SLIDER_VALUE_SCROLL_PAUSE_MS);
            if (astra_to_slider_item(current_list_item)->is_confirmed) {
                oled_set_draw_color(2);
                oled_draw_bMP(frame_x, frame_y, LIST_FOOTER_MAX_WIDTH, LIST_FOOTER_MAX_HEIGHT, footer_slider);
            }
        }
    }
}

void astra_draw_list_item() {
    vision_ui_draw_list_header();
    vision_ui_draw_list_footer();

    for (uint8_t i = 0; i < astra_selector.selected_item->parent->child_num; i++) {
        astra_list_item_t* current_list_item = astra_selector.selected_item->parent->child_list_item[i];
        const int16_t x_list_item = astra_camera.x_camera + LIST_HEADER_TO_LEFT_DISPLAY_PADDING;
        const int16_t y_list_item = current_list_item->y_list_item + astra_camera.y_camera;

        const int16_t frame_x = current_list_item->type == title_item
                                    ? x_list_item
                                    : x_list_item + LIST_HEADER_MAX_WIDTH + LIST_HEADER_TO_TEXT_PADDING;

        const int16_t frame_y = y_list_item;

        astra_set_font(astra_font);
        oled_set_draw_color(1);
        vision_ui_draw_list_item_text(current_list_item,
                                      frame_x,
                                      frame_y,
                                      frame_x + LIST_TEXT_MAX_WIDTH,
                                      y_list_item + LIST_FRAME_FIXED_HEIGHT);
    }
}

void astra_draw_selector() {
    const int16_t x_selector = (int16_t) (lrintf(astra_camera.x_camera) + LIST_HEADER_TO_LEFT_DISPLAY_PADDING -
                                          LIST_SELECTOR_TO_INNER_WIDGET_PADDING);
    const int16_t y_selector = (int16_t) lrintf(astra_selector.y_selector + astra_camera.y_camera);

    oled_set_draw_color(1);
    oled_draw_R_frame(x_selector, y_selector, astra_selector.w_selector_trg, astra_selector.h_selector_trg, 3);
    oled_set_draw_color(2);
    oled_draw_box(x_selector + 1,
                  y_selector + 2,
                  astra_selector.w_selector_trg - 2,
                  astra_selector.h_selector_trg - 4);
    oled_draw_H_line(x_selector + 2, y_selector + 1, astra_selector.w_selector_trg - 4);
    oled_draw_H_line(x_selector + 2, y_selector + astra_selector.h_selector_trg - 2, astra_selector.w_selector_trg - 4);
}

void astra_draw_widget() {
    //需要调用所有的控件draw函数 需要在core后面执行 否则会被core覆盖
    astra_draw_info_bar();
    astra_draw_pop_up();
}

void astra_draw_list() {
    //调用所有的列表相关draw函数
    astra_draw_list_appearance();
    astra_draw_list_item();
    astra_draw_selector();
}
