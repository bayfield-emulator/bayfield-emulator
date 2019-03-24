#ifndef EMUCORE_INTERNAL_H
#define EMUCORE_INTERNAL_H

#include "emucore.h"

uint8_t cpu_user_get_div(bc_cpu_t *cpu, uint16_t addr, uint8_t saved_val);
uint8_t cpu_user_set_div(bc_cpu_t *cpu, uint16_t addr, uint8_t write_val);
uint8_t cpu_user_get_tima(bc_cpu_t *cpu, uint16_t addr, uint8_t saved_val);
uint8_t cpu_user_set_tima(bc_cpu_t *cpu, uint16_t addr, uint8_t write_val);
uint8_t cpu_user_set_tac(bc_cpu_t *cpu, uint16_t addr, uint8_t write_val);

#endif