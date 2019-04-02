#include "emucore.h"

#define CLOCKS_PER_DIV_INC 256

void bc_timer_add_cycles(bc_cpu_t *cpu, int nclocks) {
    uint32_t budget = cpu->clock_leftover + nclocks;
    cpu->clock_leftover = budget % 16;
    // debug_log("adding %d cycles to clock, %d used, %d leftover", nclocks, budget, cpu->clock_leftover);

    // ticks_passed will rarely be more than 1
    uint32_t ticks_passed = budget / 16;
    if (cpu->div_clock + ticks_passed >= 16) {
        cpu->div += (ticks_passed / 16);
        cpu->div_clock = 16 - ticks_passed;
    }

    if (!cpu->timer_enable) {
        cpu->timer_clock = (cpu->timer_clock + ticks_passed) % cpu->tac_freq;
        return;
    }

    if (cpu->timer_clock + ticks_passed >= cpu->tac_freq) {
        uint32_t test = cpu->tima + ((cpu->timer_clock + ticks_passed) / cpu->tac_freq);
        // debug_log("ticks_passed: %d", cpu->timer_clock + ticks_passed);
        // check overflow
        if ((test & 0xff) < cpu->tima) {
            // debug_log("irq flag goin up");
            bc_request_interrupt(cpu, IF_TIMER);
            cpu->tima = cpu->mem.mmio_storage[0x06]; // + (test & 0xff);
            debug_log("TIMA: %u", cpu->tima);
        } else {
            cpu->tima = test & 0xff;
            debug_log("TIMA: %u", cpu->tima);
        }
        cpu->timer_clock = (cpu->timer_clock + ticks_passed) % cpu->tac_freq;
    }
}

uint8_t cpu_user_get_div(bc_cpu_t *cpu, uint16_t addr, uint8_t saved_val) {
    return cpu->div;
}

uint8_t cpu_user_set_div(bc_cpu_t *cpu, uint16_t addr, uint8_t write_val) {
    // DIV always set to 0 on write.
    cpu->div = 0;
    cpu->div_clock = 0;
    cpu->clock_leftover = 0;
    cpu->timer_clock = 0;
    return 0;
}

uint8_t cpu_user_get_tima(bc_cpu_t *cpu, uint16_t addr, uint8_t saved_val) {
    return cpu->tima;
}

uint8_t cpu_user_set_tima(bc_cpu_t *cpu, uint16_t addr, uint8_t write_val) {
    cpu->tima = write_val;
    return 0;
}

uint8_t cpu_user_set_tac(bc_cpu_t *cpu, uint16_t addr, uint8_t write_val) {
    switch (write_val & 0x3) {
        case 0: 
            cpu->tac_freq = 64; break;
        case 1:
            cpu->tac_freq = 1; break;
        case 2:
            cpu->tac_freq = 4; break;
        case 3:
            cpu->tac_freq = 16; break;
    }

    cpu->timer_enable = !!(write_val & 0x4);
    debug_log("load %d to tac", write_val);
    cpu->timer_clock %= cpu->tac_freq;

    return write_val & 0x7;
}