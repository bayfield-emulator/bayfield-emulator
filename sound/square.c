#include "sound.h"
#include "sound_internal.h"

static const int dshape[][8] = {
    {0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 1, 1, 1},
    {0, 1, 1, 1, 1, 1, 1, 0},
};

// ==========================================================================
// = channel 1
// ==========================================================================

static void update_timebase_ch1(sound_ctlr_t *state) {
    sound_timer_set_base(&state->ch1_duty_timer, 2048 - state->square1_freq);
    snd_debug(SND_DEBUG_CH1, "ch1 timebase: %d", 2048 - state->square1_freq);
}

static void do_frequency_sweep(sound_ctlr_t *state) {
    if (!state->square1_swp_enabled) {
        return;
    }

    int t = state->square1_swp_tick - 1;
    if (t != 0) {
        state->square1_swp_tick = t;
        return;
    }

    int mod = state->square1_swp_sha >> state->square1_swp_shift;
    int next_freq;
    if (state->square1_swp_direction == 0) {
        next_freq = state->square1_swp_sha + mod;
    } else {
        next_freq = state->square1_swp_sha - mod;
        if (next_freq < 0) {
            next_freq = 0;
        }
    }

    if (next_freq > 2047) {
        // "if this is greater than 2047, square 1 is disabled"
        SOUND_CHANNEL_DISABLE(state, 1);
        state->square1_swp_enabled = 0;
        snd_debug(SND_DEBUG_CH1, "disabling sq1 sweep");
    }

    snd_debug(SND_DEBUG_CH1, "%d", next_freq);
    state->square1_freq = next_freq;
    state->square1_swp_sha = next_freq;
    state->square1_swp_tick = state->square1_swp_timebase;
    update_timebase_ch1(state);
}

static uint8_t write_ff10(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    if (!state->master_enable) {
        return 0x80;
    }

    int timecode = (val >> 4) & 0x7;
    state->square1_swp_timebase = timecode;
    state->square1_swp_direction = (val >> 3) & 0x1;
    state->square1_swp_shift = val & 0x7;

    snd_debug(SND_DEBUG_CH1, "SET ff10 tb %d neg %d shf %d", state->square1_swp_timebase, state->square1_swp_direction, state->square1_swp_shift);

    return val | 0x80;
}

static uint8_t write_ff11(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    if (!state->master_enable) {
        return 0x3F;
    }

    snd_debug(SND_DEBUG_CONTROLLER, "w to ff11 %d", val);
    int len = 64 - (val & 0x3F);
    state->square1_lctr = len;
    state->square1_duty_shape = (val & 0xC0) >> 6;

    return val | 0x3F;
}

static uint8_t write_ff12(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    if (!state->master_enable) {
        return 0x00;
    }

    sound_ve_configure(&state->s1_envelope, val);
    return val;
}

static uint8_t write_ff13(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    if (!state->master_enable) {
        return 0xFF;
    }

    state->square1_freq = (state->square1_freq & 0x0700) | val;
    update_timebase_ch1(state);
    return 0xFF;
}

static uint8_t write_ff14(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    if (!state->master_enable) {
        return 0xBF;
    }

    state->square1_freq = (state->square1_freq & 0x00ff) | ((val & 0x7) << 8);
    update_timebase_ch1(state);

    if (val & 0x80) {
        if (!state->square1_lctr) {
            state->square1_lctr = 64;
        }

        SOUND_CHANNEL_ENABLE(state, 1);

        sound_ve_oninit(&state->s1_envelope);
        state->square1_duty = 0;
        sound_timer_reset(&state->ch1_duty_timer);

        state->square1_swp_sha = state->square1_freq;
        state->square1_swp_tick = state->square1_swp_timebase;
        if (state->square1_swp_shift != 0) {
            state->square1_swp_enabled = 1;
            state->square1_swp_tick = 1;
            do_frequency_sweep(state);
        }

        state->square1_swp_enabled = (state->square1_swp_timebase != 0 || state->square1_swp_shift != 0);
    }
    if (val & 0x40) {
        state->square1_lenable = 1;
    } else {
        state->square1_lenable = 0;
    }

    return val | 0xBF;
}

