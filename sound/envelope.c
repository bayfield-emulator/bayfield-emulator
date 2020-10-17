#include "sound.h"

void sound_ve_onclock(sound_ve_t *env) {
    if (!env->ve_cur_enabled) {
        return;
    }

    int tick = env->ve_cur_tick - 1;
    if (tick == 0) {
        if (env->ve_direction) {
            if (++env->ve_cur_volume > 0xF) {
                env->ve_cur_volume = 0xF;
                env->ve_cur_enabled = 0;
            }
        } else {
            if (--env->ve_cur_volume < 0) {
                env->ve_cur_volume = 0;
                env->ve_cur_enabled = 0;
            }
        }

        env->ve_cur_tick = env->ve_ctr_base;
    } else {
        env->ve_cur_tick = tick;
    }
}

void sound_ve_oninit(sound_ve_t *env) {
    env->ve_cur_tick = env->ve_ctr_base? env->ve_ctr_base : 8;
    env->ve_cur_enabled = env->ve_ctr_base? 1 : 0;
    env->ve_cur_volume = env->ve_init_volume;
}

void sound_ve_configure(sound_ve_t *env, uint8_t regval) {
    env->ve_init_volume = (regval & 0xF0) >> 4;
    env->ve_direction = (regval & 0x08) >> 3;
    env->ve_ctr_base = regval & 0x07;

    if (!env->ve_cur_enabled) {
        env->ve_cur_volume = env->ve_init_volume;
    }
    env->ve_cur_enabled = (regval & 0x07)? 1 : 0;
}
