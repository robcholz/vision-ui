//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//
#pragma once

#ifndef DRIVER_U8G2_H
#define DRIVER_U8G2_H

#include <stdint.h>

#include <u8g2.h>

#include "vision/vision_ui.h"
#include "vision_ui_config.h"

#if (VISION_UI_SCREEN_HEIGHT % 8) != 0
#error "VISION_UI_SCREEN_HEIGHT must be divisible by 8 for the u8g2 page buffer"
#endif

#define VISION_UI_TILE_BUF_HEIGHT (VISION_UI_SCREEN_HEIGHT / 8)
#define VISION_UI_BUFFER_SIZE (VISION_UI_SCREEN_WIDTH * VISION_UI_TILE_BUF_HEIGHT)

extern uint8_t U8G2_BUFFER[VISION_UI_BUFFER_SIZE];

typedef struct vision_ui_u8g2_driver_t {
    u8g2_t* handle;
    vision_ui_font_t current_font;
    int8_t str_top;
    int8_t str_bottom;
} vision_ui_u8g2_driver_t;

extern const vision_ui_driver_ops_t VISION_UI_U8G2_DRIVER_OPS;

void vision_ui_u8g2_driver_init(vision_ui_u8g2_driver_t* driver, u8g2_t* handle);
vision_ui_driver_t vision_ui_u8g2_driver_descriptor(vision_ui_u8g2_driver_t* driver);

#endif // DRIVER_U8G2_H
