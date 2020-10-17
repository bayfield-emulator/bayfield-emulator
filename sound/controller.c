#include <stdlib.h>
#include <string.h>

#include "sound.h"
#include "sound_internal.h"

//==----------------------------------------------------------------

static uint8_t write_ff24(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    state -> volume_left = (val & 0xF0) >> 4;
    state -> volume_right = (val & 0xF0);

    snd_debug(SND_DEBUG_CONTROLLER, "vol: L:%d R:%d", state->volume_left, state->volume_right);

    return val & 0x77;
}

static uint8_t write_ff25(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    state->pan_reg = val;
    snd_debug(SND_DEBUG_CONTROLLER, "pan:  R: %d %d %d %d L: %d %d %d %d",
        val & 0x80, val & 0x40, val & 0x20, val & 0x10,
        val & 0x08, val & 0x04, val & 0x02, val & 0x01);

    return val;
}

static uint8_t write_ff26(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    state->master_enable = (val & 0x80) >> 7;
    snd_debug(SND_DEBUG_CONTROLLER, "sound master: %u", state->master_enable);

    return val & 0x80;
}

static void do_length(sound_ctlr_t *state) {
    if (state->square1_lenable) {
        state->square1_lctr--;
        if (state->square1_lctr == 0) {
            state->status_reg &= 0x8E;
        }
    }

    if (state->square2_lenable) {
        state->square2_lctr--;
        if (state->square2_lctr == 0) {
            state->status_reg &= 0x8D;
        }
    }

    if (state->wave_lenable) {
        state->wave_lctr--;
        if (state->wave_lctr == 0) {
            state->status_reg &= 0x8B;
        }
    }

    if (state->noise_lenable) {
        state->noise_lctr--;
        if (state->noise_lctr == 0) {
            state->status_reg &= 0x87;
        }
    }
}

static void do_volume_env(sound_ctlr_t *state) {
    sound_ve_onclock(&state->s1_envelope, 0);
    sound_ve_onclock(&state->s2_envelope, 0);
    sound_ve_onclock(&state->noise_envelope, 1);
}

static void sequencer(sound_ctlr_t *state) {
    int step = state->step & 0x7;
    switch(state->step) {
    case 0:
        do_length(state);
        break;
    case 2:
        do_length(state);
        // sweep
        break;
    case 4:
        do_length(state);
        break;
    case 6:
        do_length(state);
        // sweep
        break;
    case 7:
        do_volume_env(state);
        break;
    default:
        break;
    }

    state->step = step + 1;
}

static void do_output(sound_ctlr_t *state, int16_t reference_volume) {
    int32_t sample_l = 0;
    int32_t sample_r = 0;

    if (state->master_enable) {
        if (state->status_reg & 0x8) {
            int noise_out = (state->noise_reg & 1)? 0 : 1;
            int16_t ns = noise_out * reference_volume * SOUND_VE_GETVOL(&state->noise_envelope);
            sample_r += ((state->pan_reg & 0x08)? ns : 0);
            sample_l += ((state->pan_reg & 0x80)? ns : 0);
        }

        if ((state->status_reg & 0x4) && state->wave_play) {
            int samp = state->wave_pram[state->wave_pread >> 1];
            if (state->wave_pread & 1) {
                samp = samp & 0xF;
            } else {
                samp = (samp >> 4) & 0xF;
            }
            int rv = samp >> state->wave_volume;
            int16_t ns = (rv / 15.0) * reference_volume;
            sample_r += (state->pan_reg & 0x04)? ns : 0;
            sample_l += (state->pan_reg & 0x40)? ns : 0;
        }

        if (state->status_reg & 0x2) {
            int lv = sound_square_getoutput_ch2(state);
            int16_t ns = lv * reference_volume * SOUND_VE_GETVOL(&state->s2_envelope);
            sample_r += (state->pan_reg & 0x02)? ns : 0;
            sample_l += (state->pan_reg & 0x20)? ns : 0;
        }

        if (state->status_reg & 0x1) {
            int lv = sound_square_getoutput_ch1(state);
            int16_t ns = lv * reference_volume * SOUND_VE_GETVOL(&state->s1_envelope);
            sample_r += (state->pan_reg & 0x01)? ns : 0;
            sample_l += (state->pan_reg & 0x10)? ns : 0;
        }
    }

    state->prod_buffer[state->prod_count++] = (int16_t)(sample_l / 4);
    state->prod_buffer[state->prod_count++] = (int16_t)(sample_r / 4);

    if (state->prod_count == state->prod_bufsize) {
        state->feed_callback(state->prod_buffer, state->prod_bufsize * sizeof(int16_t), state->feed_context);
        state->prod_count = 0;
    }
}

void sound_init(sound_ctlr_t *state) {
    state->prod_buffer = calloc(2048, sizeof(int16_t));
    state->prod_bufsize = 2048;
    state->prod_count = 0;

    memset(state->wave_pram, 0, 16);
}

void sound_run_controller(sound_ctlr_t *state, int ncyc) {
    uint32_t clk = state->clocks + ncyc;
    uint32_t pt = state->prod_tick + ncyc;

    if (pt >= state->prod_tick_base) {
        do_output(state, 10000);
        pt -= state->prod_tick_base;
    }

    state->prod_tick = pt;

    if (clk >= 2048) {
        sequencer(state);
        state->clocks = clk - 2048;
    } else {
        state->clocks = clk;
    }

    if (state->status_reg & 0x08) {
        sound_noise_onclock(state, ncyc);
    }
    if (state->status_reg & 0x04) {
        sound_wave_onclock(state, ncyc);
    }
    if (state->status_reg & 0x02) {
        sound_square_onclock_ch2(state, ncyc);
    }
    if (state->status_reg & 0x01) {
        sound_square_onclock_ch1(state, ncyc);
    }
}

void sound_install_regs(sound_ctlr_t *state, void *target, snd_mmio_add_observer_t reg_func) {
    reg_func(target, 0xFF24, (snd_mmio_write_t)&write_ff24, NULL, (void *)state);
    reg_func(target, 0xFF25, (snd_mmio_write_t)&write_ff25, NULL, (void *)state);
    reg_func(target, 0xFF26, (snd_mmio_write_t)&write_ff26, NULL, (void *)state);

    sound_square_install_regs(state, target, reg_func);
    sound_noise_install_regs(state, target, reg_func);
    sound_wave_install_regs(state, target, reg_func);
}

void sound_set_output(sound_ctlr_t *state, int samples_per_second, sound_feed_buffer_t callback, void *context) {
    state->feed_callback = callback;
    state->feed_context = context;
    state->prod_tick_base = (1048576 / samples_per_second);
    state->prod_tick = 0;

    snd_debug(SND_DEBUG_CONTROLLER, "tickbase: %d", state->prod_tick_base);
}