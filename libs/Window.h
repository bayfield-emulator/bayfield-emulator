/* 
WINDOW.H
NICK WILSON
2019
*/

#ifndef WINDOWOBJ_H
#define WINDOWOBJ_H

class Window {

private:
	void update();

	void clear();

	void present();

public:
	SDL_Window* window;
	SDL_Surface* surface;
	SDL_Renderer* renderer;

	Window() {}

	Window(int w, int h, bool visible = true);

	~Window();

	void setTitle(std::string title);

	std::string getTitle();

	void* getPixels();

	SDL_Surface* getSurface();

	void setColour(int r, int g, int b, int flags);

	void hide();

	void show();

	void minimize();

	void refresh(bool clear = true);
};


#endif