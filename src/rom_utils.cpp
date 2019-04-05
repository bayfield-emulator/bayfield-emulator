#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include "emucore.h"
#include "mbc.h"
#include "GPU.h"
#include "Window.h"
#include "bayfield.h"

void setup_mbc(cartridge_t *cart, uint8_t *full_image) {
    enum mbc_type icfg_type = bc_determine_mbc_type_from_header(full_image);
    cart->mbc_type = icfg_type;

    switch(icfg_type) {
    case MBC_TYPE_JUST_RAM:
        cart->mbc_handler = mbc_bankswitch_only_control;
        cart->extram_handler = normal_extram_write;
        break;
    case MBC_TYPE_1:
        cart->mbc_handler = mbc1_control;
        cart->extram_handler = normal_extram_write;
        break;
    case MBC_TYPE_2:
        cart->mbc_handler = mbc2_control;
        cart->extram_handler = stupid_extram_write;
        break;
    case MBC_TYPE_3:
        cart->mbc_handler = mbc3_control;
        cart->extram_handler = normal_extram_write;
        break;
    case MBC_TYPE_5:
        cart->mbc_handler = mbc5_control;
        cart->extram_handler = normal_extram_write;
        break;
    default: break;
    }

    switch(full_image[0x0149]) {
    case 1:
        cart->extram_usable_size = 0x500;
        cart->extram_base = (uint8_t *)malloc(0x500);
        break;
    case 2:
        cart->extram_usable_size = 0x2000;
        cart->extram_base = (uint8_t *)malloc(0x2000);
        break;
    case 3:
        cart->extram_usable_size = 0x2000;
        cart->extram_base = (uint8_t *)malloc(0x2000 * 4);
        break;
    case 4:
        cart->extram_usable_size = 0x2000;
        cart->extram_base = (uint8_t *)malloc(0x2000 * 16);
        break;
    case 5:
        cart->extram_usable_size = 0x2000;
        cart->extram_base = (uint8_t *)malloc(0x2000 * 8);
        break;
    }

    cart->mbc_context = (mbc_context_t *)calloc(sizeof(mbc_context_t), 1);
    cart->extram = cart->extram_base;
}

bool load_rom(emu_shared_context_t *ctx, const char *filename) {
    std::fstream stream(filename, std::ios::binary);
	stream.open(filename, std::ios::in);

	//test file existence 
	if (!stream.is_open()) {
		std::cerr << "Could not read file" << std::endl;
		return false;
	}

    stream.seekg(0, std::ios::end);
    size_t rom_size = stream.tellg();
    stream.seekg(0);
    uint8_t *full_image = (uint8_t *) malloc(rom_size);
    if (!full_image) {
        std::cerr << "Could not alloc ROM image!" << std::endl;
        return false;
    }

    stream.read((char *)full_image, rom_size);

    //assume file is legitimate ROM
    /*Title [16 bytes long from 0x0134]*/
    /*ROM Type [single byte at 0x0147]*/
    /*RAM Size [single byte at 0x0149]*/
    const char *title = (const char *)(full_image + 0x0134);
    int mbc = full_image[0x0147];
    int ram_size = full_image[0x0149];
    for (int i = 0; i < 16; i++) {
        if (title[i] & 0x80) {
            ctx->rom_title[i] = '\0';
            break;
        }
        ctx->rom_title[i] = title[i];
    }
    ctx->rom_title[15] = '\0';

    /* most of this section can be moved into emucore */
    cartridge_t my_rom;
    my_rom.rom = full_image;
    my_rom.image_size = rom_size;
    my_rom.bank1 = full_image;
    my_rom.bankx = full_image + 16384;
    
    setup_mbc(&my_rom, full_image);

    ctx->cpu->mem.rom = my_rom;

    //rom info
    printf("TITLE: %s \n"
           "ROM TYPE: %x \n"
           "ROM SIZE: %lx \n"
           "RAM_SIZE: %x \n", ctx->rom_title, mbc, rom_size, ram_size);
    return true;
}