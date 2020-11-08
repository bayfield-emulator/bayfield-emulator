/* Bayfield Main */

#include <iostream>
#include <fstream>
#include <thread>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include <SDL2/SDL.h>

#include "emucore.h"
#include "mbc.h"
#include "GPU.hpp"
#include "Window.hpp"
#include "bayfield.h"
#include "joypad.hpp"
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

void show_simple_error(std::string title, std::string message) {
    title.insert(0, "BAYFIELD: ");
    if (SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.c_str(), message.c_str(), nullptr)) {
        // if message box could not be shown, last ditch console print
        std::cerr << message << std::endl;
    }
    return;
}

int main(int argc, char** args) {

    if (SDL_Init(SDL_INIT_VIDEO)) { // SDL could not initialize
        show_simple_error("SDL INIT FAILURE", "Could not initialize required SDL subsystems.");
        return 1;
    }

    char *rom_path = NULL;

    // Check if the argv provided path is a real file first. OS X will sometimes
    // add args if it was a UI launch.
    if (argc > 1) {
        if (access(args[1], F_OK) == 0) {
            rom_path = strdup(args[1]);
        } else {
            show_simple_error("FILE NOT FOUND", "The specified ROM file does not exist.");
            return 1;
        }
    }

    if (!rom_path && fp_get_user_path(&rom_path)) {
        show_simple_error("NO FILE PROVIDED", "Please provide a ROM file's name.");
        return 1;
    }

    emu_shared_context_t cores;
    memset(&cores, 0, sizeof(emu_shared_context_t));
    init_cores(&cores);
    if (!load_rom(&cores, rom_path)) {
        show_simple_error("ROM LOAD FAILURE", "Couldn't load ROM.");
        return 1;
    }

    if (load_save(&cores, rom_path) != 0) {
        std::cerr << "Couldn't load save, continuing without one." << std::endl;
    }

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
            usleep(100);
            continue;
        }

        switch(e.type) {
        case SDL_KEYDOWN: // key press
        case SDL_KEYUP: // key release
            joyp_poll(cores.cpu, &cores.joypad, &e);
            break;
        case SDL_QUIT: // main window sent close command
            quit = true;
            break;
        default:
            break;
        }
    }

    cores.stop = 1;
    emulator_thread.join();

    if (dump_save(&cores, rom_path) != 0) {
        std::cerr << "Couldn't dump save." << std::endl;
    }
    free(rom_path);

    SDL_Quit();
    return 0;
}