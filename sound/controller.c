#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "sound.h"
#include "sound_internal.h"

//==----------------------------------------------------------------

static uint8_t write_ff24(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    if (!state->master_enable) {
        return 0x00;
    }

    state -> volume_left = (val & 0x70) >> 4;
    state -> volume_right = (val & 0x7);

    snd_debug(SND_DEBUG_CONTROLLER, "vol: L:%d R:%d", state->volume_left, state->volume_right);

    return val;
}

static uint8_t write_ff25(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    if (!state->master_enable) {
        return 0x00;
    }

    state->pan_reg = val;
    snd_debug(SND_DEBUG_CONTROLLER, "pan:  R: %d %d %d %d L: %d %d %d %d",
        val & 0x80, val & 0x40, val & 0x20, val & 0x10,
        val & 0x08, val & 0x04, val & 0x02, val & 0x01);

    return val;
}

static uint8_t write_ff26(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    bc_cpu_t *cpu = (bc_cpu_t *)gb;

    // Important: master_enable needs to stay on until we're done clearing registers
    // because of the write guards.
    int next_master_enable = (val & 0x80) >> 7;
    if (!next_master_enable) {
        // slow, but we need to make sure cleared states are visible
        // to the default read handler
        bc_mmap_putvalue(&cpu->mem, 0xFF10, 0x00);
        bc_mmap_putvalue(&cpu->mem, 0xFF11, 0x00);
        bc_mmap_putvalue(&cpu->mem, 0xFF12, 0x00);
        bc_mmap_putvalue(&cpu->mem, 0xFF14, 0x00);
        bc_mmap_putvalue(&cpu->mem, 0xFF16, 0x00);
        bc_mmap_putvalue(&cpu->mem, 0xFF17, 0x00);
        bc_mmap_putvalue(&cpu->mem, 0xFF19, 0x00);
        bc_mmap_putvalue(&cpu->mem, 0xFF1A, 0x00);
        bc_mmap_putvalue(&cpu->mem, 0xFF1B, 0x00);
        bc_mmap_putvalue(&cpu->mem, 0xFF1C, 0x00);
        bc_mmap_putvalue(&cpu->mem, 0xFF1E, 0x00);
        bc_mmap_putvalue(&cpu->mem, 0xFF20, 0x00);
        bc_mmap_putvalue(&cpu->mem, 0xFF21, 0x00);
        bc_mmap_putvalue(&cpu->mem, 0xFF22, 0x00);
        bc_mmap_putvalue(&cpu->mem, 0xFF23, 0x00);
        bc_mmap_putvalue(&cpu->mem, 0xFF24, 0x00);
        bc_mmap_putvalue(&cpu->mem, 0xFF25, 0x00);
        // don't clear FF26, for obvious reasons...

        state->status_reg = 0;
    }

    state->master_enable = next_master_enable;
    snd_debug(SND_DEBUG_CONTROLLER, "sound master: %u", state->master_enable);
    return 0;
}

static uint8_t read_ff26(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t prev) {
    uint8_t ret = (state->master_enable << 7) | (state->status_reg);
    snd_debug(SND_DEBUG_CONTROLLER, "read ff26: %x %d", ret, state->square1_lctr);

    return ret | 0x70;
}

static uint8_t write_ff2x(void *gb, sound_ctlr_t *state, uint16_t addr, uint8_t val) {
    return 0xFF;
}

static void do_length(sound_ctlr_t *state) {
    if (state->square1_lenable) {
        state->square1_lctr--;
        if (state->square1_lctr == 0) {
            SOUND_CHANNEL_DISABLE(state, 1);
        }
    }

    if (state->square2_lenable) {
        state->square2_lctr--;
        if (state->square2_lctr == 0) {
            SOUND_CHANNEL_DISABLE(state, 2);
        }
    }

    if (state->wave_lenable) {
        state->wave_lctr--;
        if (state->wave_lctr == 0) {
            SOUND_CHANNEL_DISABLE(state, 3);
        }
    }

    if (state->noise_lenable) {
        state->noise_lctr--;
        if (state->noise_lctr == 0) {
            SOUND_CHANNEL_DISABLE(state, 4);
        }
    }
}

