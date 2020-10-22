/* 
GPU.H
NICK
2019
*/

#ifndef GB_GPU_H
#define GB_GPU_H

#include <cstdint>	//standard number formats
#include <string.h>	//memset
#include <iostream> //malloc

#define KB 					0x400
#define SCREEN_OFF_COLOUR 	0x94

#define OAM_DELAY 			80
#define LINE_CLOCK_WIDTH	456

/* LCDC BITS */
#define ENABLE_LCD_DISPLAY 		0x80 // Display toggle
#define SELECT_WINDOW_MAP 		0x40 // Which section of memory the window should draw it's mapping from
#define ENABLE_WINDOW 			0x20 // Window toggle
#define SELECT_BG_WIN_TILE 		0x10 // Which section of memory the background and window should take tiles from
#define SELECT_BG_MAP 			0x08 // Which section of memory the background should draw it's mapping from
#define SIZE_OBJ 				0x04 // Sprite mode (8x8 or 8x16)
#define ENABLE_OBJ 				0x02 // Sprite toggle
#define ENABLE_BG_WIN_DISPLAY 	0x01 // Combined background & window toggle

/* LCD STATUS BITS */
#define INTR_LYC_EQ_LY 	0x40 // LY_CMP register check toggle
#define INTR_OAM		0x20 // Interrupt when GPU is in OAM read [MODE 2]
#define INTR_V_BLANK	0x10 // Interrupt when GPU is in V-Blank [MODE 1]
#define INTR_H_BLANK	0x08 // Interrupt when GPU is in H-Blank [MODE 0]
#define FLAG_LYC		0x04 // LYC comparison mode
#define FLAG_MODE		0x03 // GPU mode [0 to 3]

// MODES
#define MODE_OAM_READ	0b10
#define MODE_PIXEL_TF	0b11
#define MODE_H_BLANK	0b00
#define MODE_V_BLANK	0b01

#define SCREEN_WIDTH	160
#define SCREEN_HEIGHT	144


// The type of an interrupt handler function pointer
typedef void (*gpu_interrupt_handler_t)(void *);

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

		/* SDL_WINDOW RENDER DESTINATION */
		uint32_t* WINDOW_MEMORY;

		/* TIMING LIST */
		uint32_t POSITION = 0;

		/* OAM SPRITE LINE DATA */
		uint8_t OAM_SPR_IDX[10] = {};
		int16_t OAM_SPR_XPOS[10] = {};

		/* POINTER TO INTERRUPT ROUTINES */
		gpu_interrupt_handler_t F_INTR_LYC;
		gpu_interrupt_handler_t F_INTR_OAM;
		gpu_interrupt_handler_t F_INTR_HBL;
		gpu_interrupt_handler_t F_INTR_VBL;

		/* Pass this to interrupt functions */
		void *INTR_FUNC_CONTEXT = 0;

		/* FUNCTIONS */ 
		void clear(); // Write value to display to 'turn it off'

		// const uint32_t PALETTE[4] = {0xFFFFFFFF, 0xFFB0B0B0, 0xFF686868, 0xFF000000}; //b&w
		const uint32_t PALETTE[4] = {0xFF879457, 0xFF547659, 0xFF3B584C, 0xFF223A32}; //awful... err... authentic green

	public:

		/* Constructor */
		GPU();

		/* Destructor */
		~GPU();

		/* Setup */
		void init(uint32_t* ptr_to_win_memory);

		/* Redraw all layers */
		void redraw();

		/* Simulate [clocks] cycles of the PPU */
		void render(uint32_t clocks);

		/* Return VRAM */
		uint8_t* get_vram();

		/* Return OAM */
		uint32_t* get_oam();

		/* Get registers */
		uint8_t get_FF(uint8_t reg_no);

		/* Set bits in registers, where permitted */
		void set_FF(uint8_t reg_no, uint8_t value);

		/* Set function to call when respective interrupt is triggered */
		void set_intr_LYC(gpu_interrupt_handler_t intr);
		void set_intr_OAM(gpu_interrupt_handler_t intr);
		void set_intr_H_BLANK(gpu_interrupt_handler_t intr);
		void set_intr_V_BLANK(gpu_interrupt_handler_t intr);
		void set_intr_context(void *ctx);
};

#endif
