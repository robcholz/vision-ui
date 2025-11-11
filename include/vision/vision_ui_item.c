//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//
#include "vision_ui_item.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "vision_ui_core.h"

void* ASTRA_FONT;

void astra_set_font(void* font) {
    if (font != ASTRA_FONT) {
        oled_set_font(font);
        ASTRA_FONT = font;
    }
}

astra_info_bar_t ASTRA_INFO_BAR = {0, 1, 0 - 2 * INFO_BAR_HEIGHT, 0 - 2 * INFO_BAR_HEIGHT, 80, 80, false, 0, 1};

void astra_push_info_bar(char* content, const uint16_t span) {
    //设定显示时间的概念，超过了显示时间，就将ytrg设为初始位置，如果在显示时间之内，有新的消息涌入，则y和ytrg都不变，继续显示，且显示时间清零
    //只有显示时间到了的时候，才会复位

    ASTRA_INFO_BAR.time = get_ticks_ms();
    ASTRA_INFO_BAR.content = content;
    ASTRA_INFO_BAR.span = span;
    ASTRA_INFO_BAR.is_running = false; //每次进入该函数都代表有新的消息涌入，所以需要重置is_running

    //展开弹窗 收回弹窗和同步时间戳需要在循环中进行 所以移到了drawer中
    if (!ASTRA_INFO_BAR.is_running) {
        ASTRA_INFO_BAR.time_start = get_ticks_ms();
        ASTRA_INFO_BAR.y_info_bar_trg = 0;
        ASTRA_INFO_BAR.is_running = true;
    }

    astra_set_font(ASTRA_FONT);
    ASTRA_INFO_BAR.w_info_bar_trg = oled_get_UTF8_width(ASTRA_INFO_BAR.content) + INFO_BAR_WIDTH;
}

astra_pop_up_t ASTRA_POP_UP = {0, 1, 0 - 2 * POP_UP_HEIGHT, 0 - 2 * POP_UP_HEIGHT, 80, 80, false, 0, 1};

void astra_push_pop_up(char* content, const uint16_t span) {
    ASTRA_POP_UP.time = get_ticks_ms();
    ASTRA_POP_UP.content = content;
    ASTRA_POP_UP.span = span;
    ASTRA_POP_UP.is_running = false;

    //弹出
    if (!ASTRA_POP_UP.is_running) {
        ASTRA_POP_UP.time_start = get_ticks_ms();
        ASTRA_POP_UP.y_pop_up_trg = 20;
        ASTRA_POP_UP.is_running = true;
    }

    astra_set_font(ASTRA_FONT);
    ASTRA_POP_UP.w_pop_up_trg = oled_get_UTF8_width(ASTRA_POP_UP.content) + POP_UP_WIDTH;
}

// astra_list_item_t astra_list_item_root = {};

astra_switch_item_t* astra_to_switch_item(astra_list_item_t* astra_list_item) {
    if (astra_list_item != NULL && astra_list_item->type == SWITCH_ITEM)
        return (astra_switch_item_t*) astra_list_item;

    return (astra_switch_item_t*) astra_get_root_list();
}

astra_slider_item_t* astra_to_slider_item(astra_list_item_t* astra_list_item) {
    if (astra_list_item != NULL && astra_list_item->type == SLIDER_ITEM)
        return (astra_slider_item_t*) astra_list_item;

    return (astra_slider_item_t*) astra_get_root_list();
}

astra_user_item_t* astra_to_user_item(astra_list_item_t* astra_list_item) {
    if (astra_list_item != NULL && astra_list_item->type == USER_ITEM)
        return (astra_user_item_t*) astra_list_item;

    return (astra_user_item_t*) astra_get_root_list();
}

//tips: 不会重复创建root节点
astra_list_item_t* astra_get_root_list() {
    static astra_list_item_t* astra_list_root_item = NULL;
    if (astra_list_root_item == NULL) {
        astra_list_root_item = malloc(sizeof(astra_list_item_t));
        memset(astra_list_root_item, 0, sizeof(astra_list_item_t));
        astra_list_root_item->type = LIST_ITEM;
        astra_list_root_item->content = "VisionUI";
        astra_push_item_to_list(astra_list_root_item, astra_new_title_item(astra_list_root_item->content));
    }
    return astra_list_root_item;
}

