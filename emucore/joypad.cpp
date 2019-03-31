#include <SDL.h>
#include <iostream>
#include "emucore.h"
#include "joypad.h"

void joyp_reset(joyp_t *jpad) {
	// Note: (0 = pressed, 1 = unpressed)
	jpad->direction_state = 0xFF;
	jpad->button_state = 0xFF;
}

void joyp_set(joyp_t *jpad, uint8_t direction, uint8_t buttons){
	jpad->direction_state = direction;
	jpad->button_state= buttons;
}

uint8_t joyp_get_state(joyp_t *jpad, uint8_t jp_reg) {
	// based on whether bit 4 or bit 5 of jp_reg is 0, return direction or button respectively
	if ((jp_reg & 0x30) == 0x10) return jpad->button_state;
	else return jpad->direction_state;
}

/**
* NOTE: This is a temporary function for the main input loop.
* Still needs to be integrated. It's a little screwy as is.
*/
void joyp_poll(bc_cput_t *cpu, joyp_t *jpad) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cout << "SDL Initialization Error" << SDL_GetError() << std::endl;
		return;
	}

	bool isRunning = true;
	SDL_Event ev;
	enum bc_int_flag interrupt = IF_JOYPAD;
	uint8_t dir_acc = 0x00;
	uint8_t but_acc = 0x00;

	while (isRunning) {
		while (SDL_PollEvent(&ev)) {
			if (ev.type == SDL_QUIT) isRunning = false;
			switch (ev.type) {
			case SDL_KEYDOWN:
				switch (ev.key.keysym.sym) {
				case SDLK_RIGHT:
					dir_acc |= 0x01;
					break;
				case SDLK_LEFT:
					dir_acc |= 0x02;
					break;
				case SDLK_UP:
					dir_acc |= 0x04;
					break;
				case SDLK_DOWN:
					dir_acc |= 0x08;
					break;

				case SDLK_z: // i.e. button A
					but_acc |= 0x01;
					break;
				case SDLK_x: // i.e. button B
					but_acc |= 0x02;
					break;
				case SDLK_BACKSPACE: // select
					but_acc |= 0x04;
					break;
				case SDLK_RETURN: // start
					but_acc |= 0x08;
					break;
				}

				bc_request_interrupt(cpu, interrupt);
				break;
			case SDL_KEYUP:
				dir_acc = 0x00;
				but_acc = 0x00;
				break;
			}
			joyp_set(jpad, dir_acc, but_acc);
		}
	}
	return;
}