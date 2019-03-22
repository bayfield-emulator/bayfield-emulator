#ifndef EMUCORE_H
#define EMUCORE_H

#include <stdint.h>

typedef struct cart {
    size_t image_size;
    uint8_t *bank1;
    uint8_t *bankx;
    const uint8_t rom[0];
} cartridge_t;

typedef struct lr35902_regs {
    uint8_t A;
    uint8_t B;
    uint8_t C;
    uint8_t D;
    uint8_t E;
    uint8_t H;
    uint8_t L;
    uint8_t CPSR;
    uint16_t SP;
    uint16_t PC;
} cpu_regs_t;

#define PAIR_HL 0
#define PAIR_BC 1
#define PAIR_DE 2
static inline uint16_t bc_regs_getpairvalue(cpu_regs_t *regs, int pair) {
    switch(pair) {
        case 0: return (regs->H << 8) | regs->L;
        case 1: return (regs->B << 8) | regs->C;
        case 2: return (regs->D << 8) | regs->E;
    }
}
static inline void bc_regs_putpairvalue(cpu_regs_t *regs, int pair, uint16_t value) {
    switch(pair) {
        case 0: regs->H = value >> 8; regs->L = value & 0xFF;
        case 1: regs->B = value >> 8; regs->C = value & 0xFF;
        case 2: regs->D = value >> 8; regs->E = value & 0xFF;
    }
}

#define FLAG_CARRY      (1 << 4)
#define FLAG_HALF_CARRY (1 << 4)
#define FLAG_NEGATIVE   (1 << 4)
#define FLAG_ZERO       (1 << 4)
static inline int bc_regs_test_cpsr_flag(cpu_regs_t *regs, int flag) {
    return !!(regs->CPSR & flag);
}

typedef struct lr35902_mmap {
    const cartridge_t *rom;
    uint8_t *all_ram;
    uint8_t *wram;
    uint8_t *vram;
    uint8_t *extram;
    uint8_t *sprite;
    uint8_t *zpg;
} cpu_mmap_t;

typedef struct lr35902 {
    cpu_regs_t regs;
    cpu_mmap_t mem;
} bc_cpu_t;

bc_cpu_t *bc_cpu_init(void);
void bc_step(bc_cpu_t *cpu);

void bc_mmap_alloc(cpu_mmap_t *target);
void bc_mmap_take_rom(cpu_mmap_t *mmap, cartridge_t *rom);
uint8_t bc_mmap_getvalue(cpu_mmap_t *mmap, uint16_t addr);
void bc_mmap_putvalue(cpu_mmap_t *mmap, uint16_t addr, uint8_t value);

#endif