// ==========================================================================
// = channel 2
// ==========================================================================

static void update_timebase_ch2(sound_ctlr_t *state) {
    sound_timer_set_base(&state->ch2_duty_timer, 2048 - state->square2_freq);
    snd_debug(SND_DEBUG_CH2, "ch2 timebase: %d", 2048 - state->square2_freq);
}

static uint8_t write_ff15(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    return 0xFF;
}

static uint8_t write_ff16(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    if (!state->master_enable) {
        return 0x3F;
    }

    int len = 64 - (val & 0x3F);
    state->square2_lctr = len;
    state->square2_duty_shape = (val & 0xC0) >> 6;

    return val | 0x3F;
}

static uint8_t write_ff17(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    if (!state->master_enable) {
        return 0x00;
    }

    sound_ve_configure(&state->s2_envelope, val);
    snd_debug(SND_DEBUG_CH2, "ch2 write ve: %x", val);
    return val;
}

static uint8_t write_ff18(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    if (!state->master_enable) {
        return 0xFF;
    }

    state->square2_freq = (state->square2_freq & 0x0700) | val;
    update_timebase_ch2(state);
    return 0xFF;
}

static uint8_t write_ff19(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    if (!state->master_enable) {
        return 0xBF;
    }

    if (val & 0x80) {
        if (!state->square2_lctr) {
            state->square2_lctr = 64;
        }

        SOUND_CHANNEL_ENABLE(state, 2);

        sound_ve_oninit(&state->s2_envelope);
        state->square2_duty = 0;
        sound_timer_reset(&state->ch2_duty_timer);
    }
    if (val & 0x40) {
        state->square2_lenable = 1;
    } else {
        state->square2_lenable = 0;
    }

    state->square2_freq = (state->square2_freq & 0x00ff) | ((val & 0x7) << 8);
    update_timebase_ch2(state);
    return val | 0xBF;
}


// ==========================================================================
// = internal api
// ==========================================================================

int sound_square_getoutput_ch1(sound_ctlr_t *state) {
    return dshape[state->square1_duty_shape][state->square1_duty];
}

void sound_square_onclock_ch1(sound_ctlr_t *state, int ncyc) {
    if (sound_timer_tick(&state->ch1_duty_timer, ncyc)) {
        state->square1_duty = (state->square1_duty + 1) & 0x7;
    }
}

void sound_square_onsweep(sound_ctlr_t *state) {
    do_frequency_sweep(state);
}

int sound_square_getoutput_ch2(sound_ctlr_t *state) {
    return dshape[state->square2_duty_shape][state->square2_duty];
}

void sound_square_onclock_ch2(sound_ctlr_t *state, int ncyc) {
    if (sound_timer_tick(&state->ch2_duty_timer, ncyc)) {
        state->square2_duty = (state->square2_duty + 1) & 0x7;
    }
}

void sound_square_install_regs(sound_ctlr_t *state, void *target, snd_mmio_add_observer_t reg_func) {
    reg_func(target, 0xFF10, (snd_mmio_write_t)&write_ff10, NULL, (void *)state);
    reg_func(target, 0xFF11, (snd_mmio_write_t)&write_ff11, NULL, (void *)state);
    reg_func(target, 0xFF12, (snd_mmio_write_t)&write_ff12, NULL, (void *)state);
    reg_func(target, 0xFF13, (snd_mmio_write_t)&write_ff13, NULL, (void *)state);
    reg_func(target, 0xFF14, (snd_mmio_write_t)&write_ff14, NULL, (void *)state);
    reg_func(target, 0xFF15, (snd_mmio_write_t)&write_ff15, NULL, (void *)state);
    reg_func(target, 0xFF16, (snd_mmio_write_t)&write_ff16, NULL, (void *)state);
    reg_func(target, 0xFF17, (snd_mmio_write_t)&write_ff17, NULL, (void *)state);
    reg_func(target, 0xFF18, (snd_mmio_write_t)&write_ff18, NULL, (void *)state);
    reg_func(target, 0xFF19, (snd_mmio_write_t)&write_ff19, NULL, (void *)state);
}
