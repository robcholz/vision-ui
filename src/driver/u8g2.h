//
// Created by Finn Sheng (Ziheng Sheng) on 11/10/25.
//
#pragma once

#ifndef DRIVER_U8G2_H
#define DRIVER_U8G2_H

#include <stdint.h>

#include "vision_ui_config.h"

#if (VISION_UI_SCREEN_HEIGHT % 8) != 0
#error "VISION_UI_SCREEN_HEIGHT must be divisible by 8 for the u8g2 page buffer"
#endif

#define VISION_UI_TILE_BUF_HEIGHT (VISION_UI_SCREEN_HEIGHT / 8)
#define VISION_UI_BUFFER_SIZE (VISION_UI_SCREEN_WIDTH * VISION_UI_TILE_BUF_HEIGHT)

extern uint8_t U8G2_BUFFER[VISION_UI_BUFFER_SIZE];

#endif // DRIVER_U8G2_H
