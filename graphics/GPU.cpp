/* 
GPU.CPP
NICK
2019
*/

#include "GPU.h"

#include <iostream>
#include <string.h>

/* PRIVATE */


/* PUBLIC */

// constructor
GPU::GPU() {
	VRAM = (uint8_t *) malloc(sizeof(uint8_t) * 8 * KB);
	BG_MAP = (uint8_t *) malloc(sizeof(uint8_t) * 256 * 256);
	memset(BG_MAP, 0xFF, (sizeof(uint8_t) * 256 * 256));
	BUFFER = (uint32_t *) malloc(sizeof(uint32_t) * 256 * 256);
	TILES_BG = VRAM;
	TILES_SPRITES = VRAM + (4 * KB); //TODO: offset should be adjustable based on register setting
}

// destructor
GPU::~GPU() {
	free(VRAM);
	free(BG_MAP);
}

// initialize the GPU with a pointer to Window->Surface->Pixels
void GPU::init(uint32_t* ptr_to_win_memory) {
	WINDOW_MEMORY = ptr_to_win_memory;
}

// add a tile to the background area of VRAM
void GPU::add_bg_tile(int id, uint8_t* tile) {
	uint8_t* dest = TILES_BG + id * TILE_SIZE;
	memcpy((void *) dest, (void *) tile, TILE_SIZE);
	for (int i = 0; i < 16; i++) {
	}
}

//add a tile to the sprite area of VRAM
void GPU::add_sprite_tile(int id, uint8_t* sprite) {
	uint8_t* dest = TILES_SPRITES + id * TILE_SIZE;
	memcpy((void *) dest, (void *) sprite, TILE_SIZE);
	for (int i = 0; i < 16; i++) {
	}
}

//set an area in backgroud to render as tile [id] from backgroud/shared VRAM
void GPU::set_bg_tile(int x, int y, int id) {
	BG_MAP[x + y * TILE_SIZE/2] = id;
}

void GPU::draw_sprites() {
	return;
}

//render tiles from background map, referring to background/shared VRAM for textures
void GPU::draw_bg() {
	return;
}

void GPU::render() {
	return;
}