static void do_volume_env(sound_ctlr_t *state) {
    sound_ve_onclock(&state->s1_envelope);
    sound_ve_onclock(&state->s2_envelope);
    sound_ve_onclock(&state->noise_envelope);
}

static void do_sequencer(sound_ctlr_t *state) {
    int step = (state->sequencer_step + 1) & 0x7;
    switch(step) {
    case 0:
        do_length(state);
        break;
    case 2:
        do_length(state);
        sound_square_onsweep(state);
        break;
    case 4:
        do_length(state);
        break;
    case 6:
        do_length(state);
        sound_square_onsweep(state);
        break;
    case 7:
        do_volume_env(state);
        break;
    default:
        break;
    }

    state->sequencer_step = step;
}

static int16_t do_attenuate(sound_filter_t *f, int16_t sin) {
    int16_t sout = sin - (INT16_MAX * f->charge);
    f->charge = ((sin - sout) / INT16_MAX) * f->rate;
    return sout;
}

static void do_output(sound_ctlr_t *state, int16_t reference_volume) {
    int32_t sample_l = 0;
    int32_t sample_r = 0;

    if (state->master_enable) {
        if (SOUND_CHANNEL_IS_ON(state, 4)) {
            int noise_out = (state->noise_reg & 1)? 0 : 1;
            int16_t ns = noise_out * reference_volume * SOUND_VE_GETVOL(&state->noise_envelope);
            sample_r += ((state->pan_reg & 0x08)? ns : 0);
            sample_l += ((state->pan_reg & 0x80)? ns : 0);
        }

        if (SOUND_CHANNEL_IS_ON(state, 3) && state->wave_play) {
            uint32_t samp = state->wave_pram[state->wave_pread >> 1];
            if (state->wave_pread & 1) {
                samp = samp & 0xF;
            } else {
                samp = (samp >> 4) & 0xF;
            }
            uint32_t rv = samp >> state->wave_volume;
            int16_t ns = ((float)(rv * 2 - 15) / 15.0) * reference_volume;
            sample_r += (state->pan_reg & 0x04)? ns : 0;
            sample_l += (state->pan_reg & 0x40)? ns : 0;
        }

        if (SOUND_CHANNEL_IS_ON(state, 2)) {
            int lv = sound_square_getoutput_ch2(state);
            int16_t ns = lv * reference_volume * SOUND_VE_GETVOL(&state->s2_envelope);
            sample_r += (state->pan_reg & 0x02)? ns : 0;
            sample_l += (state->pan_reg & 0x20)? ns : 0;
        }

        if (SOUND_CHANNEL_IS_ON(state, 1)) {
            int lv = sound_square_getoutput_ch1(state);
            int16_t ns = lv * reference_volume * SOUND_VE_GETVOL(&state->s1_envelope);
            sample_r += (state->pan_reg & 0x01)? ns : 0;
            sample_l += (state->pan_reg & 0x10)? ns : 0;
        }
    }

    state->prod_buffer[state->prod_count++] = do_attenuate(&state->cap_l, (int16_t)(sample_l / 4) * (state->volume_left / 7.0));
    state->prod_buffer[state->prod_count++] = do_attenuate(&state->cap_r, (int16_t)(sample_r / 4) * (state->volume_right / 7.0));

    if (state->prod_count == state->prod_bufsize) {
        state->feed_callback(state->prod_buffer, state->prod_bufsize * sizeof(int16_t), state->feed_context);
        state->prod_count = 0;
    }
}