astra_list_item_t* astra_new_list_item(char* content) {
    astra_list_item_t* astra_list_item = malloc(sizeof(astra_list_item_t));
    memset(astra_list_item, 0, sizeof(astra_list_item_t));
    astra_list_item->type = LIST_ITEM;
    astra_list_item->content = content;
    astra_push_item_to_list(astra_list_item, astra_new_title_item(astra_list_item->content));
    return astra_list_item;
}

astra_list_item_t* astra_new_title_item(const char* title) {
    astra_list_item_t* astra_list_item = malloc(sizeof(astra_list_item_t));
    memset(astra_list_item, 0, sizeof(astra_list_item_t));
    astra_list_item->type = TITLE_ITEM;
    astra_list_item->content = title;
    return astra_list_item;
}

astra_list_item_t* astra_new_switch_item(char* content, const bool default_value, void (*on_changed)(bool value)) {
    astra_switch_item_t* astra_switch_item = malloc(sizeof(astra_switch_item_t));
    memset(astra_switch_item, 0, sizeof(astra_switch_item_t));
    astra_switch_item->base_item.type = SWITCH_ITEM;
    astra_switch_item->base_item.content = content;
    astra_switch_item->value = default_value;
    astra_switch_item->on_changed = on_changed;
    return (astra_list_item_t*) astra_switch_item;
}

astra_list_item_t* astra_new_slider_item(char* content, int16_t default_value, uint8_t step, int16_t min, int16_t max,
                                         void (*on_changed)(int16_t value)) {
    astra_slider_item_t* astra_slider_item = malloc(sizeof(astra_slider_item_t));
    memset(astra_slider_item, 0, sizeof(astra_slider_item_t));
    astra_slider_item->base_item.type = SLIDER_ITEM;
    astra_slider_item->base_item.content = content;
    astra_slider_item->value = default_value;
    astra_slider_item->value_step = step;
    astra_slider_item->value_min = min;
    astra_slider_item->value_max = max;
    astra_slider_item->on_changed = on_changed;
    return (astra_list_item_t*) astra_slider_item;
}

astra_list_item_t* astra_new_user_item(char* content, void (*init_function)(), void (*loop_function)(), void (*exit_function)()) {
    astra_user_item_t* astra_user_item = malloc(sizeof(astra_user_item_t));
    memset(astra_user_item, 0, sizeof(astra_user_item_t));
    astra_user_item->base_item.type = USER_ITEM;
    astra_user_item->base_item.content = content;
    astra_user_item->init_function = init_function;
    astra_user_item->loop_function = loop_function;
    astra_user_item->exit_function = exit_function;
    return (astra_list_item_t*) astra_user_item; //转换回基类 但保留专有数据
}

astra_selector_t ASTRA_SELECTOR = {};

astra_selector_t* astra_get_selector() {
    return &ASTRA_SELECTOR;
}

bool astra_bind_item_to_selector(astra_list_item_t* item) {
    if (item == NULL) return false;

    //找item在父节点中的序号
    uint8_t temp_index = 0;
    if (item->parent != NULL) {
        for (uint8_t i = 0; i < item->parent->child_num; i++) {
            if (item->parent->child_list_item[i] == item) {
                temp_index = i;
                break;
            }
        }
    }

    //坐标在refresh内部更新
    if (ASTRA_SELECTOR.selected_item == NULL) {
        ASTRA_SELECTOR.y_selector = 2 * SCREEN_HEIGHT; //给个初始坐标做动画
        ASTRA_SELECTOR.h_selector = 160;
    }
    ASTRA_SELECTOR.selected_index = temp_index;
    ASTRA_SELECTOR.selected_item = item;

    return true;
}

