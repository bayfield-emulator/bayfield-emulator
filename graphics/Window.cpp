/* 
WINDOW.CPP
NICK
2019
*/

#include <SDL2/SDL.h>
#include <string>

#include "Window.h"

/* PRIVATE */

void Window::update() {
	SDL_UpdateWindowSurface(window);
}

/*
Overwrite the surface with the stored colour.
*/
void Window::clear() {
	SDL_RenderClear(renderer);
}

/*
Draw the surface to the screen.
*/
void Window::present() {
	SDL_RenderPresent(renderer);
}

/* PUBLIC */

Window::Window(int w, int h, bool visible) {
	window = SDL_CreateWindow("Untitled Window", SDL_WINDOWPOS_UNDEFINED, 
		SDL_WINDOWPOS_UNDEFINED, w, h, (visible) ? SDL_WINDOW_SHOWN : SDL_WINDOW_HIDDEN);

	surface = SDL_GetWindowSurface(window);
	renderer = SDL_GetRenderer(window);
}

Window::~Window() {
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
}

/*
Set the title of the window
*/
void Window::setTitle(std::string title) {
	SDL_SetWindowTitle(window, title.c_str());
}

/*
Return the currently set window title
*/
std::string Window::getTitle() {
	return SDL_GetWindowTitle(window);
}

/*
Get the window's surface's pixels.
Read and write.
*/
void* Window::getPixels() {
	return surface->pixels;
}

/*
Useful for dumping surface to either debug or save the contents.
*/
SDL_Surface* Window::getSurface() {
	return surface;
}

/*
Set the colour. When clear() is called, this is the colour that will be used, for example.
*/
void Window::setColour(int r, int g, int b, int flags) {
	SDL_SetRenderDrawColor(renderer, r, g, b, flags);
}


/*
Hide window
*/
void Window::hide() {
	SDL_HideWindow(window);
}

/*
Show window
*/
void Window::show() {
	SDL_ShowWindow(window);
}

/*
Redraw the contents of the window.
Boolean parameter defines whether screen will be cleared before being redrawn.
*/
void Window::refresh(bool clear) {
	if (clear) this->clear();
	update();
	present();
}
