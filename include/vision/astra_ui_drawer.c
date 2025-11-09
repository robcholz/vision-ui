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
    static float _temp_h_trg = OLED_HEIGHT + 8;

    oled_set_draw_color(0);
    oled_draw_box(0, 0, OLED_WIDTH, _temp_h); //遮罩
    oled_set_draw_color(1);

    // 沙漏
    uint8_t _x_hourglass_offset = OLED_WIDTH / 2 - 8;
    int8_t _y_hourglass = _temp_h - OLED_HEIGHT / 2 - 18;
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
            oled_draw_H_line(0, _temp_h + i, OLED_WIDTH);

    //棋盘格过渡
    for (int16_t i = 0; i <= OLED_WIDTH; i += 2)
        for (int16_t j = _temp_h - 5; j <= _temp_h - 1; j++) {
            if (j % 2 == 0)
                oled_draw_pixel(i + 1, j);
            if (j % 2 == 1)
                oled_draw_pixel(i, j);
        }

    astra_exit_animation(&_temp_h, _temp_h_trg, 94);

    //下落过程
    if (astra_exit_animation_status == 0 && _temp_h == _temp_h_trg && _temp_h == OLED_HEIGHT + 8) {
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
        _temp_h_trg = OLED_HEIGHT + 8;
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

    int16_t _x_info_bar = OLED_WIDTH / 2 - astra_info_bar.w_info_bar / 2;
    int16_t _y_info_bar_1 = astra_info_bar.y_info_bar - 4;
    int16_t _y_info_bar_2 = astra_info_bar.y_info_bar + INFO_BAR_HEIGHT;

    astra_set_font(astra_font);
    oled_set_draw_color(1);
    oled_draw_R_box(_x_info_bar + 3, _y_info_bar_1 + 3,
                    (int16_t) astra_info_bar.w_info_bar, INFO_BAR_HEIGHT + 4, 4);

    oled_set_draw_color(0); //黑遮罩打底
    oled_draw_R_box((int16_t) (OLED_WIDTH / 2 - (astra_info_bar.w_info_bar + 4) / 2), _y_info_bar_1,
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

    int16_t _x_pop_up = OLED_WIDTH / 2 - astra_pop_up.w_pop_up / 2;
    int16_t _y_pop_up = astra_pop_up.y_pop_up + POP_UP_HEIGHT;

    astra_set_font(astra_font);
    oled_set_draw_color(1); //阴影打底
    oled_draw_R_box(_x_pop_up + 1, (int16_t) astra_pop_up.y_pop_up + 3,
                    (int16_t) (astra_pop_up.w_pop_up + 4),
                    POP_UP_HEIGHT, 4);

    oled_set_draw_color(0); //黑遮罩
    oled_draw_R_box((int16_t) (OLED_WIDTH / 2 - (astra_pop_up.w_pop_up + 4) / 2 - 2), (int16_t) (astra_pop_up.y_pop_up - 2),
                    (int16_t) (astra_pop_up.w_pop_up + 8), POP_UP_HEIGHT + 4, 5);

    oled_set_draw_color(1);
    oled_draw_R_box(_x_pop_up - 2, (int16_t) astra_pop_up.y_pop_up,
                    (int16_t) (astra_pop_up.w_pop_up + 4),
                    POP_UP_HEIGHT, 3);

    oled_set_draw_color(2);
    oled_draw_H_line(_x_pop_up, _y_pop_up - 2, (int16_t) astra_pop_up.w_pop_up);
    oled_draw_pixel(_x_pop_up - 1, _y_pop_up - 3);
    oled_draw_pixel((int16_t) (OLED_WIDTH / 2 + astra_pop_up.w_pop_up / 2), _y_pop_up - 3);

    oled_draw_UTF8(_x_pop_up + 3,
                   (int16_t) (astra_pop_up.y_pop_up + oled_get_str_height() + 1),
                   astra_pop_up.content);
}

void astra_draw_list_appearance() {
    oled_set_draw_color(1);
    //顶部状态栏
    oled_draw_H_line(0, 1, 66);
    oled_draw_H_line(0, 0, 67);
    // 参数化绘制配置
    const struct {
        uint8_t _start;
        uint8_t _end;
        uint8_t _step;
        uint8_t _y;
    } draw_cfg[] = {
                {67, 99, 2, 1}, // 奇数序列优化为步长2
                {68, 100, 2, 0}, // 偶数序列优化为步长2
                {102, 111, 3, 1}, // 原i%3==0等效步长3（数学变换后起始点+1）
                {103, 112, 3, 0}, // 原i%3==1等效步长3（数学变换后起始点+2）
                {115, 124, 5, 1}, // 原i%5==0等效步长5（数学变换后起始点+3）
                {116, 124, 5, 0} // 原i%5==1等效步长5（数学变换后起始点+2）
            };

    for (uint8_t j = 0; j < sizeof(draw_cfg) / sizeof(draw_cfg[0]); ++j)
        for (uint8_t i = draw_cfg[j]._start; i <= draw_cfg[j]._end; i += draw_cfg[j]._step)
            oled_draw_pixel(i, draw_cfg[j]._y);

    //右侧进度条
    oled_draw_V_line(OLED_WIDTH - 5, 0, OLED_HEIGHT);
    oled_draw_V_line(OLED_WIDTH - 1, 0, OLED_HEIGHT);

    //滑块
    static float _length_each_part = 0;
    _length_each_part = ceilf((SCREEN_HEIGHT - 10.0f) / (float) astra_selector.selected_item->parent->child_num);
    oled_draw_box(OLED_WIDTH - 4, 5 + astra_selector.selected_index * _length_each_part, 3, _length_each_part);

    //滑块内横线
    oled_set_draw_color(0);
    oled_draw_H_line(OLED_WIDTH - 4, _length_each_part + (float) astra_selector.selected_index * _length_each_part,
                     3); //中间横线

    //长度允许的情况下绘制上下横线
    if (_length_each_part >= 9) {
        oled_draw_H_line(OLED_WIDTH - 4,
                         floorf(_length_each_part - 2.0f + (float) astra_selector.selected_index * _length_each_part), 3);
        oled_draw_H_line(OLED_WIDTH - 4,
                         floorf(_length_each_part + 2.0f + (float) astra_selector.selected_index * _length_each_part), 3);
    }

    oled_set_draw_color(1);
    oled_draw_box(OLED_WIDTH - 4, 0, 3, 4);
    oled_draw_box(OLED_WIDTH - 4, OLED_HEIGHT - 4, 3, 4);
    oled_set_draw_color(0);
    oled_draw_H_line(OLED_WIDTH - 4, 2, 3);
    oled_draw_pixel(OLED_WIDTH - 3, 1);
    oled_draw_H_line(OLED_WIDTH - 4, OLED_HEIGHT - 3, 3);
    oled_draw_pixel(OLED_WIDTH - 3, OLED_HEIGHT - 2);
}

//todo 视野外的部分将不会被渲染 但是现在坐标值小于屏幕范围的左值待定 并未缜密测试
void astra_draw_list_item() {
    //selector内包含的item的parent即是当前正在被绘制的页面
    for (unsigned char i = 0; i < astra_selector.selected_item->parent->child_num; i++) {
        astra_list_item_t* _list_item = astra_selector.selected_item->parent->child_list_item[i];
        int16_t _x_list_item = astra_camera.x_camera + LIST_ITEM_LEFT_MARGIN;
        int16_t _y_list_item = _list_item->y_list_item + astra_camera.y_camera - oled_get_str_height() / 2;

        oled_set_draw_color(1);
        //绘制开头的指示器
        if (_list_item->type == list_item) {
            if (_y_list_item + 2 > LIST_INFO_BAR_HEIGHT && _y_list_item - 2 < SCREEN_HEIGHT) {
                oled_draw_H_line(2 + _x_list_item, _y_list_item - 2, 4);
                oled_draw_H_line(2 + _x_list_item, _y_list_item, 5);
                oled_draw_H_line(2 + _x_list_item, _y_list_item + 2, 3);
            }
        } else if (_list_item->type == switch_item) {
            if (_y_list_item + 7 > LIST_INFO_BAR_HEIGHT && _y_list_item + 1 < SCREEN_HEIGHT) {
                oled_draw_circle(4 + _x_list_item, _y_list_item + 1, 3);
                oled_draw_V_line(4 + _x_list_item, _y_list_item, 3);

                //开关控件指示器部分
                static const uint8_t FRAME_WIDTH = 1;
                const uint16_t box_padding = FRAME_WIDTH + (LIST_ITEM_SPACING - LIST_ITEM_SWITCH_BOX_WIDTH) / 2;
                const uint16_t frame_x = OLED_WIDTH - LIST_ITEM_RIGHT_MARGIN - SELECTOR_DECORATION_WIDTH-1;
                const uint16_t frame_y = _y_list_item - box_padding;
                oled_draw_frame(frame_x, frame_y, LIST_ITEM_SWITCH_BOX_WIDTH,
                                LIST_ITEM_SWITCH_BOX_WIDTH);
                if (*astra_to_switch_item(_list_item)->value == true) {
                    const uint16_t CHECKED_SWITCH_BOX_WIDTH =
                            LIST_ITEM_SWITCH_BOX_WIDTH - FRAME_WIDTH * 2 - FRAME_WIDTH * 2 * LIST_ITEM_SWITCH_BOX_CHECKED_SIZE_SCALE;
                    oled_draw_box(frame_x + FRAME_WIDTH * (1 + LIST_ITEM_SWITCH_BOX_CHECKED_SIZE_SCALE),
                                  frame_y + FRAME_WIDTH * (1 + LIST_ITEM_SWITCH_BOX_CHECKED_SIZE_SCALE),
                                  CHECKED_SWITCH_BOX_WIDTH,
                                  CHECKED_SWITCH_BOX_WIDTH);
                } else {
                    // do nothing
                }
            }
        } else if (_list_item->type == slider_item) {
            if (_y_list_item + 5 > LIST_INFO_BAR_HEIGHT && _y_list_item - 2 < SCREEN_HEIGHT) {
                oled_draw_V_line(3 + _x_list_item, _y_list_item - 1, 5);
                oled_draw_V_line(6 + _x_list_item, _y_list_item - 1, 5);
                oled_draw_box(2 + _x_list_item, _y_list_item - 2, 3, 3);
                oled_draw_box(5 + _x_list_item, _y_list_item + 2, 3, 3);

                //滑块控件指示器部分
                char _value_str[10] = {};
                int16_t* _value = astra_to_slider_item(_list_item)->value;
                sprintf(_value_str, "%d", *_value);

                int16_t _x_value = OLED_WIDTH - LIST_ITEM_RIGHT_MARGIN - oled_get_str_width(_value_str) + 2;

                //如果选中了就闪烁 否则就一直显示
                if (astra_to_slider_item(_list_item)->is_confirmed) {
                    static uint32_t _last_tick = 0;
                    static bool _is_visiable = false;
                    uint32_t _ticks = get_ticks_ms();

                    if (_is_visiable) {
                        oled_set_draw_color(1);
                        oled_draw_R_box(_x_value, _y_list_item - 4, oled_get_UTF8_width(_value_str) + 4, oled_get_str_height() - 2, 1);
                    }

                    oled_set_draw_color(2);
                    oled_draw_str(_x_value + 2, _y_list_item + oled_get_str_height() / 2, _value_str);

                    if (_ticks - _last_tick >= 1000) {
                        _is_visiable = !_is_visiable;
                        _last_tick = _ticks;
                    }
                } else oled_draw_str(_x_value + 2, _y_list_item + oled_get_str_height() / 2, _value_str);
            }
        } else {
            if (_y_list_item + oled_get_str_height() / 2 > LIST_INFO_BAR_HEIGHT && _y_list_item + oled_get_str_height() / 2 < SCREEN_HEIGHT)
                oled_draw_str(2 + _x_list_item, _y_list_item + oled_get_str_height() / 2, "-");
        }

        astra_set_font(astra_font);
        const bool _is_visible = (_y_list_item + oled_get_str_height() / 2 > LIST_INFO_BAR_HEIGHT &&
                                  _y_list_item - oled_get_str_height() / 2 < SCREEN_HEIGHT);
        if (_is_visible) {
            const int16_t text_x = LIST_TEXT_TO_HEADER_PADDING + _x_list_item;
            const int16_t text_y = _y_list_item + oled_get_str_height() / 2;
            const int16_t text_right_limit = SCREEN_WIDTH - LIST_ITEM_RIGHT_MARGIN - LIST_TEXT_RIGHT_PADDING;
            const int16_t visible_width = text_right_limit - text_x;

            const int16_t text_width = oled_get_UTF8_width(_list_item->content);
            int16_t scroll_offset = 0;
            bool needs_clip = false;

            if (visible_width > 0 && text_width > visible_width) {
                needs_clip = true;
                const uint32_t now = get_ticks_ms();
                if (_list_item->text_scroll_anchor == 0)
                    _list_item->text_scroll_anchor = now;

                const uint32_t elapsed = now - _list_item->text_scroll_anchor;
                const uint32_t LIST_TEXT_SCROLL_PERIOD_MS = (text_width / LIST_WIDGET_SCROLL_SPEED_PX_S) * 1000;
                if (elapsed > LIST_WIDGET_SCROLL_PAUSE_MS) {
                    const uint32_t animated_ms = (elapsed - LIST_WIDGET_SCROLL_PAUSE_MS) % LIST_TEXT_SCROLL_PERIOD_MS;
                    const float progress = (float) animated_ms / (float) LIST_TEXT_SCROLL_PERIOD_MS;
                    const float triangle = (progress <= 0.5f) ? progress * 2.0f : (2.0f - progress * 2.0f);
                    scroll_offset = (int16_t) lrintf((text_width - visible_width) * triangle);
                }
            } else {
                _list_item->text_scroll_anchor = 0;
            }

            oled_set_draw_color(1);
            if (needs_clip) {
                const int16_t clip_x0 = text_x;
                const int16_t clip_x1 = text_right_limit;
                const int16_t clip_y1 = _y_list_item + LIST_ITEM_SPACING / 2;
                const int16_t clip_y0 = clip_y1 - LIST_ITEM_SPACING;
                oled_set_clip_window(clip_x0, clip_y0, clip_x1, clip_y1);
                oled_draw_UTF8(text_x - scroll_offset, text_y, _list_item->content);
                oled_reset_clip_window();
            } else {
                oled_draw_UTF8(text_x, text_y, _list_item->content);
            }
        } else {
            _list_item->text_scroll_anchor = 0;
        }
    }
}

void astra_draw_selector() {
    const int64_t x_selector = lrintf(astra_camera.x_camera) + LIST_ITEM_LEFT_MARGIN;
    const int64_t y_selector = lrintf(astra_selector.y_selector + astra_camera.y_camera);

    oled_set_draw_color(2);
    oled_draw_box(x_selector, y_selector,
                  astra_selector.w_selector_trg, astra_selector.h_selector);

    const int64_t x0 = lrintf(astra_selector.w_selector) + x_selector;
    const int16_t y0 = y_selector;

    const int64_t h = lrintf(astra_selector.h_selector);

    oled_set_draw_color(1);
    static const uint8_t TILE_8x8[SELECTOR_DECORATION_HEIGHT] = {
        0b0101,
        0b1010,
        0b0101,
        0b1010,
        0b0101,
        0b1010,
        0b0101,
        0b1010,
    };
    for (int64_t y = 0; y < h; y += SELECTOR_DECORATION_HEIGHT) {
        const int64_t bh = (h - y > SELECTOR_DECORATION_HEIGHT) ? SELECTOR_DECORATION_HEIGHT : (h - y);
        oled_draw_bMP(x0, y0 + y, SELECTOR_DECORATION_WIDTH, bh, TILE_8x8);
    }
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