void astra_selector_go_next_item() {
    if (ASTRA_SELECTOR.selected_item->type == SLIDER_ITEM && astra_to_slider_item(ASTRA_SELECTOR.selected_item)->is_confirmed) {
        astra_slider_item_t* selected_slider_item = astra_to_slider_item(ASTRA_SELECTOR.selected_item);
        selected_slider_item->value += selected_slider_item->value_step;
        if (selected_slider_item->value >= selected_slider_item->value_max) {
            selected_slider_item->value = selected_slider_item->value_max;
        }
        selected_slider_item->on_changed(selected_slider_item->value);
        return;
    }

    if (ASTRA_SELECTOR.selected_item->type == USER_ITEM && astra_to_user_item(ASTRA_SELECTOR.selected_item)->in_user_item) return;

    //到达最末端
    if (ASTRA_SELECTOR.selected_index == ASTRA_SELECTOR.selected_item->parent->child_num - 1) {
        ASTRA_SELECTOR.selected_item = ASTRA_SELECTOR.selected_item->parent->child_list_item[0];
        ASTRA_SELECTOR.selected_index = 0;
        return;
    }

    ASTRA_SELECTOR.selected_item = ASTRA_SELECTOR.selected_item->parent->child_list_item[++ASTRA_SELECTOR.selected_index];
}

void astra_selector_go_prev_item() {
    if (ASTRA_SELECTOR.selected_item->type == SLIDER_ITEM && astra_to_slider_item(ASTRA_SELECTOR.selected_item)->is_confirmed) {
        astra_slider_item_t* selected_slider_item = astra_to_slider_item(ASTRA_SELECTOR.selected_item);
        selected_slider_item->value -= selected_slider_item->value_step;
        if (selected_slider_item->value <= selected_slider_item->value_min) {
            selected_slider_item->value = selected_slider_item->value_min;
        }
        selected_slider_item->on_changed(selected_slider_item->value);
        return;
    }

    if (ASTRA_SELECTOR.selected_item->type == USER_ITEM && astra_to_user_item(ASTRA_SELECTOR.selected_item)->in_user_item) return;

    //到达最前端
    if (ASTRA_SELECTOR.selected_index == 0) {
        ASTRA_SELECTOR.selected_item = ASTRA_SELECTOR.selected_item->parent->child_list_item[
            ASTRA_SELECTOR.selected_item->parent->child_num - 1];
        ASTRA_SELECTOR.selected_index = ASTRA_SELECTOR.selected_item->parent->child_num - 1;
        return;
    }

    ASTRA_SELECTOR.selected_item = ASTRA_SELECTOR.selected_item->parent->child_list_item[--ASTRA_SELECTOR.selected_index];
}

bool ASTRA_EXIT_ANIMATION_FINISHED = true;

/** @brief 确认当前选择的item
  * @note 如果选择了list 就进入选择的list
  * @note 如果选择了特殊item 就翻转/调整对应的值
  */
void astra_selector_jump_to_selected_item() {
    if (!IS_IN_ASTRA) return;

    if (ASTRA_SELECTOR.selected_item->type == USER_ITEM) {
        ASTRA_EXIT_ANIMATION_FINISHED = false;
        // astra_selector.selected_item->in_user_item = true;
        astra_user_item_t* selected_user_item = astra_to_user_item(ASTRA_SELECTOR.selected_item);
        selected_user_item->entering_user_item = true;
        selected_user_item->exiting_user_item = false;
        selected_user_item->user_item_inited = false;
        selected_user_item->user_item_looping = false;
        return;
    }

    if (ASTRA_SELECTOR.selected_item->type == TITLE_ITEM) {
        astra_selector_exit_current_item();
        return;
    }

    if (ASTRA_SELECTOR.selected_item->type == SWITCH_ITEM) {
        astra_switch_item_t*
                selected_switch_item = astra_to_switch_item(ASTRA_SELECTOR.selected_item);
        selected_switch_item->value = !selected_switch_item->value;
        selected_switch_item->on_changed(selected_switch_item->value);
        return;
    }

    if (ASTRA_SELECTOR.selected_item->type == SLIDER_ITEM) {
        astra_slider_item_t* selected_slider_item = astra_to_slider_item(ASTRA_SELECTOR.selected_item);
        if (!selected_slider_item->is_confirmed) {
            selected_slider_item->is_confirmed = true; //如果没选中 就选中
            return;
        }
        if (selected_slider_item->is_confirmed) {
            selected_slider_item->is_confirmed = false;
            return;
        }
    }

    if (ASTRA_SELECTOR.selected_item->child_num == 0) return;

    //给选择的item的子item坐标清零 做动画
    for (uint8_t i = 0; i < ASTRA_SELECTOR.selected_item->child_num; i++)
        ASTRA_SELECTOR.selected_item->child_list_item[i]->y_list_item = 0;

    ASTRA_SELECTOR.selected_index = 0;
    ASTRA_SELECTOR.selected_item = ASTRA_SELECTOR.selected_item->child_list_item[0];
}

