#include "sound.h"
#include "sound_internal.h"

// per https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Noise_Channel
static const int noise_div[] = {8, 16, 32, 48, 64, 80, 96, 112};
static const int xorbit[] = {0, 1, 1, 0};

// private

static uint8_t write_ff20(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    int len = 64 - (val & 0x3F);
    state->noise_lctr = len;

    return 0xFF;
}

static uint8_t write_ff21(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    sound_ve_configure(&state->noise_envelope, val);
    snd_debug(SND_DEBUG_CH4, "set noise ve %x", (int)val);
    return val;
}

static uint8_t write_ff22(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    state->noise_low = (val & 0x8) >> 3;
    state->noise_timebase = (noise_div[val & 0x7] << ((val >> 4) & 0xF)) / 8;
    return val;
}

static uint8_t write_ff23(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    if (val & 0x80) {
        state->noise_lctr = 64;
        state->noise_reg = 0x7FFF;
        state->status_reg |= 0x08;
        sound_ve_oninit(&state->noise_envelope);
    }
    if (val & 0x40) {
        state->noise_lenable = 1;
    }

    return val | 0xBF;
}

static void update_noise_register(sound_ctlr_t *state) {
    uint32_t next = xorbit[state->noise_reg & 0x3];
    if (state->noise_low) {
        next = (next << 14) | (next << 6);
        state->noise_reg = ((state->noise_reg >> 1) & 0x3fbf) | next;
    } else {
        next = (next << 14);
        state->noise_reg = (state->noise_reg >> 1) | next;
    }

    // snd_debug(SND_DEBUG_CH4, "%x", state->noise_reg);
}


// public

void sound_noise_onclock(sound_ctlr_t *state, int ncyc) {
    state->noise_tick += ncyc;

    if (state->noise_tick > state->noise_timebase) {
        state->noise_tick -= state->noise_timebase;
        update_noise_register(state);
    }
}

void sound_noise_install_regs(sound_ctlr_t *state, void *target, snd_mmio_add_observer_t reg_func) {
    reg_func(target, 0xFF20, (snd_mmio_write_t)&write_ff20, NULL, (void *)state);
    reg_func(target, 0xFF21, (snd_mmio_write_t)&write_ff21, NULL, (void *)state);
    reg_func(target, 0xFF22, (snd_mmio_write_t)&write_ff22, NULL, (void *)state);
    reg_func(target, 0xFF23, (snd_mmio_write_t)&write_ff23, NULL, (void *)state);
}
