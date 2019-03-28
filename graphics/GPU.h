/* 
GPU.H
NICK
2019
*/

#include <cstdint>

#define TILE_W_H 8
#define BUFFER_W_H 32
#define TILE_SIZE 16
#define KB 1024

#ifndef GB_GPU_H
#define GB_GPU_H

class GPU {
	private:
		uint8_t GPU_REG_LCD_CONTROL = 0;
		uint8_t GPU_REG_LCD_STATUS = 0;
		uint8_t GPU_REG_SCROLLY = 0;
		uint8_t GPU_REG_SCROLLX = 0;
		uint8_t GPU_REG_WINDOWY = 0;
		uint8_t GPU_REG_WINDOWX = 0;
		uint8_t GPU_REG_PALETTE_BG = 0xE4;
		uint8_t GPU_REG_PALETTE_S0 = 0xE4;
		uint8_t GPU_REG_PALETTE_S1 = 0xE4;

		uint8_t* VRAM;
		uint8_t* BG_MAP;
		uint8_t* OAM;
		uint32_t* BG_BUFFER;
		uint32_t* WINDOW_BUFFER;
		uint32_t* SPRITE_BUFFER;
		uint8_t* TILES_BG;
		uint8_t* TILES_SPRITES;

		uint32_t* WINDOW_MEMORY;

		const uint32_t PALETTE[4] = {0xFFFFFFFF, 0xFFA8A8A8, 0xFF545454, 0xFF000000};

	public:
		GPU();
		~GPU();

		/* Setup */
		void init(uint32_t* ptr_to_win_memory);

		/* DEBUGGING PURPOSES ONLY */
		void setBgBufferAddress(uint32_t* buf_addr);
		void setSpriteBufferAddress(uint32_t* buf_addr); //not done yet
		void setWindowBufferAddress(uint32_t* buf_addr); //not done yet

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

		/* Set background tile [id] to background map at [x] [y] */
		void set_bg_tile(int x, int y, int id);

		/* Set scroll register values (absolute) */
		void set_scroll(int8_t x, int8_t y);

		/* Render sprites to buffer */
		void draw_sprites(); //not done yet

		/* Render window to buffer */
		void draw_window(); //not done yet		

		/* Render backgroud tiles to buffer */
		void draw_bg();

		/* Take all layers and copy them to the Window surface */
		void render();
};

#endif
