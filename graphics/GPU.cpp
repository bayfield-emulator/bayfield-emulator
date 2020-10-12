/* 
GPU.CPP
NICK
2019
*/

#include "GPU.h"

/* PRIVATE */

void GPU::clear() {
	memset(WINDOW_MEMORY, SCREEN_OFF_COLOUR, (sizeof(uint32_t) * 160 * 144));
}

//draw the sprites to a buffer
void GPU::draw_sprites() {
#if 0
	memset(SPRITE_BUFFER, 0x00, sizeof(uint32_t) * 256 * 256);

	bool MODE_DOUBLE_HEIGHT = (GPU_REG_LCD_CONTROL & SIZE_OBJ);

	for (uint8_t oam_pos = 0; oam_pos < 40; oam_pos++) {

		uint8_t* SPRT_DATA_ADDR = (uint8_t *)(OAM + oam_pos);

		int16_t spr_x = (int16_t) SPRT_DATA_ADDR[1];
		int16_t spr_y = (int16_t) SPRT_DATA_ADDR[0];
		uint8_t spr_tile_id = SPRT_DATA_ADDR[2];
		// bool priority = (SPRT_DATA_ADDR[3] >> 7) & 0x1; /* TODO */
		bool flip_y = (SPRT_DATA_ADDR[3] >> 6) & 0x1;
		bool flip_x = (SPRT_DATA_ADDR[3] >> 5) & 0x1;
		bool palette = (SPRT_DATA_ADDR[3] >> 4) & 0x1;

		uint16_t* TILE_ADDR = (uint16_t *)(TILES_SPRITES + spr_tile_id * 16);

		/* If sprite is on screen, draw it  */
		spr_y -= 16; //convert to top
		spr_x -= 8; //convert to left

		// copy [tile -> buffer] loops
		for (int16_t y = 0; y < 8; y++)	{
			if ((spr_y + y) < 0 || (spr_y + y) > SCREEN_HEIGHT) continue; //skip lines off screen

			for (int16_t x = 0; x < 8; x++) {
				if ((spr_x + x) < 0 || (spr_x + x) > SCREEN_WIDTH) continue; //skip pixels off screen

				// get value of line of pixels, flip if required
				uint16_t A = TILE_ADDR[flip_y ? (7 - y) : y];

				// flip x if required
				uint16_t ux = (flip_x ? (7 - x) : x);

				// shuffle bits about to make sense of GB's storage format
				// 0b0123456789abcdef -> 80, 91, a2, b3, etc.
				uint8_t B = ((A >> (15 - ux) & 0x01) + (((A >> (7 - ux)) & 0x01) << 1));

				// run through palette remapping to correct colour
				uint8_t C = (((palette) ? GPU_REG_PALETTE_S1 : GPU_REG_PALETTE_S0) >> (B << 1)) & 0x03;

				uint16_t buf_pos = (x + spr_x) + (y + spr_y) * 256;

				// look up what colour the saved bits represent, copy it over to buffer
				// if it's colour 0, it's transparent, ignore it
				if (C) SPRITE_BUFFER[buf_pos] = PALETTE[C];
			}
		}
		// }

		/* If in 8x16 mode, draw lower tile */
		if (MODE_DOUBLE_HEIGHT) {
			/* Select next sprite */
			TILE_ADDR = (uint16_t *)(TILES_SPRITES + (spr_tile_id + 1) * 16);
			/* Move target draw area down by one tile */
			spr_y += 8;

			for (int16_t y = 0; y < 8; y++)	{
				if ((spr_y + y) < 0 || (spr_y + y) > SCREEN_HEIGHT) continue; //skip lines off screen

				for (int16_t x = 0; x < 8; x++) {
					if ((spr_x + x) < 0 || (spr_x + x) > SCREEN_WIDTH) continue; //skip pixels off screen

					// get value of line of pixels, flip if required
					uint16_t A = TILE_ADDR[flip_y ? (7 - y) : y];

					// flip x if required
					uint16_t ux = (flip_x ? (7 - x) : x);

					// shuffle bits about to make sense of GB's storage format
					// 0b0123456789abcdef -> 80, 91, a2, b3, etc.
					uint8_t B = ((A >> (15 - ux) & 0x01) + (((A >> (7 - ux)) & 0x01) << 1));

					// run through palette remapping to correct colour
					uint8_t C = (((palette) ? GPU_REG_PALETTE_S1 : GPU_REG_PALETTE_S0) >> (B << 1)) & 0x03;

					uint16_t buf_pos = (x + spr_x) + (y + spr_y) * 256;

					// look up what colour the saved bits represent, copy it over to buffer
					// if it's colour 0, it's transparent, ignore it
					if (C) SPRITE_BUFFER[buf_pos] = PALETTE[C];
				}
			}
		}
	}
#endif
}

