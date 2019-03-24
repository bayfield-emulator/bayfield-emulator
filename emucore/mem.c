#include <stdlib.h>
#include <string.h>
#include "emucore.h"

#define ALL_RAM_SIZE (8192 * 3 + 160 + 128)
#define DEFAULT_OBSERVER (bc_mmio_observe_t)0x1
#define DEFAULT_FETCHER (bc_mmio_fetch_t)0x1

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
void bc_mmap_release_rom(cpu_mmap_t *mmap, cartridge_t *rom) {
    mmap->rom = NULL;
}

void bc_mmap_add_mmio_observer(cpu_mmap_t *mmap, uint16_t addr, bc_mmio_observe_t write_proc, bc_mmio_fetch_t read_proc) {
    int slot = addr - 0xFF00;
    mmap->observers[slot].get = read_proc? read_proc : DEFAULT_FETCHER;
    mmap->observers[slot].set = write_proc? write_proc : DEFAULT_OBSERVER;
}

uint8_t bc_mmap_getvalue(cpu_mmap_t *mmap, uint16_t addr) {
    if (addr >= 0xFF00 && addr <= 0xFF7F) {
        int slot = addr - 0xFF00;
        if (!mmap->observers[slot].get) {
            panic("read from unregistered MMIO register 0x%04x", slot);
        } else if (mmap->observers[slot].get == DEFAULT_FETCHER) {
            return mmap->mmio_storage[slot];
        } else {
            return mmap->observers[slot].get(mmap->cpu, addr, mmap->mmio_storage[slot]);
        }
    }

    return mmap->all_ram[addr];
}
void bc_mmap_putvalue(cpu_mmap_t *mmap, uint16_t addr, uint8_t value) {
    if (addr >= 0xFF00 && addr <= 0xFF7F) {
        int slot = addr - 0xFF00;
        if (!mmap->observers[slot].get) {
            panic("write to unregistered MMIO register 0x%04x", slot);
        } else if (mmap->observers[slot].set == DEFAULT_OBSERVER) {
            mmap->mmio_storage[slot] = value;
        } else {
            mmap->mmio_storage[slot] = mmap->observers[slot].set(mmap->cpu, addr, value);
        }
    }

    mmap->all_ram[addr] = value;
}

void bc_mmap_release(cpu_mmap_t *target) {
    free(target->all_ram);
    if (target->rom) {
        free(target->rom);
    }
    memset(target, 0, sizeof(cpu_mmap_t));
}