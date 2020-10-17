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
    state->square1_timebase = (2048 - state->square1_freq);
    if (state->square1_tick > state->square1_timebase) {
        state->square1_tick = 0;
    }

    snd_debug(SND_DEBUG_CH1, "ch1 timebase: %d", state->square1_timebase);
}

static uint8_t write_ff11(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    int len = 64 - (val & 0x3F);
    state->square1_lctr = len;
    state->square1_duty_shape = (val & 0xC0) >> 6;

    return 0xFF;
}

static uint8_t write_ff12(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    sound_ve_configure(&state->s1_envelope, val);
    return val;
}

static uint8_t write_ff13(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    state->square1_freq = (state->square1_freq & 0x0700) | val;
    update_timebase_ch1(state);
    return 0xFF;
}

static uint8_t write_ff14(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    if (val & 0x80) {
        state->square1_lctr = 64;
        state->status_reg |= 0x01;

        sound_ve_oninit(&state->s1_envelope);
        state->square1_duty = 0;
    }
    if (val & 0x40) {
        state->square1_lenable = 1;
    }

    state->square1_freq = (state->square1_freq & 0x00ff) | ((val & 0x7) << 8);
    update_timebase_ch1(state);
    return val | 0xBF;
}

// ==========================================================================
// = channel 2
// ==========================================================================

static void update_timebase_ch2(sound_ctlr_t *state) {
    state->square2_timebase = (2048 - state->square2_freq);
    if (state->square2_tick > state->square2_timebase) {
        state->square2_tick = 0;
    }

    snd_debug(SND_DEBUG_CH2, "ch2 timebase: %d", state->square2_timebase);
}

static uint8_t write_ff16(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    int len = 64 - (val & 0x3F);
    state->square2_lctr = len;
    state->square2_duty_shape = (val & 0xC0) >> 6;

    return 0xFF;
}

static uint8_t write_ff17(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    sound_ve_configure(&state->s2_envelope, val);
    return val;
}

static uint8_t write_ff18(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    state->square2_freq = (state->square2_freq & 0x0700) | val;
    update_timebase_ch2(state);
    return 0xFF;
}

static uint8_t write_ff19(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    if (val & 0x80) {
        state->square2_lctr = 64;
        state->status_reg |= 0x02;

        sound_ve_oninit(&state->s2_envelope);
        state->square2_duty = 0;
    }
    if (val & 0x40) {
        state->square2_lenable = 1;
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
    state->square1_tick += ncyc;

    if (state->square1_tick > state->square1_timebase) {
        state->square1_duty = (state->square1_duty + 1) & 0x7;
        state->square1_tick -= state->square1_timebase;
    }
}

int sound_square_getoutput_ch2(sound_ctlr_t *state) {
    return dshape[state->square2_duty_shape][state->square2_duty];
}

void sound_square_onclock_ch2(sound_ctlr_t *state, int ncyc) {
    state->square2_tick += ncyc;

    if (state->square2_tick > state->square2_timebase) {
        state->square2_duty = (state->square2_duty + 1) & 0x7;
        state->square2_tick -= state->square2_timebase;
    }
}

void sound_square_install_regs(sound_ctlr_t *state, void *target, snd_mmio_add_observer_t reg_func) {
    reg_func(target, 0xFF11, (snd_mmio_write_t)&write_ff11, NULL, (void *)state);
    reg_func(target, 0xFF12, (snd_mmio_write_t)&write_ff12, NULL, (void *)state);
    reg_func(target, 0xFF13, (snd_mmio_write_t)&write_ff13, NULL, (void *)state);
    reg_func(target, 0xFF14, (snd_mmio_write_t)&write_ff14, NULL, (void *)state);

    reg_func(target, 0xFF16, (snd_mmio_write_t)&write_ff16, NULL, (void *)state);
    reg_func(target, 0xFF17, (snd_mmio_write_t)&write_ff17, NULL, (void *)state);
    reg_func(target, 0xFF18, (snd_mmio_write_t)&write_ff18, NULL, (void *)state);
    reg_func(target, 0xFF19, (snd_mmio_write_t)&write_ff19, NULL, (void *)state);
}