//draw the window object to a buffer
void GPU::draw_window() {
#if 0
	// loop over the tile map and look for tiles to be drawn
	for (int tile_map_y = 0; tile_map_y < 32; tile_map_y++) {
		for (int tile_map_x = 0; tile_map_x < 32; tile_map_x++) {

			// this value in the map indicated it would be this tile (if it exists)
			uint8_t *base;
			if (GPU_REG_LCD_CONTROL & SELECT_WINDOW_MAP) {
				base = BG_MAP;
			} else {
				base = WINDOW_MAP; 
			}
			uint8_t tile_id = base[tile_map_x + tile_map_y * 32];

			// not a tile (memset in constructor guarantees this)
			if (tile_id == 0xFF) continue;

			uint16_t* TILE_ADDR;
			if (GPU_REG_LCD_CONTROL & SELECT_BG_WIN_TILE) {
				TILE_ADDR = (uint16_t *) TILES_BG + tile_id * 8;
			} else {
				int8_t *signed_tile_id = (int8_t *)&tile_id;
				TILE_ADDR = (uint16_t *) (TILES_BG + 0x1000) + (*signed_tile_id) * 8;
			}

			// copy [tile -> buffer] loops
			for (int y = 0; y < 8; y++)	{
				for (int x = 0; x < 8; x++) {

					// get value of line of pixels
					uint16_t A = TILE_ADDR[y];

					// shuffle bits about to make sense of GB's storage format
					// 0b0123456789abcdef -> 80, 91, a2, b3, etc.
					uint8_t B = ((A >> (15 - x) & 0x01) + (((A >> (7 - x)) & 0x01) << 1));

					// run through palette remapping to correct colour
					uint8_t C = (GPU_REG_PALETTE_BG >> (B << 1)) & 0x03;

					// absolute x and y coordinates to simplify math
					int abs_x = (x + tile_map_x * 8);
					int abs_y = (y + tile_map_y * 8);
					int buf_pos = abs_x + abs_y * 256;

					// look up what colour the saved bits represent, copy it over to buffer
					WINDOW_BUFFER[buf_pos] = PALETTE[C];
				}
			}
		}
	}
#endif
}

//render tiles from background map, referring to background/shared VRAM for textures
void GPU::draw_bg() {
#if 0
	// loop over the tile map and look for tiles to be drawn
	for (int tile_map_y = 0; tile_map_y < 32; tile_map_y++) {
		for (int tile_map_x = 0; tile_map_x < 32; tile_map_x++) {

			// this value in the map indicated it would be this tile (if it exists)
			uint8_t *base;
			if (GPU_REG_LCD_CONTROL & SELECT_BG_MAP) {
				base = WINDOW_MAP;
			} else {
				base = BG_MAP; 
			}
			uint8_t tile_id = base[tile_map_x + tile_map_y * 32];

			// not a tile (memset in constructor guarantees this)
			if (tile_id == 0xFF) continue;

			uint16_t* TILE_ADDR;
			if (GPU_REG_LCD_CONTROL & SELECT_BG_WIN_TILE) {
				TILE_ADDR = (uint16_t *) TILES_BG + tile_id * 8;
			} else {
				int8_t *signed_tile_id = (int8_t *)&tile_id;
				TILE_ADDR = (uint16_t *) (TILES_BG + 0x1000) + (*signed_tile_id) * 8;
			}

			if (TILE_ADDR == 0x0) continue;

			// copy [tile -> buffer] loops
			for (int y = 0; y < 8; y++)	{
				for (int x = 0; x < 8; x++) {

					// get value of line of pixels
					uint16_t A = TILE_ADDR[y];

					// shuffle bits about to make sense of GB's storage format
					// 0b0123456789abcdef -> 80, 91, a2, b3, etc.
					uint8_t B = ((A >> (15 - x) & 0x01) + (((A >> (7 - x)) & 0x01) << 1));

					// run through palette remapping to correct colour
					uint8_t C = (GPU_REG_PALETTE_BG >> (B << 1)) & 0x03;

					// absolute x and y coordinates to simplify math
					int abs_x = (x + tile_map_x * 8);
					int abs_y = (y + tile_map_y * 8);
					int buf_pos = abs_x + abs_y * 256;

					// look up what colour the saved bits represent, copy it over to buffer
					BG_BUFFER[buf_pos] = PALETTE[C];
				}
			}
		}
	}
#endif
}

