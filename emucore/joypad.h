#include <stdint.h>

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
	uint8_t direction_state;
	uint8_t button_state;
} joyp_t;

void joyp_init(cpu_mmap_t *mmap, joyp_t *jpad);
void joyp_set(joyp_t *jpad, uint8_t direction, uint8_t buttons);
/* jp_reg is the value of the joypad register in memory at 0xFF00.
   Used to select column.*/
uint8_t joyp_get_state(joyp_t *jpad, uint8_t jp_reg);
void joyp_poll(bc_cput_t *cpu, joyp_t *jpad);