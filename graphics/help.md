## How to run the graphics demo (on Linux):
1.     sudo apt-get install libsdl2-2.0-0 libsdl2-dev
2. If you have not installed `clang` (it's optional), change the makefile so that `clang++` becomes `g++`.
2.     make
3.     ./demo <path to rom>

## How to run the graphics demo (on macOS):
1. Get the SDL development framework under **Development Libraries** [here](https://www.libsdl.org/download-2.0.php).
2. Mount the `.dmg`, open it, and follow the instructions in the readme (it's only 1 step).
3. If you do not have `clang` installed, install it. I won't provide a tutorial here.
2.     make
3.     ./demo <path to rom>

## How to run the graphics demo (on Windows):
1. Don't.

### Why did the demo drop bitmaps next to it?
Those are dumps of the buffers. You can compare the Window buffer with the Backgroud buffer to see if the Window is being drawn properly.
