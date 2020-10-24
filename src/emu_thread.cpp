#include <iostream>
#include <thread>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "emucore.h"
#include "GPU.h"
#include "Window.h"
#include "bayfield.h"
#include "sound.h"

#define CPU_CLOCKS_PER_SEC 4194304
#define TICKS_PER_SEC 128
#define CYCS_PER_TICK (CPU_CLOCKS_PER_SEC / TICKS_PER_SEC)
#define USEC_PER_SEC 1000000
#define SLEEP_TIME (USEC_PER_SEC / TICKS_PER_SEC)

bc_cpu_t *global_cpu;

const char *convert_cpsr(int cpsr) {
    static char s[5] = {0};
    if (!s[0]) {
        memcpy(s, "ZNHC", 4);
    }
    s[0] = (cpsr & FLAG_ZERO)? 'Z' : 'z';
    s[1] = (cpsr & FLAG_NEGATIVE)? 'N' : 'n';
    s[2] = (cpsr & FLAG_HALF_CARRY)? 'H' : 'h';
    s[3] = (cpsr & FLAG_CARRY)? 'C' : 'c';
    return s;
}

void dump_regs(bc_cpu_t *global_cpu) {
    printf("registers:\n");
    printf("\tA: $%02x (%d)\tB: $%02x (%d)\tC: $%02x (%d)\n",
        global_cpu->regs.A, global_cpu->regs.A,
        global_cpu->regs.B, global_cpu->regs.B,
        global_cpu->regs.C, global_cpu->regs.C);
    printf("\tD: $%02x (%d)\tE: $%02x (%d)\tH: $%02x (%d)\n",
        global_cpu->regs.D, global_cpu->regs.D,
        global_cpu->regs.E, global_cpu->regs.E,
        global_cpu->regs.H, global_cpu->regs.H);
    printf("\tL: $%02x (%d)\n", global_cpu->regs.L, global_cpu->regs.L);
    printf("\tSP: $%04x\tPC: $%04x\tCPSR: $%02x (%s)\n",
        global_cpu->regs.SP, global_cpu->regs.PC, global_cpu->regs.CPSR,
        convert_cpsr(global_cpu->regs.CPSR));
}

void panic(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    printf("\npanic(");
    vprintf(fmt, ap);
    printf(")\n");

    va_end(ap);

    dump_regs(global_cpu);
    exit(1);
}

static void gpu_interrupt_request_VBLANK(emu_shared_context_t *ctx) {
    // debug_log("vblanking now");
    bc_request_interrupt(ctx->cpu, IF_VBLANK);

    ctx->gpu->redraw();
}

static void gpu_interrupt_request_STAT(emu_shared_context_t *ctx) {
    // debug_log("lcds now");
    bc_request_interrupt(ctx->cpu, IF_LCDSTAT);
}

static uint8_t gpu_mmio_write_trampoline(bc_cpu_t *cpu, emu_shared_context_t *ctx, uint16_t addr, uint8_t reg_val) {
    debug_log("GPU MMIO write! %x to %x", reg_val, addr);
    ctx->gpu->set_FF(addr & 0xFF, reg_val);
    return 0;
}

static uint8_t gpu_mmio_read_trampoline(bc_cpu_t *cpu, emu_shared_context_t *ctx, uint16_t addr, uint8_t reg_val) {
    return ctx->gpu->get_FF(addr & 0xFF);
}

static uint8_t do_dma_request(bc_cpu_t *cpu, emu_shared_context_t *ctx, uint16_t addr, uint8_t reg_val) {
    if (reg_val >= 0xF1) {
        debug_log("Dropping invalid DMA request: %x", reg_val);
        return 0;
    }

    debug_log("DMA request from user region %x00-%x9F", reg_val, reg_val);

    if (reg_val >= 0xA0 && reg_val <= 0xBF) {
        if (((reg_val << 8) | 0x9F) - 0xA000 < cpu->mem.rom.extram_usable_size) {
            debug_log("Dropping out-of-bounds extram DMA request: %x", reg_val);
            return 0;
        }
    }

    cpu->dma_lockdown_time = 640;
    // ctx->gpu->set_dma_lockdown(640);
    memcpy(ctx->gpu->get_oam(), bc_mmap_calc(&cpu->mem, reg_val << 8), 0xA0);
    return reg_val;
}

static void feed_audio(int16_t *samples, size_t length, void *context) {
    emu_shared_context_t *ctx = (emu_shared_context_t *)context;
    if (!ctx->audio_device) {
        return;
    }

    SDL_QueueAudio(ctx->audio_device, samples, length);

    uint32_t queue_size = SDL_GetQueuedAudioSize(ctx->audio_device);
    if (queue_size < 4096) {
        fprintf(stderr, "warning: queued audio samples size suspiciously low: %u (bytes), %u (samples)\n",
            queue_size, (uint32_t)(queue_size / 2 / sizeof(int16_t)));
    }
}

