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
#include "joypad.h"

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
    // Adjust this if you want to put the screen somewhere else
    SDL_Rect gameboy_screen_rect = (SDL_Rect){48, 28, 160, 144};
    if (frame) {
        SDL_BlitSurface(frame, NULL, wind_buf, NULL);
        SDL_FreeSurface(frame);
    }

    win->refresh(false);
    std::thread emulator_thread(emu_thread_go, &cores);

    uint32_t vtime = 0;
    SDL_Event e;
    //While application is running
    while (!quit) {
        if (SDL_GetTicks() - vtime > 16) {
            // std::cout << "frame time!" << std::endl;
            // Take the one that isn't currently owned by GPU
            // FIXME we should use a mutex just in case
            SDL_Surface *front_buffer = cores.draw_buffers[cores.drawing_buffer? 0 : 1];
            SDL_BlitSurface(front_buffer, NULL, wind_buf, &gameboy_screen_rect);
            win->refresh(false);
            vtime = SDL_GetTicks();
        }

        //Handle events on queue
        if (!SDL_PollEvent(&e)) {
            usleep(1000);
            continue;
        }

        //User requests quit
        switch(e.type) {
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            joyp_poll(cores.cpu, &cores.joypad, &e);
            break;
        case SDL_QUIT:
            quit = true;
            cores.stop = 1;
            break;
        default:
            break;
        }
    }

    cores.stop = 1;
    emulator_thread.join();
    SDL_Quit();

    return 0;
}