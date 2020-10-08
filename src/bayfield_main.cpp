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
#include "file_picker.h"

#define SCALE 3

SDL_Surface *copy_frame(const char *executable_path) {
    const char *frame_rel = "assets/eframe.bmp";
    SDL_Surface *ret;
    char *path;

    if (executable_path[0] != '/') {
        ret = SDL_LoadBMP(frame_rel);
        return ret;
    }

    // If launched with an absolute path, check for eframe relative to the executable.
    path = new char[strlen(executable_path) + strlen(frame_rel) + 1];
    memcpy(path, executable_path, strlen(executable_path) + 1);
    memcpy(strrchr(path, '/') + 1, frame_rel, strlen(frame_rel) + 1);

    ret = SDL_LoadBMP(path);
    delete[] path;
    return ret;
}

int main(int argc, char** args) {
    char *rom_path = NULL;
    SDL_Init(SDL_INIT_VIDEO);

    // Check if the argv provided path is a real file first. OS X will sometimes
    // add args if it was a UI launch.
    if (argc > 1) {
        if (access(args[1], F_OK) == 0) {
            rom_path = strdup(args[1]);
        } else {
            std::cerr << "The specified file does not exist." << std::endl;
        }
    }

    if (!rom_path && fp_get_user_path(&rom_path)) {
        std::cerr << "Please provide a file name" << std::endl;
        return 1;
    }

    emu_shared_context_t cores;
    memset(&cores, 0, sizeof(emu_shared_context_t));
    init_cores(&cores);
    if (!load_rom(&cores, rom_path)) {
        std::cerr << "Couldn't load ROM!" << std::endl;
        return 1;
    }

    free(rom_path);
    bool quit = false;

    //set up window
    Window *win = new Window(256 * SCALE, 212 * SCALE);
    win->setTitle(cores.rom_title);
    win->setColour(0);
    win->refresh(true);

    // Draw a frame around the emulator. We make the GPU draw into a separate surface that gets
    // composited onto the window.
    SDL_Surface *wind_buf = win->getSurface();

    SDL_Surface *frame = copy_frame(args[0]);
    // Adjust this if you want to put the screen somewhere else
    SDL_Rect gameboy_screen_rect = (SDL_Rect){48 * SCALE, 28 * SCALE, 160 * SCALE, 144 * SCALE};
    SDL_Rect win_size = (SDL_Rect){0, 0, 256 * SCALE, 212 * SCALE};
    if (frame) {
        SDL_BlitScaled(frame, NULL, wind_buf, &win_size);
        SDL_FreeSurface(frame);
    }

    win->refresh(false);
    std::thread emulator_thread(emu_thread_go, &cores);

    uint32_t vtime = 0;
    SDL_Event e;
    //While application is running
    while (!quit) {
        if (SDL_GetTicks() - vtime > 16) {
            // Take the one that isn't currently owned by GPU
            // FIXME we should use a mutex just in case
            SDL_Surface *front_buffer = cores.draw_buffers[cores.drawing_buffer];
            SDL_BlitScaled(front_buffer, NULL, wind_buf, &gameboy_screen_rect);
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