void init_cores(emu_shared_context_t *ctx) {
    SDL_Surface *check = NULL;
    ctx->draw_buffers[0] = check = SDL_CreateRGBSurface(0, 160, 144, 32, 0, 0, 0, 0);
    if (!check) {
        debug_log("Failed to create draw buffer 0! reason: %s", SDL_GetError());
    }
    ctx->draw_buffers[1] = check = SDL_CreateRGBSurface(0, 160, 144, 32, 0, 0, 0, 0);
    if (!check) {
        debug_log("Failed to create draw buffer 1! reason: %s", SDL_GetError());
    }
    ctx->drawing_buffer = 0;

    SDL_Rect all = (SDL_Rect){0, 0, 160, 144};
    SDL_FillRect(ctx->draw_buffers[0], &all, 0xff00000);
    SDL_FillRect(ctx->draw_buffers[1], &all, 0x00ff000);

    ctx->gpu = new GPU();
    ctx->gpu->init((uint32_t *)ctx->draw_buffers[0]->pixels);

    ctx->cpu = bc_cpu_init();
    ctx->cpu->mem.vram = ctx->gpu->get_vram();
    ctx->cpu->mem.oam = (uint8_t *)ctx->gpu->get_oam();

    joyp_init(&ctx->cpu->mem, &ctx->joypad);

    bc_mmap_add_mmio_observer(&ctx->cpu->mem, 0xFF40, (bc_mmio_observe_t)gpu_mmio_write_trampoline, (bc_mmio_fetch_t)gpu_mmio_read_trampoline, ctx);
    bc_mmap_add_mmio_observer(&ctx->cpu->mem, 0xFF41, (bc_mmio_observe_t)gpu_mmio_write_trampoline, (bc_mmio_fetch_t)gpu_mmio_read_trampoline, ctx);
    bc_mmap_add_mmio_observer(&ctx->cpu->mem, 0xFF42, (bc_mmio_observe_t)gpu_mmio_write_trampoline, (bc_mmio_fetch_t)gpu_mmio_read_trampoline, ctx);
    bc_mmap_add_mmio_observer(&ctx->cpu->mem, 0xFF43, (bc_mmio_observe_t)gpu_mmio_write_trampoline, (bc_mmio_fetch_t)gpu_mmio_read_trampoline, ctx);
    bc_mmap_add_mmio_observer(&ctx->cpu->mem, 0xFF44, (bc_mmio_observe_t)gpu_mmio_write_trampoline, (bc_mmio_fetch_t)gpu_mmio_read_trampoline, ctx);
    bc_mmap_add_mmio_observer(&ctx->cpu->mem, 0xFF45, (bc_mmio_observe_t)gpu_mmio_write_trampoline, (bc_mmio_fetch_t)gpu_mmio_read_trampoline, ctx);
    bc_mmap_add_mmio_observer(&ctx->cpu->mem, 0xFF47, (bc_mmio_observe_t)gpu_mmio_write_trampoline, (bc_mmio_fetch_t)gpu_mmio_read_trampoline, ctx);
    bc_mmap_add_mmio_observer(&ctx->cpu->mem, 0xFF48, (bc_mmio_observe_t)gpu_mmio_write_trampoline, (bc_mmio_fetch_t)gpu_mmio_read_trampoline, ctx);
    bc_mmap_add_mmio_observer(&ctx->cpu->mem, 0xFF49, (bc_mmio_observe_t)gpu_mmio_write_trampoline, (bc_mmio_fetch_t)gpu_mmio_read_trampoline, ctx);
    bc_mmap_add_mmio_observer(&ctx->cpu->mem, 0xFF4A, (bc_mmio_observe_t)gpu_mmio_write_trampoline, (bc_mmio_fetch_t)gpu_mmio_read_trampoline, ctx);
    bc_mmap_add_mmio_observer(&ctx->cpu->mem, 0xFF4B, (bc_mmio_observe_t)gpu_mmio_write_trampoline, (bc_mmio_fetch_t)gpu_mmio_read_trampoline, ctx);

    bc_mmap_add_mmio_observer(&ctx->cpu->mem, 0xFF46, (bc_mmio_observe_t)do_dma_request, NULL, ctx);

    ctx->gpu->set_intr_context(ctx);
    ctx->gpu->set_intr_V_BLANK((gpu_interrupt_handler_t)gpu_interrupt_request_VBLANK);
    ctx->gpu->set_intr_OAM((gpu_interrupt_handler_t)gpu_interrupt_request_STAT);
    ctx->gpu->set_intr_H_BLANK((gpu_interrupt_handler_t)gpu_interrupt_request_STAT);
    ctx->gpu->set_intr_LYC((gpu_interrupt_handler_t)gpu_interrupt_request_STAT);

    sound_init(&ctx->sound_controller);
    sound_install_regs(&ctx->sound_controller, (void *)&(ctx->cpu->mem), (snd_mmio_add_observer_t)&bc_mmap_add_mmio_observer);
    sound_set_volume(&ctx->sound_controller, 8000);
    sound_set_output(&ctx->sound_controller, AUDIO_SAMPLERATE, (sound_feed_buffer_t)&feed_audio, (void *)ctx);
}

