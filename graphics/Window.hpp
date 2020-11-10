/* 
WINDOW.HPP
NICK
2019
*/

#include <SDL2/SDL.h>	//SDL
#include <cstdint>		//standard number formats
#include <string>		//string type
#include <string.h>		//memset

#ifndef WINDOWOBJ_H
#define WINDOWOBJ_H

class Window {

private:
	void update();

	void clear();

	uint8_t colour = 0;

	int width, height;

public:
	SDL_Window* window;
	SDL_Surface* surface;

	Window() {}

	Window(int w, int h, bool visible = true);

	~Window();

	void setTitle(std::string title);

	std::string getTitle();

	void* getPixels();

	SDL_Surface* getSurface();

	void setColour(uint8_t s);

	void hide();

	void show();

	void minimize();

	void refresh(bool clear = true);
};


#endif