void astra_selector_exit_current_item() {
    if (ASTRA_SELECTOR.selected_item->type == SLIDER_ITEM && astra_to_slider_item(ASTRA_SELECTOR.selected_item)->is_confirmed) {
        //如果已选中又长按退出键
        astra_slider_item_t* selected_slider_item = astra_to_slider_item(ASTRA_SELECTOR.selected_item);

        if (selected_slider_item->is_confirmed) {
            selected_slider_item->is_confirmed = false;
            return;
        }
    }

    if (ASTRA_SELECTOR.selected_item->type == USER_ITEM && astra_to_user_item(ASTRA_SELECTOR.selected_item)->in_user_item) {
        ASTRA_EXIT_ANIMATION_FINISHED = false; //需要重新绘制退场动画
        // astra_selector.selected_item->in_user_item = false;
        astra_user_item_t* selected_user_item = astra_to_user_item(ASTRA_SELECTOR.selected_item);
        selected_user_item->entering_user_item = false;
        selected_user_item->exiting_user_item = true;
        selected_user_item->user_item_inited = false;
        selected_user_item->user_item_looping = false;
        return;
    }

    if (ASTRA_SELECTOR.selected_item->parent->layer == 0 && IS_IN_ASTRA) {
        if (ALLOW_EXIT_ASTRA_UI_BY_USER) {
            IS_IN_ASTRA = false;
        }
        return;
    }

    //给选择的item的父item的父item的所有子item坐标清零 做动画
    for (uint8_t i = 0; i < ASTRA_SELECTOR.selected_item->parent->parent->child_num; i++) {
        ASTRA_SELECTOR.selected_item->parent->parent->child_list_item[i]->y_list_item = 0;
    }

    //找到当前选择的item的父item在它的父item中的位置
    uint8_t temp_index = 0;
    for (uint8_t i = 0; i < ASTRA_SELECTOR.selected_item->parent->parent->child_num; i++) {
        if (ASTRA_SELECTOR.selected_item->parent->parent->child_list_item[i] == ASTRA_SELECTOR.selected_item->parent) {
            temp_index = i;
            break;
        }
    }
    ASTRA_SELECTOR.selected_index = temp_index;
    ASTRA_SELECTOR.selected_item = ASTRA_SELECTOR.selected_item->parent;
}

bool astra_push_item_to_list(astra_list_item_t* parent, astra_list_item_t* child) {
    if (parent == NULL) return false;
    if (child == NULL) return false;
    if (parent->child_num >= MAX_LIST_CHILD_NUM) return false;
    if (parent->layer >= MAX_LIST_LAYER) return false;

    child->layer = parent->layer + 1;

    astra_set_font(ASTRA_FONT);
    float next_y = LIST_TITLE_TO_DISPLAY_TOP_PADDING;
    if (parent->child_num > 0) {
        astra_list_item_t* last_child = parent->child_list_item[parent->child_num - 1];
        const uint8_t gap_after_last = (last_child->type == TITLE_ITEM)
                                           ? LIST_TITLE_TO_FRAME_PADDING
                                           : LIST_FRAME_BETWEEN_PADDING;
        next_y = last_child->y_list_item_trg + LIST_FRAME_FIXED_HEIGHT + gap_after_last;
    } else if (child->type != TITLE_ITEM) {
        // 没有标题时仍旧保留顶端 padding
        next_y = LIST_TITLE_TO_DISPLAY_TOP_PADDING;
    }
    child->y_list_item_trg = next_y;

    if (parent->layer == 0 && parent->child_num == 0) {
        astra_bind_item_to_selector(child); //初始化并绑定selector
        astra_bind_selector_to_camera(&ASTRA_SELECTOR); //初始化并绑定camera
    }

    parent->child_list_item[parent->child_num++] = child;
    child->parent = parent;

    return true;
}

astra_camera_t ASTRA_CAMERA = {0, 0, 0, 0}; //在refresh加上camera的坐标

void astra_bind_selector_to_camera(astra_selector_t* selector) {
    if (selector == NULL) return;

    ASTRA_CAMERA.selector = selector; //坐标在refresh内部更新
}
