//
// Created by Finn Sheng (Ziheng Sheng) on 11/12/25.
//
#include "vision_ui_animation.h"

#include <math.h>

float vision_ui_smoothstep(float t) {
    if (t < 0) {
        t = 0;
    }
    if (t > 1) {
        t = 1;
    }
    return t * t * (3.0f - 2.0f * t); // 3t^2 - 2t^3
}

void vision_ui_animation_do(float* pos, const float pos_trg, const float speed, const float delta_ms) {
    if (*pos == pos_trg) {
        return;
    }

    if (fabsf(*pos - pos_trg) <= 1.0f) {
        *pos = pos_trg;
        return;
    }

    const float frame_ms = delta_ms > 0.0f ? delta_ms : 16.6667f;
    const float frame_ratio = frame_ms / 16.6667f;
    const float step = (pos_trg - *pos) / (100.0f - speed) * frame_ratio;
    *pos += step;
}