void release_cores(emu_shared_context_t *ctx) {
    bc_cpu_release(ctx->cpu);
    delete ctx->gpu;

    SDL_FreeSurface(ctx->draw_buffers[0]);
    SDL_FreeSurface(ctx->draw_buffers[1]);
}

static void run_hardware(emu_shared_context_t *ctx, int ncycs) {
    debug_assert(ncycs > 0 && (ncycs % 4) == 0, "Need a multiple of 4 clocks");
    // bc_cpu_step(ctx->cpu, ncycs);
    // ctx->gpu->render(ncycs);
    while (ncycs > 0) {
        bc_cpu_step(ctx->cpu, 4);
        ctx->gpu->render(4);
        sound_run_controller(&ctx->sound_controller, 1);
        bc_cpu_step(ctx->cpu, 4);
        ctx->gpu->render(4);
        sound_run_controller(&ctx->sound_controller, 1);
        bc_cpu_step(ctx->cpu, 4);
        ctx->gpu->render(4);
        sound_run_controller(&ctx->sound_controller, 1);
        bc_cpu_step(ctx->cpu, 4);
        ctx->gpu->render(4);
        sound_run_controller(&ctx->sound_controller, 1);
        ncycs -= 16;
    }
}

void emu_thread_go(emu_shared_context_t *ctx) {
    bc_cpu_reset(ctx->cpu);
    // only for panic
    global_cpu = ctx->cpu;

    int frame_stat = 0;
    int frame_counter_epoch = SDL_GetTicks();
    int fps = 0;
    int cps = 0;

    int64_t step_time;
    int64_t step_time2;

    while (1) {
        step_time = usec_since(0);

        run_hardware(ctx, CYCS_PER_TICK);

        if (ctx->stop) {
            break;
        }
        
        cps += CYCS_PER_TICK;
        // usleep(SLEEP_TIME - 500); 

        if (SDL_GetTicks() - frame_counter_epoch >= 1000) {
            frame_counter_epoch = SDL_GetTicks();
            // printf("\t\t\t\t\tFPS: %d\n", fps);
            // printf("\t\t\t\t\tCycs ran: %d\n", cps);
            fps = 0;
            cps = 0;
        }

        if (!frame_stat && (ctx->gpu->get_FF(0x41) & 0x3) == 1) {
            // GPU in vblank
            // printf("GPU entered vblank!\n");
            // ctx->gpu->draw_bg();
            // ctx->gpu->draw_window();
            // ctx->gpu->draw_sprites();
            // swap_buffers(ctx);
            fps++;
            frame_stat = 1;
        } else if (frame_stat && (ctx->gpu->get_FF(0x41) & 0x3) != 1) {
            frame_stat = 0;
        }

        step_time2 = usec_since(step_time);

        // All the code below is for making sure we run exactly at 4mhz.
        int nsteps_left = (1 - ((float)cps / CPU_CLOCKS_PER_SEC)) * TICKS_PER_SEC;
        int time_left = (1000 - (SDL_GetTicks() - frame_counter_epoch)) * 1000;
        // Get time we'll spend sleeping for the rest of this sec,
        // over the number of sleeps we have left
        int sleep_time;
        if (step_time2 * nsteps_left >= time_left) {
            printf("Warning: over budget! %lld > %d\n", step_time2 * nsteps_left, time_left);
            sleep_time = 0;
        } else if (nsteps_left == 0) {
            // Nothing left to do so sleep until the next second
            sleep_time = time_left;
        } else {
            sleep_time = (time_left - (step_time2 * nsteps_left)) / nsteps_left;
        }
        // printf("Step time: %d        Steps left: %d      Sleep time: %d       usec left: %d\n",
        //    step_time2, nsteps_left, sleep_time, time_left);
        usleep(sleep_time); 
    }
}