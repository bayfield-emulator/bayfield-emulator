/* 
GPU.H
NICK
2019
*/

#include <cstdint>	//standard number formats
#include <string.h>	//memset
#include <iostream> //malloc

#define BUFFER_W_H 			0x20
#define TILE_SIZE 			0x10
#define KB 					0x400
#define SCREEN_OFF_COLOUR 	0xFFDEDEDE

#define OAM_DELAY 		80
#define TRANSFER_DELAY 	172
#define H_BLANK_DELAY 	204
#define V_BLANK_DELAY 	4560

/* LCDC BITS */
#define ENABLE_LCD_DISPLAY 		0x80 // Display toggle
#define SELECT_WINDOW_MAP 		0x40 // Which section of memory the window should draw it's mapping from
#define ENABLE_WINDOW 			0x20 // Window toggle
#define SELECT_BG_WIN_TILE 		0x10 // Which section of memory the background and window should take tiles from
#define SELECT_BG_MAP 			0x08 // Which section of memory the background should draw it's mapping from
#define SIZE_OBJ 				0x04 // Sprite mode (8x8 or 8x16)
#define ENABLE_OBJ 				0x02 // Sprite toggle
#define ENABLE_BG_WIN_DISPLAY 	0x01 // Combined background & window toggle

/* TODO: 
	BIT 6 - Might be required
	BIT 4 - Might be required
	BIT 3 - Might be required
	BIT 2 - Unlikely to be required
*/

/* LCD STATUS BITS */
#define INTR_LYC_EQ_LY 	0x40 // LY_CMP register check toggle
#define INTR_OAM		0x20 // Interrupt when GPU is in OAM read [MODE 2]
#define INTR_V_BLANK	0x10 // Interrupt when GPU is in V-Blank [MODE 1]
#define INTR_H_BLANK	0x08 // Interrupt when GPU is in H-Blank [MODE 0]
#define FLAG_LYC		0x04 // LYC compairison mode
#define FLAG_MODE		0x03 // GPU mode [0 to 3]

// MODES
#define MODE_OAM_READ	0b10
#define MODE_PIXEL_TF	0b11
#define MODE_H_BLANK	0b00
#define MODE_V_BLANK	0b01

/* TODO:
	ALL BITS
*/

#ifndef GB_GPU_H
#define GB_GPU_H

class GPU {
	private:

		/* REGISTERS */
		uint8_t GPU_REG_LCD_CONTROL = 0xA3;	// General screen-related values
		uint8_t GPU_REG_LCD_STATUS = 0;		// Screen-related interrupt values
		uint8_t GPU_REG_SCROLLY = 0;		// Vertical screen offset
		uint8_t GPU_REG_SCROLLX = 0;		// Horizontal screen offset
		uint8_t GPU_REG_LCDCUR_Y = 0;		// Line the GPU is currently scanning out to LCD
		uint8_t GPU_REG_LY_CMP = 0;			// Value to compare against LCDCUR_Y for interrupt
		uint8_t GPU_REG_DMA = 0;			// Data transfer related
		uint8_t GPU_REG_PALETTE_BG = 0xE4;	// Background colour palette
		uint8_t GPU_REG_PALETTE_S0 = 0xE4;	// Sprite palette [0]
		uint8_t GPU_REG_PALETTE_S1 = 0xE4;	// Sprite palette [1]
		uint8_t GPU_REG_WINDOWY = 0;		// 'Window' object vertical position
		uint8_t GPU_REG_WINDOWX = 0;		// 'Window' object horizontal position

		/* REAL 8KB VRAM SECTIONS */
		uint8_t* VRAM;				// Start of VRAM
		uint8_t* TILES_BG;			// Start of background tile data
		uint8_t* TILES_SPRITES;		// Start of sprite tile data
		uint8_t* BG_MAP;			// Start of background map
		uint8_t* WINDOW_MAP;		// Start of window map
		uint32_t* OAM; 				// Start of OAM (sprite metadata)

		/* BUFFERS */
		uint32_t* BG_BUFFER;
		uint32_t* WINDOW_BUFFER;
		uint32_t* SPRITE_BUFFER;

		/* SDL_WINDOW RENDER DESTINATION */
		uint32_t* WINDOW_MEMORY;

		/* TIMING LIST */
		uint8_t SPRITE_DELAY[144];
		uint32_t POSITION = 0;

		/* FUNCTIONS */ 
		void clear(); // Write value to display to 'turn it off'

		const uint32_t PALETTE[4] = {0xFFFFFFFF, 0xFFA8A8A8, 0xFF545454, 0xFF000000};

	public:

		/* Contructor */
		GPU();

		/* Destructor */
		~GPU();

		/* Setup */
		void init(uint32_t* ptr_to_win_memory);

		/* Change the colours assigned to the 2-bit background values */
		void remap_bg_colours(uint8_t map);

		/* Change the colours assigned to 2-bit sprite values in the first sprite value */
		void remap_s0_colours(uint8_t map);
		
		/* Change the colours assigned to 2-bit sprite values in the second sprite value */
		void remap_s1_colours(uint8_t map);

		/* Copy input array to GPU tile memory */
		void add_bg_tile(int id, uint8_t* tile);

		/* Copy input array to GPU tile memory */
		void add_sprite_tile(int id, uint8_t* sprite);

		/* Set object sprite, x, y, flip, palette and priority */
		void set_sprite_data(uint8_t pos, uint8_t x, uint8_t y, uint8_t id, uint8_t misc);

		/* Set background tile [id] to background map at [x] [y] */
		void set_bg_tile(int x, int y, int id);

		/* Set background tile [id] to background map at [x] [y] */
		void set_window_tile(int x, int y, int id);

		/* Set scroll register values (absolute) */
		void set_scroll(int8_t x, int8_t y);

		/* Set the window's coordinates */
		void set_win_pos(uint8_t x, uint8_t y);

		/* Render sprites to buffer */
		void draw_sprites(); //not done yet

		/* Render window to buffer */
		void draw_window(); //not done yet		

		/* Render backgroud tiles to buffer */
		void draw_bg();

		/* Simulate [clocks] cycles of the PPU*/
		void render(uint32_t clocks);

		/* Return VRAM */
		uint8_t* get_vram();

		/* Return OAM */
		uint32_t* get_oam();

		/* Return LCD control register */
		uint8_t* get_lcdc();

		/* Return LCD status register */
		/* 
		WARNING: DO NOT WRITE TO BITS 0, 1 AND 2
		THEY ARE READ ONLY. I'M TRUSTING YOU HERE.
		*/
		uint8_t* get_lcds();

		/* Set y position for line interrupt */
		void set_ly_cmp(uint8_t pos);

		/* Send address for DMA memory transfer */
		void init_DMA(uint8_t* addr);
};

#endif
