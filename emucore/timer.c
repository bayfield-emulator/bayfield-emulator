#include "emucore.h"

#define CLOCKS_PER_DIV_INC 256

void freshen_timer_values(bc_cpu_t *cpu) {
    uint32_t div_delta = (cpu->current_clock - cpu->div_last_clock);
    uint32_t tima_delta = (cpu->current_clock - cpu->timer_last_clock);
    uint32_t steps_div = div_delta / CLOCKS_PER_DIV_INC;

    if (steps_div > 0) {
        cpu->div += steps_div;
        cpu->div_last_clock = cpu->current_clock;
    }
    // is it time to increment TIMA?
    if (tima_delta >= cpu->timer_count) {
        // number of ticks skipped due to staleness
        uint32_t steps_tac = tima_delta / cpu->clocks_per_timer;
        uint8_t tima = cpu->tima;
        uint8_t next_tima = tima + steps_tac;
        // check for overflow - if it did cpu_step should have issued an IRQ before getting here
        if (next_tima < tima) {
            if (tima + steps_tac != 0) {
                panic("freshen_timer_values: we should have issued a timer IRQ %d ticks ago!", tima + steps_tac);
            } else {
                bc_request_interrupt(cpu, IF_TIMER);
            }
        }
        // On overflow, TIMA is set to TMA, which is mmio register FF06.
        cpu->tima = next_tima == 0? cpu->mem.mmio_storage[0x06] : next_tima;
        cpu->timer_count = cpu->clocks_per_timer - (tima_delta - cpu->timer_count);
    } else {
        cpu->timer_count -= tima_delta;
    }

    cpu->timer_last_clock = cpu->current_clock;
}

uint8_t cpu_user_get_div(bc_cpu_t *cpu, uint16_t addr, uint8_t saved_val) {
    freshen_timer_values(cpu);
    return cpu->div;
}

uint8_t cpu_user_set_div(bc_cpu_t *cpu, uint16_t addr, uint8_t write_val) {
    // DIV always set to 0 on write.
    cpu->div = 0;
    cpu->div_last_clock = cpu->current_clock;
    cpu->timer_count = cpu->clocks_per_timer;
    cpu->timer_last_clock = cpu->current_clock;
    return 0;
}

uint8_t cpu_user_get_tima(bc_cpu_t *cpu, uint16_t addr, uint8_t saved_val) {
    freshen_timer_values(cpu);
    return cpu->tima;
}

uint8_t cpu_user_set_tima(bc_cpu_t *cpu, uint16_t addr, uint8_t write_val) {
    cpu->tima = write_val;
    return 0;
}

uint8_t cpu_user_set_tac(bc_cpu_t *cpu, uint16_t addr, uint8_t write_val) {
    switch (write_val & 0x3) {
        case 0: 
            cpu->clocks_per_timer = 1024; break;
        case 1:
            cpu->clocks_per_timer = 16; break;
        case 2:
            cpu->clocks_per_timer = 64; break;
        case 3:
            cpu->clocks_per_timer = 256; break;
        default:
            panic("should never get here");
    }
    freshen_timer_values(cpu);
    return cpu->tima;
}