/* PUBLIC */

// constructor
GPU::GPU() {
	VRAM = (uint8_t *) malloc(sizeof(uint8_t) * 8 * KB);
	BG_BUFFER = (uint32_t *) malloc(sizeof(uint32_t) * 256 * 256);
	WINDOW_BUFFER = (uint32_t *) malloc(sizeof(uint32_t) * 256 * 256);
	SPRITE_BUFFER = (uint32_t *) malloc(sizeof(uint32_t) * 256 * 256);
	TILES_BG = VRAM;
	TILES_SPRITES = VRAM;
	BG_MAP = VRAM + (6 * KB);
	WINDOW_MAP = VRAM + (7 * KB);
	OAM = (uint32_t *) malloc(sizeof(uint32_t) * 40);
	memset(BG_MAP, 0xFF, (sizeof(uint8_t) * 32 * 32));
	memset(WINDOW_MAP, 0xFF, (sizeof(uint8_t) * 32 * 32));
	memset(OAM, 0x00, (sizeof(uint32_t) * 40));
}

// destructor
GPU::~GPU() {
	free(VRAM);
	free(BG_BUFFER); 
	free(WINDOW_BUFFER); 
	free(SPRITE_BUFFER); 
	free(OAM);
}

//initialize the GPU with a pointer to Window->Surface->Pixels
void GPU::init(uint32_t* ptr_to_win_memory) {
	WINDOW_MEMORY = ptr_to_win_memory;
}

//simplified single call to draw
//calling like this offers guarantees about order, important for sprites layer
void GPU::redraw() {
	draw_bg();
	draw_window();
	draw_sprites();
}

