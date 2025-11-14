//
// Created by Finn Sheng (Ziheng Sheng) on 11/12/25.
//
#include "vision_ui_animation.h"

#include <math.h>

float vision_ui_smoothstep(float t) {
    // clamp to [0,1]
    if (t < 0.0f) {
        t = 0.0f;
    }
    if (t > 1.0f) {
        t = 1.0f;
    }
    // quintic S-curve: 6t^5 - 15t^4 + 10t^3
    return t * t * t * (t * (6.0f * t - 15.0f) + 10.0f);
}

void vision_ui_animation_s_curve(float* pos, const float pos_trg, const float speed, const float delta_ms) {
    if (*pos == pos_trg) {
        return;
    }

    const float dist = fabsf(*pos - pos_trg);
    if (dist <= 1.0f) {
        *pos = pos_trg;
        return;
    }

    const float raw_ms = (delta_ms > 0.0f ? delta_ms : 16.6667f);
    const float clamped_ms = raw_ms > 33.3333f ? 33.3333f : raw_ms;
    const float dt = clamped_ms / 1000.0f;

    // 2. speed(0~100) → duration(0.25s ~ 0.05s)
    float s_norm = speed * 0.01f; // 0~1
    if (s_norm < 0.0f) {
        s_norm = 0.0f;
    }
    if (s_norm > 1.0f) {
        s_norm = 1.0f;
    }

    float duration = 0.25f - 0.20f * s_norm; // 0.25 → 0.05
    if (duration < 0.01f) {
        duration = 0.01f;
    }

    const float tau = duration / 3.0f;

    const float step = 1.0f - expf(-dt / tau); // 0~1

    *pos += (pos_trg - *pos) * step;

    if (fabsf(*pos - pos_trg) <= 0.5f) {
        *pos = pos_trg;
    }
}

static void vision_ui_animation_2nd_ode(
        const float damping_ratio,
        const float base_nat_freq,
        const float nat_freq_per_speed,
        float* pos,
        float* velocity,
        const float pos_trg,
        const float speed,
        const float delta_ms
) {
    float x = *pos;
    float v = *velocity;

    const float raw_ms = (delta_ms > 0.0f ? delta_ms : 16.6667f);
    const float clamped_ms = raw_ms > 33.3333f ? 33.3333f : raw_ms;
    const float dt = clamped_ms / 1000.0f;

    const float prev_err = x - pos_trg;

    const float omega_n = base_nat_freq + nat_freq_per_speed * speed;
    const float zeta = damping_ratio;

    const float a = -2.0f * zeta * omega_n * v - (omega_n * omega_n) * (x - pos_trg);

    v += a * dt;
    x += v * dt;

    const float new_err = x - pos_trg;

    if (prev_err * new_err <= 0.0f || fabsf(new_err) < 0.5f) {
        x = pos_trg;
        v = 0.0f;
    }

    *velocity = v;
    *pos = x;
}

void vision_ui_animation_2nd_ode_no_overshoot(
        float* pos,
        float* velocity,
        const float pos_trg,
        const float speed,
        const float delta_ms
) {
    vision_ui_animation_2nd_ode(0.8f, 9.5f, 0.2f, pos, velocity, pos_trg, speed, delta_ms);
}

void vision_ui_animation_2nd_ode_slight_overshoot(
        float* pos,
        float* velocity,
        const float pos_trg,
        const float speed,
        const float delta_ms
) {
    vision_ui_animation_2nd_ode(0.7f, 8.0f, 0.2f, pos, velocity, pos_trg, speed, delta_ms);
}
