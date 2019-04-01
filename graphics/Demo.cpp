/* IMPORTS */
#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <unistd.h>
#include <cstdint>

#include <SDL2/SDL.h>
#include "Window.h"
#include "GPU.h"

#define WINDOW_H 144
#define WINDOW_W 160

using namespace std; //fight me


char TITLE[17];
uint8_t ROM_TYPE;
uint8_t ROM_SIZE;
uint8_t RAM_SIZE;
uint8_t JDM;

SDL_Event e;

int main(int argc, char const *argv[]) {

	SDL_Init(SDL_INIT_VIDEO);

	bool quit = false;

	//test args
	if (argc < 2) {
		cerr << "Please provide a file name" << endl;
		return 1;
	}

	//try to open file
	fstream stream(argv[1], ios::binary);
	stream.open(argv[1], ios::in);

	//test file existence 
	if (!stream.is_open()) {
		cerr << "Could not read file" << endl;
		return 2;
	}

	//assume file is legitimate ROM

	/*Title [16 bytes long from 0x0134]*/
	stream.seekg(0x0134);
	stream.get(TITLE, sizeof(TITLE));

	/*ROM Type [single byte at 0x0147]*/
	stream.seekg(0x0147);
	ROM_SIZE = stream.get();

	/*ROM Size [single byte at 0x0148]*/
	ROM_SIZE = stream.get();

	/*RAM Size [single byte at 0x0149]*/
	RAM_SIZE = stream.get();

	/*Nation of sale [single byte at 0x014A]*/
	stream.seekg(0x014A);
	JDM = !stream.get();

	//rom info
	cout << "   TITLE: " << TITLE << endl;
	cout << "ROM TYPE: " << (uint16_t) ROM_TYPE << endl;
	cout << "ROM SIZE: " << (uint16_t) ROM_SIZE << endl;
	cout << "RAM SIZE: " << (uint16_t) RAM_SIZE << endl;
	cout << "JAPANESE: " << (bool) JDM << endl;

	//set up window
	Window* win = new Window(WINDOW_W, WINDOW_H);
	win->setTitle(string(TITLE));
	win->setColour(255, 255, 255, 0);
	win->refresh(true);

	//create background tile textures
	uint8_t blackBlock[16] = {
	  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
	};

	uint8_t darkgreyBlock[16] = {
	  0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00
	};

	uint8_t lightgreyBlock[16] = {
	  0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF
	};

	uint8_t whiteBlock[16] = {
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	//create and initialize PPU with Window->Surface->Pixels
	GPU* main_gpu = new GPU();
	main_gpu->init((uint32_t *) win->getPixels());

	//set background buffer as our external Surface (for debugging reasons)
	SDL_Surface* bg_buffer = SDL_CreateRGBSurface(0, 256, 256, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	main_gpu->setBgBufferAddress((uint32_t *) bg_buffer->pixels);

	//set background buffer as our external Surface (for debugging reasons)
	SDL_Surface* sprite_buffer = SDL_CreateRGBSurface(0, 256, 256, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	main_gpu->setSpriteBufferAddress((uint32_t *) sprite_buffer->pixels);

	//set background buffer as our external Surface (for debugging reasons)
	SDL_Surface* win_buffer = SDL_CreateRGBSurface(0, 256, 256, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	main_gpu->setWindowBufferAddress((uint32_t *) win_buffer->pixels);

	cout << "First, draw a background of diagonal stripes.\n\tSee bg_buffer.bmp to confirm." << endl;

	//add background tiles to VRAM
	main_gpu->add_bg_tile(0, whiteBlock);
	main_gpu->add_bg_tile(1, lightgreyBlock);
	main_gpu->add_bg_tile(2, darkgreyBlock);
	main_gpu->add_bg_tile(3, blackBlock);

	//map out tiles to create a pattern (just as a demo)
	for (int y = 0; y < 32; y++) {
		for (int x = 0; x < 32; x++) {
			main_gpu->set_bg_tile(x, y, (x+y)%4);
		}
	}

	//draw background to buffer
	main_gpu->draw_bg();

	cout << "Next, draw a small checkerboard pattern with sprites near bottom right.\n\tSee spr_buffer.bmp to confirm." << endl;

	//add sprite tiles to VRAM
	main_gpu->add_sprite_tile(0, whiteBlock);
	main_gpu->add_sprite_tile(1, lightgreyBlock);
	main_gpu->add_sprite_tile(2, darkgreyBlock);
	main_gpu->add_sprite_tile(3, blackBlock);

	//generate some sprite data, add to OAM
	uint8_t id = 0;
	for (int y = 108; y < 140; y+=8) {
		for (int x = 92; x < 156; x+=8) {
			main_gpu->set_sprite_data(id, x + 8, y + 8, ((id) % 4), 0x00);
			id++;
		}
		id++;
	}

	main_gpu->draw_sprites();

	cout << "Then create the window. It's all black with a single white block to help identify offset.\n\tSee win_buffer.bmp to confirm." << endl;

	//draw sprites to buffer
	main_gpu->set_win_pos(16, 16);

	//generate window (all black)
	for (int y = 0; y < 32; y++) {
		for (int x = 0; x < 32; x++) {
			main_gpu->set_window_tile(x, y, 3);
		}
	}

	main_gpu->set_window_tile(3, 3, 0);

	main_gpu->draw_window();

	cout << "Finally, combine all the layers into the correctly sized frame (160 x 144).\n\tSee raw_frame.bmp for final output and to confirm offsets are correct." << endl;

	//copy buffer to window
	//includes scroll register offsets for background
	main_gpu->render();
	win->refresh(false);

	//dump Window contents
	SDL_SaveBMP(win->getSurface(), "raw_frame.bmp");

	//dump background buffer contents
	SDL_SaveBMP(bg_buffer, "bg_buffer.bmp");

	//dump background buffer contents
	SDL_SaveBMP(sprite_buffer, "spr_buffer.bmp");

	//dump background buffer contents
	SDL_SaveBMP(win_buffer, "win_buffer.bmp");

	//While application is running
	while (!quit) {
		//Handle events on queue
		while (SDL_PollEvent(&e) != 0) {
			//User requests quit
			if (e.type == SDL_QUIT) { //single window mode
				quit = true;
			}
			else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE) { //multi-window mode
				quit = true;
			}
			//Mouse button press
			else if (e.type == SDL_MOUSEBUTTONDOWN) {
				// queue->push_back(e);
			}
			//Mouse button release
			else if (e.type == SDL_MOUSEBUTTONUP) {
				// queue->push_back(e);
			}
		}
		usleep(10000);
	}

	SDL_Quit();

	return 0;
}
