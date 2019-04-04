#include <stdint.h>
#include "emucore.h"
#pragma once

/**
* direction_state:
*	Bit 0:	Right
*	Bit 1:	Left
*	Bit 2:	Up
*	Bit 3:	Down
*
* button_state:
*	Bit 0:	A
*	Bit 1:	B
*	Bit 2:	Select
*	Bit 3:	Start
*/
typedef struct joypad {
	uint8_t selection;
	uint8_t direction_state;
	uint8_t button_state;
} joyp_t;

void joyp_init(cpu_mmap_t *mmap, joyp_t *jpad);
int joyp_set(joyp_t *jpad, uint8_t direction, uint8_t buttons);
/* jp_reg is the value of the joypad register in memory at 0xFF00.
   Used to select column.*/
uint8_t joyp_get_state(bc_cpu_t *cpu, joyp_t *jpad, uint16_t addr, uint8_t jp_reg);
uint8_t joyp_set_sel(bc_cpu_t *cpu, joyp_t *jpad, uint16_t addr, uint8_t jp_reg);
void joyp_poll(bc_cpu_t *cpu, joyp_t *joypad, SDL_Event *key);