//assemble all buffers and produce final frame
// clocks: Number of cycles to simulate
void GPU::render(uint32_t clocks) {

	uint32_t COMPLETED_CLOCKS = 0;

	while (true) {
		if (!(GPU_REG_LCD_CONTROL & ENABLE_LCD_DISPLAY)) { //display 'turned off' so clear it and return
			clear();
			GPU_REG_LCDCUR_Y = 0;
			POSITION = 0;
			return; //all cycles were completed (GPU idle)
		}
		while (GPU_REG_LCDCUR_Y < 154) {

			if (COMPLETED_CLOCKS + 1 > clocks) { //if clocks is set to expire, save position and return
				POSITION += COMPLETED_CLOCKS;
				POSITION %= 70224;
				return;
			}

			if ((COMPLETED_CLOCKS + POSITION) > 65664) { //ENTER V-BLANK
				GPU_REG_LCD_STATUS = ((GPU_REG_LCD_STATUS & ~FLAG_MODE) | MODE_V_BLANK); //set v-blank mode
				if ((COMPLETED_CLOCKS + POSITION) == 65665) {
					if (F_INTR_VBL != NULL) F_INTR_VBL(INTR_FUNC_CONTEXT);  /* V_BLANK INTERRUPT */
				}
				COMPLETED_CLOCKS++;
				if (!((COMPLETED_CLOCKS + POSITION) % 456)) GPU_REG_LCDCUR_Y++;
				if (GPU_REG_LCDCUR_Y == 154) GPU_REG_LCDCUR_Y = 0;
			}
			else {
				switch ((COMPLETED_CLOCKS + POSITION) % 456) {
					case 0: //before OAM read, check for interrupt on this line
						if ((GPU_REG_LCD_STATUS & INTR_LYC_EQ_LY) && (GPU_REG_LY_CMP == GPU_REG_LCDCUR_Y)) {
							if (F_INTR_LYC != NULL) F_INTR_LYC(INTR_FUNC_CONTEXT); /* LY_CMP INTERRUPT */
						}
					case 1: //OAM READ
						GPU_REG_LCD_STATUS = ((GPU_REG_LCD_STATUS & ~FLAG_MODE) | MODE_OAM_READ); //set OAM read mode

						/* Set up OAM sprite line array */

						{

						uint8_t last_pos = 0;
						
						/* Search through sprite list and find items that fall on the given line */
						for (uint8_t i = 0; i < 10; i++) {

							uint8_t* SPRT_DATA_ADDR = (uint8_t *) OAM;

							for (uint8_t j = last_pos; j < 40; j++) {

								// int16_t spr_x = (int16_t) SPRT_DATA_ADDR[1] - 8;
								int16_t spr_y = (int16_t) SPRT_DATA_ADDR[0] - 16;

								if (spr_y < GPU_REG_LCDCUR_Y && (spr_y + ((GPU_REG_LCD_CONTROL & SIZE_OBJ) ? 16 : 8) > GPU_REG_LCDCUR_Y)) { //if sprite appears on this line
									OAM_SPR_DATA[i] = SPRT_DATA_ADDR[2]; //add tile ID
									last_pos = ++j; //add next position to check
									continue;
								}

								SPRT_DATA_ADDR++;
							}
						}

						}

						COMPLETED_CLOCKS++;
						break;
					case 2 ... 79: //OAM IDLE
						COMPLETED_CLOCKS++;
						break;
					case 80 ... 239: //PIXEL TRANSFER
						GPU_REG_LCD_STATUS = ((GPU_REG_LCD_STATUS & ~FLAG_MODE) | MODE_PIXEL_TF); //set pixel transfer read mode
						
						{

						uint8_t x = ((COMPLETED_CLOCKS + POSITION) % 456) - 80;
						uint8_t y = GPU_REG_LCDCUR_Y;

						// calculate horizontal shift
						uint8_t bg_sx = (x + GPU_REG_SCROLLX);
						uint8_t win_x = (x - (GPU_REG_WINDOWX - 7)); //7 is a hard-coded value

						// calculate vertical shift
						uint8_t bg_sy = (GPU_REG_LCDCUR_Y + GPU_REG_SCROLLY);
						uint8_t win_y = (GPU_REG_LCDCUR_Y - GPU_REG_WINDOWY);

						// check if background and window are enabled
						if (GPU_REG_LCD_CONTROL & ENABLE_BG_WIN_DISPLAY) {

						/* BACKGROUND DRAW */
							uint8_t *base;
							if (GPU_REG_LCD_CONTROL & SELECT_BG_MAP) { // BG map mode
								base = WINDOW_MAP;
							} else {
								base = BG_MAP;
							}
							uint8_t tile_id = base[(bg_sx >> 3) + (bg_sy >> 3) * 32];

							uint16_t* TILE_ADDR;
							if (GPU_REG_LCD_CONTROL & SELECT_BG_WIN_TILE) {
								TILE_ADDR = (uint16_t *) TILES_BG + tile_id * 8;
							} else {
								int8_t *signed_tile_id = (int8_t *)&tile_id;
								TILE_ADDR = (uint16_t *) (TILES_BG + 0x1000) + (*signed_tile_id) * 8;
							}

							// get value of line of pixels
							uint16_t A = TILE_ADDR[bg_sy % 8];

							// shuffle bits
							uint8_t B = ((A >> (15 - (bg_sx % 8)) & 0x01) + (((A >> (7 - (bg_sx % 8))) & 0x01) << 1));

							// palette remapping
							uint8_t C = (GPU_REG_PALETTE_BG >> (B << 1)) & 0x03;

							// write new pixel data
							WINDOW_MEMORY[x + y * SCREEN_WIDTH] = PALETTE[C];


						/* WINDOW DRAW */
							if (GPU_REG_LCD_CONTROL & ENABLE_WINDOW) { // Window draw is enabled
								if (GPU_REG_WINDOWY <= y && (GPU_REG_WINDOWX - 7) <= x) { // Current pixel is in bounds of window

									uint8_t *base;
									if (GPU_REG_LCD_CONTROL & SELECT_WINDOW_MAP) { // Window map mode
										base = WINDOW_MAP;
									} else {
										base = BG_MAP;
									}
									uint8_t tile_id = base[(win_x >> 3) + (win_y >> 3) * 32];


									uint16_t* TILE_ADDR;
									if (GPU_REG_LCD_CONTROL & SELECT_BG_WIN_TILE) {
										TILE_ADDR = (uint16_t *) TILES_BG + tile_id * 8;
									} else {
										int8_t *signed_tile_id = (int8_t *)&tile_id;
										TILE_ADDR = (uint16_t *) (TILES_BG + 0x1000) + (*signed_tile_id) * 8;
									}

									// get value of line of pixels
									uint16_t A = TILE_ADDR[win_y % 8];

									// shuffle bits
									uint8_t B = ((A >> (15 - (win_x % 8)) & 0x01) + (((A >> (7 - (win_x % 8))) & 0x01) << 1));

									// palette remapping
									uint8_t C = (GPU_REG_PALETTE_BG >> (B << 1)) & 0x03;

									// write new pixel data
									WINDOW_MEMORY[x + y * SCREEN_WIDTH] = PALETTE[C];
								}
							}
						}

						/* TODO: SPRITES HERE */

						}

						COMPLETED_CLOCKS++;
						break;
					case 240 ... 251: //Wasted pixel transfer clocks (emulation doesn't need them but real unit does)
						COMPLETED_CLOCKS++;
						break; 
					case 252 ... 454: //H-BLANK
						GPU_REG_LCD_STATUS = ((GPU_REG_LCD_STATUS & ~FLAG_MODE) | MODE_H_BLANK); //set h-blank mode
						if (((COMPLETED_CLOCKS + POSITION) % 456) == 252 && (GPU_REG_LCD_STATUS & INTR_H_BLANK)) {
							if (F_INTR_HBL != NULL) F_INTR_HBL(INTR_FUNC_CONTEXT);  /* H-BLANK INTERRUPT */
						}
						COMPLETED_CLOCKS++;
						break;
					case 455: //LAST COLUMN OF H-BLANK
						GPU_REG_LCDCUR_Y++;
						COMPLETED_CLOCKS++;
						break;
				}
			}
		}
	}
}

