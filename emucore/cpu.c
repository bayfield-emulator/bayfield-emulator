#include <stdlib.h>
#include <string.h>
#include "emucore.h"
#include "emucore_internal.h"

#define MMIO_INT_FLAG_INDEX 0x0F
#define CLOCKS_PER_SEC 4194304
#define CLOCKS_PER_MACH 4

static uint8_t cpu_mmio_unwritable(bc_cpu_t *cpu, uint16_t addr, uint8_t write_val) {
    panic("cpu_mmio_unwritable: program attempted write to readonly MMIO register 0x%04x", addr);
}

static uint8_t cpu_user_get_irqs(bc_cpu_t *cpu, uint16_t addr, uint8_t saved_val) {
    return cpu->irqs;
}

static uint8_t cpu_user_set_irqs(bc_cpu_t *cpu, uint16_t addr, uint8_t write_val) {
    cpu->irqs = write_val;
    return 0;
}

bc_cpu_t *bc_cpu_init(void) {
    bc_cpu_t *ret = malloc(sizeof(bc_cpu_t));
    memset(&ret->regs, 0, sizeof(cpu_regs_t));
    bc_mmap_alloc(&ret->mem);
    ret->mem.cpu = ret;

    bc_mmap_add_mmio_observer(&ret->mem, 0xFF0F, cpu_user_set_irqs, cpu_user_get_irqs);
    bc_mmap_add_mmio_observer(&ret->mem, 0xFF04, cpu_user_set_div, cpu_user_get_div);
    bc_mmap_add_mmio_observer(&ret->mem, 0xFF05, cpu_user_set_tima, cpu_user_get_tima);
    bc_mmap_add_mmio_observer(&ret->mem, 0xFF06, NULL, NULL);
    bc_mmap_add_mmio_observer(&ret->mem, 0xFF07, cpu_user_set_tac, NULL);

    return ret;
}

void bc_request_interrupt(bc_cpu_t *cpu, enum bc_int_flag interrupt) {
    interrupt &= 0x1f;
    cpu->irqs |= interrupt;
} 

void bc_cpu_step(bc_cpu_t *cpu, int ncycles) {
    panic("unimplemented :(");
}

void bc_cpu_reset(bc_cpu_t *cpu) {
    cpu->regs.A = 0x01;
    cpu->regs.CPSR = 0xB0;
    cpu->regs.B = 0x00;
    cpu->regs.C = 0x13;
    cpu->regs.D = 0x00;
    cpu->regs.E = 0xD8;
    cpu->regs.H = 0x01;
    cpu->regs.L = 0x4D;
    cpu->regs.SP = 0xFFFE;
    cpu->regs.PC = 0x0100;
    bc_mmap_putvalue(&cpu->mem, 0xFF05, 0);
    bc_mmap_putvalue(&cpu->mem, 0xFF06, 0); // timer mod: 0
    bc_mmap_putvalue(&cpu->mem, 0xFF07, 0); // timer ctl: 4.096khz

    bc_mmap_putvalue(&cpu->mem, 0xFF10, 0x80); 
    bc_mmap_putvalue(&cpu->mem, 0xFF11, 0xBF); 
    bc_mmap_putvalue(&cpu->mem, 0xFF12, 0xF3); 
    bc_mmap_putvalue(&cpu->mem, 0xFF14, 0xBF); 
    bc_mmap_putvalue(&cpu->mem, 0xFF16, 0x3F); 
    bc_mmap_putvalue(&cpu->mem, 0xFF17, 0x00); 
    bc_mmap_putvalue(&cpu->mem, 0xFF19, 0xBF); 
    bc_mmap_putvalue(&cpu->mem, 0xFF1A, 0x7F); 
    bc_mmap_putvalue(&cpu->mem, 0xFF1B, 0xFF); 
    bc_mmap_putvalue(&cpu->mem, 0xFF1C, 0x9F); 
    bc_mmap_putvalue(&cpu->mem, 0xFF1E, 0xBF); 
    bc_mmap_putvalue(&cpu->mem, 0xFF20, 0xFF); 
    bc_mmap_putvalue(&cpu->mem, 0xFF21, 0x00); 
    bc_mmap_putvalue(&cpu->mem, 0xFF22, 0x00); 
    bc_mmap_putvalue(&cpu->mem, 0xFF23, 0xBF); 
    bc_mmap_putvalue(&cpu->mem, 0xFF24, 0x77); 
    bc_mmap_putvalue(&cpu->mem, 0xFF25, 0xF3); 
    bc_mmap_putvalue(&cpu->mem, 0xFF26, 0xF1); 
}

void bc_cpu_release(bc_cpu_t *cpu) {
    bc_mmap_release(&cpu->mem);
    free(cpu);
}