void sound_init(sound_ctlr_t *state) {
    memset(state, 0, sizeof(sound_ctlr_t));

    state->prod_buffer = calloc(2048, sizeof(int16_t));
    state->prod_bufsize = 2048;
    state->prod_count = 0;

    sound_timer_set_base(&state->sequencer_timer, 2048);
    sound_timer_reset(&state->sequencer_timer);
}

void sound_run_controller(sound_ctlr_t *state, int ncyc) {
    if (sound_timer_tick(&state->sequencer_timer, ncyc)) {
        do_sequencer(state);
    }

    if (SOUND_CHANNEL_IS_ON(state, 4)) {
        sound_noise_onclock(state, ncyc);
    }
    if (SOUND_CHANNEL_IS_ON(state, 3)) {
        // Workaround because the wave channel requires higher precision than
        // we can provide (per https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Wave_Channel ,
        // works out to 2 mhz)
        // So double up every cycle and hope no software uses very high frequencies.
        sound_wave_onclock(state, ncyc * 2);
    }
    if (SOUND_CHANNEL_IS_ON(state, 2)) {
        sound_square_onclock_ch2(state, ncyc);
    }
    if (SOUND_CHANNEL_IS_ON(state, 1)) {
        sound_square_onclock_ch1(state, ncyc);
    }

    if (sound_timer_tick(&state->prod_timer, ncyc)) {
        do_output(state, state->reference_volume);
    }
}

void sound_install_regs(sound_ctlr_t *state, void *target, snd_mmio_add_observer_t reg_func) {
    reg_func(target, 0xFF24, (snd_mmio_write_t)&write_ff24, NULL, (void *)state);
    reg_func(target, 0xFF25, (snd_mmio_write_t)&write_ff25, NULL, (void *)state);
    reg_func(target, 0xFF26, (snd_mmio_write_t)&write_ff26, (snd_mmio_read_t)&read_ff26, (void *)state);

    reg_func(target, 0xFF27, (snd_mmio_write_t)&write_ff2x, NULL, (void *)state);
    reg_func(target, 0xFF28, (snd_mmio_write_t)&write_ff2x, NULL, (void *)state);
    reg_func(target, 0xFF29, (snd_mmio_write_t)&write_ff2x, NULL, (void *)state);
    reg_func(target, 0xFF2A, (snd_mmio_write_t)&write_ff2x, NULL, (void *)state);
    reg_func(target, 0xFF2B, (snd_mmio_write_t)&write_ff2x, NULL, (void *)state);
    reg_func(target, 0xFF2C, (snd_mmio_write_t)&write_ff2x, NULL, (void *)state);
    reg_func(target, 0xFF2D, (snd_mmio_write_t)&write_ff2x, NULL, (void *)state);
    reg_func(target, 0xFF2E, (snd_mmio_write_t)&write_ff2x, NULL, (void *)state);
    reg_func(target, 0xFF2F, (snd_mmio_write_t)&write_ff2x, NULL, (void *)state);

    sound_square_install_regs(state, target, reg_func);
    sound_noise_install_regs(state, target, reg_func);
    sound_wave_install_regs(state, target, reg_func);
}

void sound_set_volume(sound_ctlr_t *state, int16_t volume) {
    state->reference_volume = volume;
    snd_debug(SND_DEBUG_CONTROLLER, "volume: %d", (int)volume);
}

void sound_set_output(sound_ctlr_t *state, int samples_per_second, sound_feed_buffer_t callback, void *context) {
    state->feed_callback = callback;
    state->feed_context = context;

    sound_timer_set_base(&state->prod_timer, 1048576 / samples_per_second);
    sound_timer_reset(&state->prod_timer);

    state->cap_l.rate = powf(0.999958f, (float)4194304 / samples_per_second);
    state->cap_r.rate = powf(0.999958f, (float)4194304 / samples_per_second);

    snd_debug(SND_DEBUG_CONTROLLER, "tickbase: %d", state->prod_timer.timebase);
}