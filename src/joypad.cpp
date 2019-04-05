#include <SDL2/SDL.h>
#include <iostream>
#include "emucore.h"
#include "joypad.h"

// 0001
#define JOYPAD_REG_HEAD_BUTTON_SELECTED (0x1 << 4)
// 0010
#define JOYPAD_REG_HEAD_DIRECTIONS_SELECTED (0x2 << 4)

void joyp_init(cpu_mmap_t *mmap, joyp_t *jpad) {
	bc_mmap_add_mmio_observer(mmap, 0xFF00, (bc_mmio_observe_t)joyp_set_sel, (bc_mmio_fetch_t)joyp_get_state, jpad);
	// Note: (0 = pressed, 1 = unpressed)
	jpad->direction_state = 0xF | JOYPAD_REG_HEAD_DIRECTIONS_SELECTED;
	jpad->button_state = 0xF | JOYPAD_REG_HEAD_BUTTON_SELECTED;
}

int joyp_set(joyp_t *jpad, uint8_t direction, uint8_t buttons) {
	int interrupt = 0;
	if (((direction ^ jpad->direction_state) & 0xF) &&
		(jpad->selection & JOYPAD_REG_HEAD_DIRECTIONS_SELECTED) == 0) {
		interrupt = 1;
	}
	if (((buttons ^ jpad->button_state) & 0xF) &&
		(jpad->selection & JOYPAD_REG_HEAD_BUTTON_SELECTED) == 0) {
		interrupt = 1;
	}

	jpad->direction_state = direction | JOYPAD_REG_HEAD_DIRECTIONS_SELECTED;
	jpad->button_state = buttons | JOYPAD_REG_HEAD_BUTTON_SELECTED;

	debug_log("Button state did change: %s %s %s %s      %s %s %s %s INT: %d",
		((direction & 0x04) == 0)? "UP" : "up",
		((direction & 0x08) == 0)? "DOWN" : "down",
		((direction & 0x02) == 0)? "LEFT" : "left",
		((direction & 0x01) == 0)? "RIGHT" : "right",

		((buttons & 0x01) == 0)? "A" : "a",
		((buttons & 0x02) == 0)? "B" : "b",
		((buttons & 0x08) == 0)? "START" : "start",
		((buttons & 0x04) == 0)? "SELECT" : "select",
		interrupt
	);
	return interrupt;
}

uint8_t joyp_get_state(bc_cpu_t *cpu, joyp_t *jpad, uint16_t addr, uint8_t jp_reg) {
	// based on whether bit 4 or bit 5 of jp_reg is 0, return direction or button respectively
	if ((jp_reg & 0x30) == 0x10) return jpad->button_state;
	else return jpad->direction_state;
}

uint8_t joyp_set_sel(bc_cpu_t *cpu, joyp_t *jpad, uint16_t addr, uint8_t jp_reg) {
	// based on whether bit 4 or bit 5 of jp_reg is 0, return direction or button respectively
	jpad->selection = jp_reg & 0x30;
	debug_log("CPU joypad mask is now %x", jp_reg);
	return jp_reg & 0x30;
}

/**
* NOTE: This is a temporary function for the main input loop.
* Still needs to be integrated. It's a little screwy as is.
*/
void joyp_poll(bc_cpu_t *cpu, joyp_t *jpad, SDL_Event *ev) {
	enum bc_int_flag interrupt = IF_JOYPAD;
	// Inverted so 1 = held
	uint8_t dir_acc = (jpad->direction_state & 0xF) ^ 0xF;
	uint8_t but_acc = (jpad->button_state & 0xF) ^ 0xF;

	uint8_t dir_delta = 0;
	uint8_t but_delta = 0;

	if (ev->key.repeat) {
		return;
	}

	switch (ev->key.keysym.sym) {
	case SDLK_RIGHT:
		dir_delta |= 0x01;
		break;
	case SDLK_LEFT:
		dir_delta |= 0x02;
		break;
	case SDLK_UP:
		dir_delta |= 0x04;
		break;
	case SDLK_DOWN:
		dir_delta |= 0x08;
		break;

	case SDLK_z: // i.e. button A
		but_delta |= 0x01;
		break;
	case SDLK_x: // i.e. button B
		but_delta |= 0x02;
		break;
	case SDLK_BACKSPACE: // select
		but_delta |= 0x04;
		break;
	case SDLK_RETURN: // start
		but_delta |= 0x08;
		break;
	}

	// Set/clear changed flag in acc
	if (ev->type == SDL_KEYUP) {
		dir_acc &= ~dir_delta;
		but_acc &= ~but_delta;
	} else {
		dir_acc |= dir_delta;
		but_acc |= but_delta;
	}

	// We need to invert these back, so 0 = down
	if (joyp_set(jpad, dir_acc ^ 0xF, but_acc ^ 0xF)) {
		bc_request_interrupt(cpu, interrupt);
	}
}