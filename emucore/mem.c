#include <stdlib.h>
#include <string.h>
#include "emucore.h"

#define ALL_RAM_SIZE (8192 * 3 + 160 + 128)

void bc_mmap_alloc(cpu_mmap_t *target) {
    uint8_t *ram = calloc(1, ALL_RAM_SIZE);
    target->all_ram = ram;
    target->vram = ram;
    target->wram = ram + 8192;
    target->extram = ram + 16384;
    target->sprite = ram + 24576;
    target->zpg = ram + 24576 + 160;
}

void bc_mmap_take_rom(cpu_mmap_t *mmap, cartridge_t *rom) {
    mmap->rom = rom;
}

uint8_t bc_mmap_getvalue(cpu_mmap_t *mmap, uint16_t addr) {
    return mmap->all_ram[addr];
}
void bc_mmap_putvalue(cpu_mmap_t *mmap, uint16_t addr, uint8_t value) {
    mmap->all_ram[addr] = value;
}

void bc_mmap_release(cpu_mmap_t *target) {
    free(target->all_ram);
    memset(target, 0, sizeof(cpu_mmap_t));
}