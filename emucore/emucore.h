#ifndef EMUCORE_H
#define EMUCORE_H

// This is the only public header for emucore, and the only one safe to use with C++.
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>

#if DEBUG
#define debug_assert(expr) do { if (!(expr)) panic("debug_assert:%s", #expr) } while(0)
#define debug_log(s, ...) (printf("(debug) in %s (%s:%d):" s "\n", __func__, __FILE__, __LINE__, ##__VA_ARGS__))
#else
#define debug_assert(expr) /**/
#define debug_log(s, ...) /**/
#endif

typedef struct instruction insn_desc_t;
typedef struct cart cartridge_t;
typedef struct lr35902_regs cpu_regs_t;
typedef struct lr35902_mmap cpu_mmap_t;
typedef struct lr35902 bc_cpu_t;

typedef struct cart {
    size_t image_size;
    uint8_t *bank1;
    uint8_t *bankx;
    uint8_t rom[0];
} cartridge_t;

typedef struct lr35902_regs {
    uint8_t B;
    uint8_t C;
    uint8_t D;
    uint8_t E;
    uint8_t H;
    uint8_t L;
    uint8_t A;
    uint8_t CPSR;
    uint16_t SP;
    uint16_t PC;
} cpu_regs_t;

/* write_val: value given by program code - you can return a different value to save to the register. */
typedef uint8_t (*bc_mmio_observe_t)(bc_cpu_t *cpu, uint16_t addr, uint8_t write_val);
typedef uint8_t (*bc_mmio_fetch_t)(bc_cpu_t *cpu, uint16_t addr, uint8_t saved_val);

typedef struct lr35902_mmap {
    bc_cpu_t *cpu;
    cartridge_t *rom;
    uint8_t *all_ram;
    uint8_t *wram;
    uint8_t *vram;
    uint8_t *extram;
    uint8_t *sprite;
    uint8_t *zpg;
    uint8_t mmio_storage[128];
    struct cpu_mmio_observer {
        bc_mmio_fetch_t get;
        bc_mmio_observe_t set;
    } observers[128];
} cpu_mmap_t;

typedef struct lr35902 {
    cpu_regs_t regs;
    cpu_mmap_t mem;
    const insn_desc_t *current_instruction;
    uint16_t instruction_param;
    /* Cycles leftover for current ISR.
     * It takes 5 machine cycles to start executing an ISR. */
    uint32_t cycles_for_irq;
    /* Cycles leftover for current instruction. */
    uint32_t cycles_for_insn;
    /* Stall after executing current instruction, for branches, etc. */
    uint32_t cycles_for_stall;
    uint32_t stalled_cycles;

    uint32_t current_clock;
    uint32_t timer_last_clock;
    uint32_t div_last_clock;
    uint32_t clocks_per_timer;
    uint32_t timer_count;
    uint8_t div;
    uint8_t tima;

    /* Global halt. */
    uint8_t stop;
    /* soft halt: when the HALT instruction is executed, no processing is done until the next irq */
    uint8_t halt;
    uint8_t irqs;
    /* Controls the interrupts that can be delivered, the top bit (unused on GB) is the global enable bit */
    uint8_t irq_mask;
} bc_cpu_t;

/* ==== CPU ======================================================================= */

extern void panic(const char *, ...) __attribute__((noreturn));

bc_cpu_t *bc_cpu_init(void);
void bc_cpu_release(bc_cpu_t *target);
/* Run the cpu for the specified number of machine cycles.
   FIXME: maybe should be clock cycles, since all other CPU code is based around clocks */
void bc_cpu_step(bc_cpu_t *cpu, int ncycles);
/* Reset the system, including regs and mmap */
void bc_cpu_reset(bc_cpu_t *cpu);

enum bc_int_flag { IF_VBLANK = 1, IF_LCDSTAT = 2, IF_TIMER = 4, IF_SERIAL = 8, IF_JOYPAD = 16, IF_MASTER = 0x80 };
/* Request an interrupt. */
void bc_request_interrupt(bc_cpu_t *cpu, enum bc_int_flag interrupt);

#define PAIR_HL 0
#define PAIR_BC 1
#define PAIR_DE 2
#define PAIR_AF 3
static inline uint16_t bc_regs_getpairvalue(cpu_regs_t *regs, int pair) {
    switch(pair) {
        case 0: return (regs->H << 8) | regs->L;
        case 1: return (regs->B << 8) | regs->C;
        case 2: return (regs->D << 8) | regs->E;
        case 3: return (regs->A << 8) | regs->CPSR;
    }

    panic("Write to undefined register pair '%d'", pair);
}
static inline void bc_regs_putpairvalue(cpu_regs_t *regs, int pair, uint16_t value) {
    switch(pair) {
        case 0: regs->H = value >> 8; regs->L = value & 0xFF; break;
        case 1: regs->B = value >> 8; regs->C = value & 0xFF; break;
        case 2: regs->D = value >> 8; regs->E = value & 0xFF; break;
        case 3: regs->A = value >> 8; regs->CPSR = value & 0xFF; break;
    }
}

#define FLAG_CARRY      (1 << 4)
#define FLAG_HALF_CARRY (1 << 5)
#define FLAG_NEGATIVE   (1 << 6)
#define FLAG_ZERO       (1 << 7)
static inline int bc_regs_test_cpsr_flag(cpu_regs_t *regs, int flag) {
    return !!(regs->CPSR & flag);
}

static inline uint8_t bc_convertsignedvalue(int8_t val) {
    if (val >= 0) {
        return val;
    } else {
        return (uint8_t)((int)val + 256);
    }
}
static inline int bc_convertunsignedvalue(uint8_t val) {
    if (val > 127) {
        return *(int8_t *)&val;
    }
    return val;
}

/* ==== MEMORY ==================================================================== */

void bc_mmap_alloc(cpu_mmap_t *target);
void bc_mmap_release(cpu_mmap_t *target);
/* The mmap takes ownership of the passed rom. If you want to free it, call bc_mmap_release_rom first. */
void bc_mmap_take_rom(cpu_mmap_t *mmap, cartridge_t *rom);
void bc_mmap_release_rom(cpu_mmap_t *mmap, cartridge_t *rom);

/* Read/write memory. This is done in the context of CPU code, so MMIO observers will be called. */
uint8_t bc_mmap_getvalue(cpu_mmap_t *mmap, uint16_t addr);
void bc_mmap_putvalue(cpu_mmap_t *mmap, uint16_t addr, uint8_t value);
void bc_mmap_putvalue16(cpu_mmap_t *mmap, uint16_t addr, uint16_t value);

void bc_mmap_putstack16(cpu_mmap_t *mmap, uint16_t value);
uint16_t bc_mmap_popstack16(cpu_mmap_t *mmap);

/* Add an observer for a MMIO register. When the CPU writes to the register specified by addr,
 * write_proc will be called. The returned value is saved by the mmap, and will be passed to read_proc
 * the next time the CPU reads from the mmio reg. */
void bc_mmap_add_mmio_observer(cpu_mmap_t *mmap, uint16_t addr, bc_mmio_observe_t write_proc, bc_mmio_fetch_t read_proc);

#ifdef __cplusplus
}
#endif

#endif