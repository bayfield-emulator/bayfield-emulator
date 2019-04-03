#include <iostream>
#include <fstream>
#include <thread>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "emucore.h"
#include "mbc.h"
#include "GPU.h"
#include "Window.h"
#include "bayfield.h"

SDL_Surface *copy_frame(void) {
    SDL_Surface *ret = SDL_LoadBMP("eframe.bmp");
    if (!ret) {
        ret = SDL_LoadBMP("src/eframe.bmp");
    }
    return ret;
}

int main(int argc, char const *argv[]) {
    //test args
    if (argc < 2) {
        std::cerr << "Please provide a file name" << std::endl;
        return 1;
    }

    SDL_Init(SDL_INIT_VIDEO);

    emu_shared_context_t cores = {0};
    init_cores(&cores);
    if (!load_rom(&cores, argv[1])) {
        std::cerr << "Couldn't load ROM!" << std::endl;
    }

    bool quit = false;

    //set up window
    Window *win = new Window(256, 212);
    win->setTitle(cores.rom_title);
    win->setColour(0);
    win->refresh(true);

    // Draw a frame around the emulator. We make the GPU draw into a separate surface that gets
    // composited onto the window.
    SDL_Surface *wind_buf = win->getSurface();

    SDL_Surface *frame = copy_frame();
    if (frame) {
        SDL_BlitSurface(frame, NULL, wind_buf, NULL);
        SDL_FreeSurface(frame);
    }

    win->refresh(false);
    std::thread emulator_thread(emu_thread_go, &cores);

    SDL_Event e;
    //While application is running
    while (!quit) {
        //Handle events on queue
        if (!SDL_PollEvent(&e)) {
            usleep(1000);
            continue;
        }
            //User requests quit
        if (e.type == SDL_QUIT) { //single window mode
            quit = true;
            cores.stop = 1;
        }
        else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE) { //multi-window mode
            quit = true;
            cores.stop = 1;
        }

        if (e.type == SDL_MOUSEBUTTONDOWN) {
                // queue->push_back(e);
        } else if (e.type == SDL_MOUSEBUTTONUP) {
                // queue->push_back(e);
        }
    }

    cores.stop = 1;
    emulator_thread.join();
    SDL_Quit();

    return 0;
}