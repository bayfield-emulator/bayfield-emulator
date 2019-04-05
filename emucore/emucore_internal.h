#ifndef EMUCORE_INTERNAL_H
#define EMUCORE_INTERNAL_H

#include "emucore.h"

// The interrupt handler moves PC to these addresses when the respective
// irq is raised
#define ISR_VBLANK 0x40
#define ISR_LCDSTAT 0x48
#define ISR_TIMER 0x50
#define ISR_SERIAL 0x58
#define ISR_JOYPAD 0x60

void bc_timer_add_cycles(bc_cpu_t *cpu, int nclocks);
uint8_t cpu_user_get_div(bc_cpu_t *cpu, void *context, uint16_t addr, uint8_t saved_val);
uint8_t cpu_user_set_div(bc_cpu_t *cpu, void *context, uint16_t addr, uint8_t write_val);
uint8_t cpu_user_get_tima(bc_cpu_t *cpu, void *context, uint16_t addr, uint8_t saved_val);
uint8_t cpu_user_set_tima(bc_cpu_t *cpu, void *context, uint16_t addr, uint8_t write_val);
uint8_t cpu_user_set_tac(bc_cpu_t *cpu, void *context, uint16_t addr, uint8_t write_val);

#define STALL_TYPE_FRONT 0xFFFFFFFF
#define STALL_TYPE_BACK 0x0
static inline void bc_cpu_stall(bc_cpu_t *cpu, int ncycles, uint32_t type) {
    // debug_log("adding stall for %d clocks", ncycles);
    debug_assert(cpu->cycles_for_stall == 0, "can't overlap stalls!");
    cpu->cycles_for_stall = ncycles;
    cpu->stall_counts_towards_budget = type;
    bc_timer_add_cycles(cpu, ncycles);
}

#endif