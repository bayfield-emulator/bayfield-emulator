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

void init_cores(emu_shared_context_t *ctx) {
    SDL_Surface *check = NULL;
    ctx->draw_buffers[0] = check = SDL_CreateRGBSurface(0, 160, 144, 24, 0xff, 0xff00, 0xff00000, 0);
    if (!check) {
        debug_log("Failed to create draw buffer 0!");
    }
    ctx->draw_buffers[1] = check = SDL_CreateRGBSurface(0, 160, 144, 24, 0xff, 0xff00, 0xff00000, 0);
    if (!check) {
        debug_log("Failed to create draw buffer 1!");
    }
    ctx->drawing_buffer = 0;

    SDL_Rect all = (SDL_Rect){0, 0, 160, 144};
    SDL_FillRect(ctx->draw_buffers[0], &all, 0xff00000);
    SDL_FillRect(ctx->draw_buffers[1], &all, 0x00ff000);

    ctx->gpu = new GPU();
    ctx->gpu->init((uint32_t *)ctx->draw_buffers[0]->pixels);

    // FIXME these are for debugging only?
    uint32_t *gpu_buffers = (uint32_t *)calloc(256 * 256, 12);
    ctx->gpu->setBgBufferAddress(gpu_buffers);
    ctx->gpu->setSpriteBufferAddress(gpu_buffers + (256 * 256));
    ctx->gpu->setWindowBufferAddress(gpu_buffers + (256 * 256 * 2));

    ctx->cpu = bc_cpu_init();
    ctx->cpu->mem.vram = ctx->gpu->get_vram();
    ctx->cpu->mem.oam = (uint8_t *)ctx->gpu->get_oam();
}

void release_cores(emu_shared_context_t *ctx) {
    bc_cpu_release(ctx->cpu);
    delete ctx->gpu;

    SDL_FreeSurface(ctx->draw_buffers[0]);
    SDL_FreeSurface(ctx->draw_buffers[1]);
}

void emu_thread_go(emu_shared_context_t *ctx) {
    bc_cpu_reset(ctx->cpu);
    // only for panic
    global_cpu = ctx->cpu;

    while (!ctx->stop) {
        bc_cpu_step(ctx->cpu, 16);
        ctx->gpu->render(16);
    }
}