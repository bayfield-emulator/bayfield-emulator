#ifndef BAYFIELD_H
#define BAYFIELD_H

#include <SDL2/SDL.h>
#include "emucore.h"
#include "joypad.h"
#include "GPU.h"

typedef struct {
    char rom_title[16];
    bc_cpu_t *cpu;
    GPU *gpu;
    joyp_t joypad;
    // Ideally, the GPU will tell us when it finishes a frame,
    // and we'll change the buffer in use so the main thread can always pull
    // a fully rendered image.
    SDL_Surface *draw_buffers[2];
    // index of buffer in use
    int drawing_buffer;
    int stop;
} emu_shared_context_t;

// FUNCTIONS IN emu_thread.cpp

void emu_thread_go(emu_shared_context_t *ctx);
void init_cores(emu_shared_context_t *ctx);
void release_cores(emu_shared_context_t *ctx);

// FUNCTIONS IN rom_utils.cpp

// Sets up the rom part of the cpu's memory map, and MBC stuff if needed
bool load_rom(emu_shared_context_t *ctx, const char *filename);

// FUNCTIONS IN clock.cpp

// Compute the microseconds that passed between now and usec. 
// 1 second = 1 million usec
uint32_t usec_since(uint32_t usec);

#endif