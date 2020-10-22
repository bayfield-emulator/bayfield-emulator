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

size_t real_ram_size(uint8_t ram_code, uint8_t mbc_type) {
    switch(ram_code) {
    case 1:
        return 0x500;
    case 2:
        return 0x2000;
    case 3:
        return 0x8000;
    case 4:
        return 0x20000;
    case 5:
        return 0x10000;
    default:
        return 0;
    }
}

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
        cart->extram_handler = rtc_extram_write;
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
        cart->extram_base = (uint8_t *)calloc(0x500, sizeof(uint8_t));
        break;
    case 2:
        cart->extram_usable_size = 0x2000;
        cart->extram_base = (uint8_t *)calloc(0x2000, sizeof(uint8_t));
        break;
    case 3:
        cart->extram_usable_size = 0x2000;
        cart->extram_base = (uint8_t *)calloc(0x2000 * 4, sizeof(uint8_t));
        break;
    case 4:
        cart->extram_usable_size = 0x2000;
        cart->extram_base = (uint8_t *)calloc(0x2000 * 16, sizeof(uint8_t));
        break;
    case 5:
        cart->extram_usable_size = 0x2000;
        cart->extram_base = (uint8_t *)calloc(0x2000 * 8, sizeof(uint8_t));
        break;
    case 0:
        if (icfg_type == MBC_TYPE_2) {
            cart->extram_usable_size = 512;
            cart->extram_base = (uint8_t *)calloc(512, sizeof(uint8_t));
        } else {
            cart->extram_usable_size = 0;
        }
    default:
        cart->extram_usable_size = 0;
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

    // Constrain to 16K chunk. This is the size of a ROM bank.
    size_t rounded = (rom_size & 0x3FFF)? (0x4000 * ((rom_size / 16384) + 1)) : rom_size;
    // Make sure our buffer is at least 32K because we'll be setting up the bank switchable
    // region to point there.
    if (rounded < 0x8000) {
        rounded = 0x8000;
    }

    uint8_t *full_image = (uint8_t *) malloc(rounded);
    if (!full_image) {
        std::cerr << "Could not alloc ROM image!" << std::endl;
        return false;
    }

    if (rom_size < rounded) {
        // Maybe this empty area should be prot_none pages instead of filled with FFs but let's
        // keep it simple for now
        memset(full_image + rom_size, 0xFF, rounded - rom_size);
    }

    stream.read((char *)full_image, rom_size);
    if (stream.gcount() != rom_size) {
        std::cerr << "Could not read all of ROM image." << std::endl;
        free(full_image);
        stream.close();
        return false;
    }

    stream.close();

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
    memset(&my_rom, 0, sizeof(cartridge_t));
    my_rom.rom = full_image;
    my_rom.image_size = rounded;
    my_rom.bank1 = full_image;
    my_rom.bankx = full_image + 16384;

    setup_mbc(&my_rom, full_image);

    ctx->cpu->mem.rom = my_rom;

    //rom info
    printf("TITLE: %s \n"
           "ROM TYPE: %x \n"
           "ROM SIZE: %llx \n"
           "RAM SIZE: %x \n", ctx->rom_title, mbc, (uint64_t)rom_size, ram_size);
    return true;
}

// djb2 hash from http://www.cse.yorku.ca/~oz/hash.html
uint32_t hash_header(uint8_t *rom_base) {
    uint32_t hash = 5381;

    uint8_t *end = rom_base + 0x150;
    for (; rom_base < end; ++rom_base) {
        hash = hash * 33 ^ *rom_base;
    }

    return hash;
}

int load_save(emu_shared_context_t *ctx, const char *filename) {
    cartridge_t *cart = &(ctx->cpu->mem.rom);
    if (!cart->extram_usable_size) {
        return 0;
    }

    // [filename] + '.sav' + 0
    size_t fn_max = strlen(filename) + 4 + 1;
    char *path = (char *)calloc(fn_max, 1);
    snprintf(path, fn_max, "%s.sav", filename);

    uint32_t checksum = 0;
    std::fstream stream(path, std::ios::in | std::ios::binary);

    if (!stream.is_open()) {
        std::cerr << "No previous save" << std::endl;
        free(path);
        return 1;
    }

    stream.read((char *)&checksum, 4);
    stream.seekg(12, std::ios::cur);
    uint32_t real_checksum = hash_header(cart->rom);

    if (real_checksum != ntohl(checksum)) {
        fprintf(stderr, "save is for wrong rom image: (sav)%x vs (rom)%x\n", checksum, real_checksum);
        free(path);
        return 1;
    }

    size_t ram_size = real_ram_size(cart->rom[0x0149], cart->rom[0x0147]);
    stream.read((char *)(cart->extram_base), ram_size);

    if (stream.gcount() != ram_size) {
        fprintf(stderr, "could not read full save image, expected %zu, got %zu\n",
            ram_size, (size_t)stream.gcount());
        free(path);
        memset(cart->extram_base, 0, ram_size);
        return 1;
    }

    stream.close();
    free(path);
    return 0;
}

int dump_save(emu_shared_context_t *ctx, const char *filename) {
    cartridge_t *cart = &(ctx->cpu->mem.rom);
    if (!cart->extram_usable_size) {
        return 0;
    }

    // [filename] + '.sav' + 0
    size_t fn_max = strlen(filename) + 4 + 1;
    char *path = (char *)calloc(fn_max, 1);
    snprintf(path, fn_max, "%s.sav", filename);

    uint32_t real_checksum = htonl(hash_header(cart->rom));
    std::fstream stream(path, std::ios::out | std::ios::binary);
    free(path);

    if (!stream.is_open()) {
        std::cerr << "Could not open save file" << std::endl;
        return 1;
    }

    stream.write((const char *)&real_checksum, 4);
    stream.write("BAYFIELDSAVE", 12);
    stream.write((const char *)(cart->extram_base), real_ram_size(cart->rom[0x0149], cart->rom[0x0147]));
    stream.close();
    return 0;
}