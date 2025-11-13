//
// Created by Finn Sheng (Ziheng Sheng) on 11/12/25.
//

#ifndef VISION_UI_ANIMATION_H
#define VISION_UI_ANIMATION_H

extern float vision_ui_smoothstep(float t);

extern void vision_ui_animation_s_curve(float* pos, float pos_trg, float speed, float delta_ms);

extern void vision_ui_animation_2nd_ode_no_overshoot(float* pos, float* velocity, float post_trg, float speed, float delta_ms);

extern void vision_ui_animation_2nd_ode_slight_overshoot(float* pos, float* velocity, float post_trg, float speed, float delta_ms);

#endif // VISION_UI_ANIMATION_H
