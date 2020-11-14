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

    int PROGRAM_SCALE = 3;
    bool DRAW_FRAME = true;
    int GPU_P = 0;

    if (SDL_Init(SDL_INIT_VIDEO)) { // SDL could not initialize
        show_simple_error("SDL INIT FAILURE", "Could not initialize required SDL subsystems.");
        return 1;
    }

    char *rom_path = NULL;

    // args parsing
    for (int i = 1; i < argc; i++) {
        if (args[i][0] == '-') {
            switch (args[i][1]) {
                case 'v': //version
                    std::cout << "BAYFIELD EMULATOR v1.2.0" << std::endl;
                    return 0;
                case 's': //scale
                    if (i + 1 < argc) {
                        int val = atoi(args[i+1]);
                        PROGRAM_SCALE = ((val < 1) ? PROGRAM_SCALE : val);
                        i++;
                    }
                    break;
                case 'p': //palette
                    if (i + 1 < argc) {
                        GPU_P = atoi(args[i+1]);
                        i++;
                    }
                    break;
                case 'f': //border
                    DRAW_FRAME = false;
                    break;
            }
        }
        else if (args[i][0] == '?') {
            std::cout << "HELP MESSAGE HERE" << std::endl;
            return 0;
        }
        else {
            if (access(args[i], F_OK) == 0) {
                rom_path = strdup(args[i]);
            } else {
                show_simple_error("FILE NOT FOUND", "The specified ROM file does not exist.");
                return 1;
            }
        }
    }

    if (!rom_path && fp_get_user_path(&rom_path)) {
        show_simple_error("NO FILE PROVIDED", "Please provide a ROM file's name.");
        return 1;
    }

    emu_shared_context_t cores;
    memset(&cores, 0, sizeof(emu_shared_context_t));
    init_cores(&cores);

    switch (load_rom(&cores, rom_path)) {
        case ROM_OK:
            break;
        case ROM_FAIL_VALIDATION:
            show_simple_error("ROM ERROR", "ROM failed to validate. Is it intact and a real ROM?");
            return 1;
        case ROM_FAIL_FULL_VALIDATION:
            show_simple_error("NON-FATAL ROM ERROR", "ROM failed full validation. \nReal hardware does not check this, but it may indicate a corrupted ROM file.");
            break;
        case ROM_FAIL_READ:
            show_simple_error("ROM LOAD FAILURE", "Failed to read ROM file.");
            return 1;
        case ROM_SIZE_MISMATCH:
            show_simple_error("ROM ERROR", "ROM file size does not match it's declared size.");
            return 1;
        case MEM_FAIL_ALLOC:
            show_simple_error("ROM LOAD FAILURE", "Failed to allocate sufficient memory for ROM.");
            return 1;
        default:
            return 1;
    }

    if (load_save(&cores, rom_path) != 0) {
        std::cerr << "Couldn't load save, continuing without one." << std::endl;
    }

    // global quit flag
    bool quit = false;

    // windows size set depending on if frame is drawn or not
    int W_WIDTH, W_HEIGHT;
    if (DRAW_FRAME) {
        W_WIDTH = 256;
        W_HEIGHT = 212;
    }
    else {
        W_WIDTH = 160;
        W_HEIGHT = 144;
    }

    //set up window
    Window *win = new Window(W_WIDTH * PROGRAM_SCALE, W_HEIGHT * PROGRAM_SCALE);

    win->setTitle(cores.rom_title);
    win->setColour(0);
    win->refresh(true);

    // Draw a frame around the emulator. We make the GPU draw into a separate surface that gets
    // composited onto the window.
    SDL_Surface *wind_buf = win->getSurface();

    SDL_Rect gameboy_screen_rect;

    if (DRAW_FRAME) {
        // Adjust this if you want to put the screen somewhere else
        gameboy_screen_rect = (SDL_Rect){48 * PROGRAM_SCALE, 28 * PROGRAM_SCALE, 160 * PROGRAM_SCALE, 144 * PROGRAM_SCALE};
        SDL_Rect win_size = (SDL_Rect){0, 0, 256 * PROGRAM_SCALE, 212 * PROGRAM_SCALE};

        SDL_Surface *frame = copy_frame(args[0]);

        if (frame) {
            SDL_BlitScaled(frame, NULL, wind_buf, &win_size);
            SDL_FreeSurface(frame);
        }
    }
    else {
        gameboy_screen_rect = (SDL_Rect){0, 0, 160 * PROGRAM_SCALE, 144 * PROGRAM_SCALE};
    }

    if (GPU_P) {
        cores.gpu->choose_palette(GPU_P);
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
            SDL_Delay(1);
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