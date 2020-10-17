#include "sound.h"
#include "sound_internal.h"

// PRIVATE

// int wave_lenable;
// int wave_lctr;
// uint8_t wave_pram[16];
// int wave_pread;
// int wave_volume;
// float wave_timebase;
// int wave_tick;

static const int shf_table[] = {4, 0, 1, 2};

static void update_timebase(sound_ctlr_t *state) {
    state->wave_timebase = (2048 - state->wave_freq) * 16;
    if (state->wave_tick > state->wave_timebase) {
        state->wave_tick = 0;
    }

    snd_debug(SND_DEBUG_CH3, "timebase: %d\n", state->wave_timebase);
}

static uint8_t write_ff1a(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    state->wave_play = val >> 7;
    return val | 0x7F;
}

static uint8_t write_ff1b(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    int len = 256 - val;
    state->wave_lctr = len;

    return 0xFF;
}

static uint8_t write_ff1c(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    state->wave_volume = shf_table[(val >> 5) & 0x03];
    return val | 0x9F;
}

static uint8_t write_ff1d(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    state->wave_freq = (state->wave_freq & 0x0700) | val;
    update_timebase(state);
    return 0xFF;
}

static uint8_t write_ff1e(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    if (val & 0x80) {
        state->wave_lctr = 256;
        state->wave_pread = 0;
        state->status_reg |= 0x04;
    }
    if (val & 0x40) {
        state->wave_lenable = 1;
    }

    state->wave_freq = (state->wave_freq & 0x00ff) | ((val & 0x7) << 8);
    update_timebase(state);
    return val | 0xBF;
}

// set waveram
static uint8_t write_ff3x(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    if (0) {
        state->wave_pram[state->wave_pread] = val;
    } else {
        state->wave_pram[addr - 0xFF30] = val;
    }

    // printf("wpram:");
    // for (int i = 0; i < 16; ++i) {
    //     printf("%02x", state->wave_pram[i]);
    // }
    // printf("\n");
    return 0;
}


// public

void sound_wave_onclock(sound_ctlr_t *state, int ncyc) {
    if (!state->wave_play) {
        return;
    }

    state->wave_tick += ncyc;

    if (state->wave_tick > state->wave_timebase) {
        state->wave_tick -= state->wave_timebase;
        state->wave_pread = (state->wave_pread + 1) & 0x1F;
    }
}

void sound_wave_install_regs(sound_ctlr_t *state, void *target, snd_mmio_add_observer_t reg_func) {
    reg_func(target, 0xFF1A, (snd_mmio_write_t)&write_ff1a, NULL, (void *)state);
    reg_func(target, 0xFF1B, (snd_mmio_write_t)&write_ff1b, NULL, (void *)state);
    reg_func(target, 0xFF1C, (snd_mmio_write_t)&write_ff1c, NULL, (void *)state);
    reg_func(target, 0xFF1D, (snd_mmio_write_t)&write_ff1d, NULL, (void *)state);
    reg_func(target, 0xFF1E, (snd_mmio_write_t)&write_ff1e, NULL, (void *)state);

    reg_func(target, 0xFF30, (snd_mmio_write_t)&write_ff3x, NULL, (void *)state);
    reg_func(target, 0xFF31, (snd_mmio_write_t)&write_ff3x, NULL, (void *)state);
    reg_func(target, 0xFF32, (snd_mmio_write_t)&write_ff3x, NULL, (void *)state);
    reg_func(target, 0xFF33, (snd_mmio_write_t)&write_ff3x, NULL, (void *)state);
    reg_func(target, 0xFF34, (snd_mmio_write_t)&write_ff3x, NULL, (void *)state);
    reg_func(target, 0xFF35, (snd_mmio_write_t)&write_ff3x, NULL, (void *)state);
    reg_func(target, 0xFF36, (snd_mmio_write_t)&write_ff3x, NULL, (void *)state);
    reg_func(target, 0xFF37, (snd_mmio_write_t)&write_ff3x, NULL, (void *)state);
    reg_func(target, 0xFF38, (snd_mmio_write_t)&write_ff3x, NULL, (void *)state);
    reg_func(target, 0xFF39, (snd_mmio_write_t)&write_ff3x, NULL, (void *)state);
    reg_func(target, 0xFF3A, (snd_mmio_write_t)&write_ff3x, NULL, (void *)state);
    reg_func(target, 0xFF3B, (snd_mmio_write_t)&write_ff3x, NULL, (void *)state);
    reg_func(target, 0xFF3C, (snd_mmio_write_t)&write_ff3x, NULL, (void *)state);
    reg_func(target, 0xFF3D, (snd_mmio_write_t)&write_ff3x, NULL, (void *)state);
    reg_func(target, 0xFF3E, (snd_mmio_write_t)&write_ff3x, NULL, (void *)state);
    reg_func(target, 0xFF3F, (snd_mmio_write_t)&write_ff3x, NULL, (void *)state);
}