uint8_t* GPU::get_vram() {
	return VRAM;
}

uint32_t* GPU::get_oam() {
	return OAM;
}

uint8_t GPU::get_FF(uint8_t reg_no) {
	switch (reg_no) {
		case 0x40:
			return GPU_REG_LCD_CONTROL;
		case 0x41:
			return GPU_REG_LCD_STATUS;
		case 0x42:
			return GPU_REG_SCROLLY;
		case 0x43:
			return GPU_REG_SCROLLX;
		case 0x44:
			return GPU_REG_LCDCUR_Y;
		case 0x45:
			return GPU_REG_LY_CMP;
		case 0x46:
			return GPU_REG_DMA; //should not contain anything, not implemented
		case 0x47:
			return GPU_REG_PALETTE_BG;
		case 0x48:
			return GPU_REG_PALETTE_S0;
		case 0x49:
			return GPU_REG_PALETTE_S1;
		case 0x4A:
			return GPU_REG_WINDOWY;
		case 0x4B:
			return GPU_REG_WINDOWX;
		default:
			return 0xFF;
	}
}

void GPU::set_FF(uint8_t reg_no, uint8_t value) {
	switch (reg_no) {
		case 0x40:
			GPU_REG_LCD_CONTROL = value;
			return;
		case 0x41:
			GPU_REG_LCD_STATUS = value & 0x78; //all read, partial write
			return;
		case 0x42:
			GPU_REG_SCROLLY = value;
			return;
		case 0x43:
			GPU_REG_SCROLLX = value;
			return;
		case 0x44:
			GPU_REG_LCDCUR_Y = 0x00; /* Read only. "Writing will reset the counter." - Pandocs */
			return;
		case 0x45:
			GPU_REG_LY_CMP = value;
			return;
		case 0x46:
			GPU_REG_DMA = value;
			return;
		case 0x47:
			GPU_REG_PALETTE_BG = value;
			return;
		case 0x48:
			GPU_REG_PALETTE_S0 = value;
			return;
		case 0x49:
			GPU_REG_PALETTE_S1 = value;
			return;
		case 0x4A:
			GPU_REG_WINDOWY = value;
			return;
		case 0x4B:
			GPU_REG_WINDOWX = value;
			return;
		default:
			return;
	}
}

void GPU::set_intr_context(void *ctx){
	INTR_FUNC_CONTEXT = ctx;
}

void GPU::set_intr_LYC(gpu_interrupt_handler_t intr){
	F_INTR_LYC = intr;
}

void GPU::set_intr_OAM(gpu_interrupt_handler_t intr){
	F_INTR_OAM = intr;
}

void GPU::set_intr_H_BLANK(gpu_interrupt_handler_t intr){
	F_INTR_HBL = intr;
}

void GPU::set_intr_V_BLANK(gpu_interrupt_handler_t intr){
	F_INTR_VBL = intr;
}

/* TODO */
// Sprite memory offset should be adjustable based on register setting
// As above, but for window
// Check default register values -> this would normally be set by the system before a program begins
// Sprites disappearing near top of screen?
// Sprite draw ordering (lower x priority)
// Sprite priority flag
// 10 sprite-per-line cap enforcement?
