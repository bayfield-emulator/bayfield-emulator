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
#define debug_assert(expr, msg) do { if (!(expr)) panic("debug_assert:%s, failing expr: %s in %s (%s:%d)", msg, #expr, __func__, __FILE__, __LINE__); } while(0)
#define debug_log(s, ...) (fprintf(stderr, "(debug) in %s (%s:%d):" s "\n", __func__, __FILE__, __LINE__, ##__VA_ARGS__))
#else
#define debug_assert(expr, msg) /**/
#define debug_log(s, ...) /**/
#endif

typedef struct instruction insn_desc_t;
typedef struct cart cartridge_t;
typedef struct lr35902_regs cpu_regs_t;
typedef struct lr35902_mmap cpu_mmap_t;
typedef struct lr35902 bc_cpu_t;
typedef struct mbc_context mbc_context_t;

typedef void (*bc_mbc_write_proc_t)(cpu_mmap_t *mem, uint16_t addr, uint8_t write_val);
typedef void (*bc_extram_write_proc_t)(cpu_mmap_t *mem, uint16_t addr, uint8_t write_val);

typedef struct cart {
    size_t image_size;
    int mbc_type;
    uint8_t *rom;
    uint8_t *extram_base;
    bc_mbc_write_proc_t mbc_handler;
    /* The only reason this exists is because MBC2 carts only have 4 bits available per RAM byte,
     * so we'll mask it on write. There is no read function, we do it directly out of extram. */
    /* We will actually need a read handler if we implement RTC support. */
    bc_extram_write_proc_t extram_handler;
    mbc_context_t *mbc_context;
    uint16_t extram_usable_size;

    uint8_t *extram;
    uint8_t *bank1;
    uint8_t *bankx;
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
typedef uint8_t (*bc_mmio_observe_t)(bc_cpu_t *cpu, void *context, uint16_t addr, uint8_t write_val);
typedef uint8_t (*bc_mmio_fetch_t)(bc_cpu_t *cpu, void *context, uint16_t addr, uint8_t saved_val);

typedef struct lr35902_mmap {
    cartridge_t rom;
    bc_cpu_t *cpu;
    uint8_t *all_ram;
    uint8_t *wram;
    uint8_t *hram;
    uint8_t mmio_storage[128];
    struct cpu_mmio_observer {
        bc_mmio_fetch_t get;
        bc_mmio_observe_t set;
        void *ctx;
    } observers[128];

    /* These exist outside of the internal ram allocation. */
    uint8_t *vram;
    uint8_t *oam;
} cpu_mmap_t;

typedef struct lr35902 {
    cpu_regs_t regs;
    cpu_mmap_t mem;
    uint16_t pc_of_current_instruction;
    const insn_desc_t *current_instruction;
    uint16_t instruction_param;

    /* Stall after executing current instruction, for branches, etc. */
    uint32_t cycles_for_stall;
    uint32_t stalled_cycles;
    /* There are two types of stalls: front stalls, which fill in the time that an
     * instruction would take to execute in the case that the budget given to bc_step
     * isn't enough to actually execute the next instruction. When the stall is finished,
     * the stalled clocks are added back into the budget so that 
     *     (instruction cycles) == stall cycles + budget at the time of stall. 
     * The second type is the back stall, which simulate work that takes place outside an
     * instruction, i.e. the extra cost from jumps (see IMP_call_ret_absolute), and 
     * interrupts (jumping to an ISR address takes 20 cycles).
     * These stalls DO NOT add their time back to the budget, since it was spent doing 
     * "real work". */
    uint32_t stall_counts_towards_budget;
    /* If non-zero, only HRAM is accessible. Decremented automatically every clock. */
    uint32_t dma_lockdown_time;

    /* Clock source for both timer and divider regs.
     * It ticks once every 16 cycles. */
    uint32_t clock_leftover;
    /* Increment DIV register whenever this ticks over 16. */
    uint32_t div_clock;
    /* Increment TIMA register whenever this ticks over tac_freq */
    uint32_t timer_clock;
    /* user configured timer frequency */
    uint32_t tac_freq;

    uint8_t div;
    uint8_t tima;
    uint8_t timer_enable;

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

    panic("read undefined register pair '%d'", pair);
}
static inline void bc_regs_putpairvalue(cpu_regs_t *regs, int pair, uint16_t value) {
    switch(pair) {
        case 0: regs->H = value >> 8; regs->L = value & 0xFF; break;
        case 1: regs->B = value >> 8; regs->C = value & 0xFF; break;
        case 2: regs->D = value >> 8; regs->E = value & 0xFF; break;
        case 3: regs->A = value >> 8; regs->CPSR = value & 0xF0; break;
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
static inline uint16_t bc_convertsignedvalue16(int16_t val) {
    if (val >= 0) {
        return val;
    } else {
        return (uint16_t)((int)val + 65536);
    }
}
static inline int8_t bc_convertunsignedvalue(uint8_t val) {
    if (val > 127) {
        return *(int8_t *)&val;
    }
    return val;
}

/* ==== MEMORY ==================================================================== */

void bc_mmap_alloc(cpu_mmap_t *target);
void bc_mmap_release(cpu_mmap_t *target);

/* Convert a CPU address to a C address. */
uint8_t *bc_mmap_calc(cpu_mmap_t *mem, uint16_t addr);

/* Read/write memory. This is done in the context of CPU code, so MMIO observers will be called. */
uint8_t bc_mmap_getvalue(cpu_mmap_t *mmap, uint16_t addr);
void bc_mmap_putvalue(cpu_mmap_t *mmap, uint16_t addr, uint8_t value);
void bc_mmap_putvalue16(cpu_mmap_t *mmap, uint16_t addr, uint16_t value);

void bc_mmap_putstack16(cpu_mmap_t *mmap, uint16_t value);
uint16_t bc_mmap_popstack16(cpu_mmap_t *mmap);

/* Add an observer for a MMIO register. When the CPU writes to the register specified by addr,
 * write_proc will be called. The returned value is saved by the mmap, and will be passed to read_proc
 * the next time the CPU reads from the mmio reg. 
 * New: The context will also be passed to both callbacks. */
void bc_mmap_add_mmio_observer(cpu_mmap_t *mmap, uint16_t addr,
    bc_mmio_observe_t write_proc, bc_mmio_fetch_t read_proc, void *context);

#ifdef __cplusplus
}
#endif

#endif