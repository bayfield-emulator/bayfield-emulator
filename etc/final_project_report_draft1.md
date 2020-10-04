# A Nintendo Gameboy Emulator
### Or:
### How I Learned to Stop Worrying and Love the Architecture

### CSC 350 Final Project Report

*Student Information Redacted*

***

### Introduction
For our final project, we chose to design and implement an emulator of the handheld gaming console, released in 1989 by Nintendo, the Game Boy. The main goal of this project was to gain an understanding of the architecture of the Game Boy and produce an executable capable of running Game Boy games. We also extended the architecture by adding a two-stage pipeline.


##### Brief History of the Game Boy
The Nintendo Game Boy was the successor to Nintendo’s previous handheld system, the Game & Watch. [Fill]

### Architecture & Emulator Overview
Although the Game Boy CPU resembles the Intel 8080 and Zilog Z80, the Game Boy’s CPU is technically a processor called a Sharp LR25902 [y]. The designers at Nintendo appear to have considered several factors into consideration when designing the Game Boy (e.g. power consumption, price, form factor). [NOTE: expand] Here we present an overview of the Game Boy architecture. The following subsections briefly describe key elements of the architecture and explain related sections of our implementation.

##### Processor
The processor is an 8-bit CPU based off the 8080 and Z80 as mentioned above [x]. The processor is clocked at 4.194304 MHz. [Might merge with instructions]

##### Memory Map
The system consists of two identical 8 KB banks of RAM, one for the main system and one as VRAM. See Fig. x for a general overview of the memory map.

NOTE: Don't worry, all the images will be in the final doc
[Image]

Fig. x. [w] Game Boy memory map

The Cartridge Header Area starting at 0100 is where the Game Boy begins execution. There is typically a jump to 0150, the real beginning of the game ROM [w]. The header contains various kinds data about the cartridge including a Nintendo logo, which is loaded onto the screen at startup. If this logo does not match up with that in internal ROM, the Game Boy halts execution [x].

The next area is for cartridge ROM. In what is referred to as ROM bank 0 is 16Kb of fixed memory. After bank 0 is an area of switchable cartridge ROM memory. Game Boy cartridges may contain a Memory Bank Controller (MBC) which allow banks of ROM (and/or RAM) stored on the cartridge to be mapped into the memory space and thereby allowing for larger ROMs than with a fixed mapping [w]. There are several types of MBCs, and each type supports different ROM/RAM sizes, so our implementation needed to detect the MBC and emulate its operation [elaborate on details].

Addresses 8000 - DFFF and FE00 - FE9F map onto general purpose and video memory. Areas of video memory are defined for tiles, described in the following section; background map; and OAM (Object Attribute Memory, i.e. sprite RAM). One curious area is the Echo RAM at address E000 - FE00. It holds a copy of internal (aka work) RAM [x]. The exact purpose of this area aside is not apparent, but most sources simply state it is not to be used [include sources].

The higher address space is used for I/O registers and High RAM. The I/O registers include joypad, timer, interrupt flag registers, and other hardware control registers.

##### Video
The graphical output is powered by an 8-bit PPU clocked at the same 4 MHz as the CPU. The PPU manages it's own memory, with any writes from the CPU needing to pass through the PPU first. The screen supports a 2-bit colour palette, making for "4 bad shades of green" as Michael Steil put it (33c3 talk link here?)
The video system consists of three main concepts: background, window and sprites. The background is a set of 8 by 8 grids of pixels, tiled across the screen. The area tiles may be assigned to is larger then the actual display, allowing the unit to 'scroll' horizontally and vertically to give the illusion of a moving background. The window is similar to the background, even drawing it's pixel data from the same area of memory, but unlike the background, it is not affected by the PPU's scroll registers, and will remain stationary where positioned. This makes it ideal for small scoreboards along the right or bottom of the display. The final thing to draw is the sprites. These are very self-explanatory -- the blocks in Tetris, Mario, Donkey Kong -- all are sprites. These are the objects in the game that have to move independently of the background. They are drawn on top of the background (with a single exception) and are the only layer to support transparency. There may be up to 40 in use in a game at a time.

##### Timer

##### Interrupts

##### Instructions

### Examples

### User Guide
##### Build Guide
NOTE: Add this section when we have everything together

##### Controls
The Game Boy had a simple interface which we implemented as the following:
- Direction
    - Up - Up arrow key
    - Down - Down arrow key
    - Left - Left arrow key
    - Right - Right Arrow key
- Operation
    - A button - Z key
    - B button - X key
    - Select - Backspace
    - Start - Return

### References
[ x]	DP. Game Boy™ CPU Manual. Accessed: Apr. 2, 2019. [Online]. Available: http://marc.rawer.de/Gameboy/Docs/GBCPUman.pdf

[y]	http://gbdev.gg8.se/wiki/articles/Game_Boy_Technical_Data






