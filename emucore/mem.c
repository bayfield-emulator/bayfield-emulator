#include <stdlib.h>
#include <string.h>
#include "emucore.h"

#define ALL_RAM_SIZE 32768
#define DEFAULT_OBSERVER (bc_mmio_observe_t)0x1
#define DEFAULT_FETCHER (bc_mmio_fetch_t)0x1

void bc_mmap_alloc(cpu_mmap_t *target) {
    uint8_t *ram = calloc(1, ALL_RAM_SIZE);
    target->all_ram = ram;
    target->vram = ram;
    target->extram = ram + 0x2000;
    target->wram = ram + 0x4000;
    target->sprite = ram + 0x7e00;
    target->zpg = ram + 0x7f80;
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

uint8_t *bc_mmap_calc(cpu_mmap_t *mem, uint16_t addr) {
    if (addr < 0x4000) {
        return mem->rom->bank1 + addr;
    } else if (addr < 0x8000) {
        return mem->rom->bankx + (addr - 0x4000);
    } else {
        return mem->all_ram + (addr - 0x8000);
    }
}

uint8_t bc_mmap_getvalue(cpu_mmap_t *mmap, uint16_t addr) {
    if (addr >= 0xFF00 && addr <= 0xFF7F) {
        int slot = addr - 0xFF00;
        if (!mmap->observers[slot].get) {
            // FIXME: should panic once all mmios implemented
            // debug_log("warning: read from unregistered MMIO register 0x%04x", slot);
            return mmap->mmio_storage[slot];
        } else if (mmap->observers[slot].get == DEFAULT_FETCHER) {
            return mmap->mmio_storage[slot];
        } else {
            return mmap->observers[slot].get(mmap->cpu, addr, mmap->mmio_storage[slot]);
        }
    }

    /* this is the interrupt mask register, outside of normal MMIO space.
     * We combine the master interrupt enable and the mask registers, so remove if_master
     * before returning to program. */
    if (addr == 0xFFFF) {
        return mmap->cpu->irq_mask & (~IF_MASTER);
    }

    return *bc_mmap_calc(mmap, addr);
}
void bc_mmap_putvalue(cpu_mmap_t *mmap, uint16_t addr, uint8_t value) {
    if (addr >= 0xFF00 && addr <= 0xFF7F) {
        int slot = addr - 0xFF00;
        if (!mmap->observers[slot].get) {
            mmap->mmio_storage[slot] = value;
            // FIXME: should panic once all mmios implemented
            // debug_log("warning: write to unregistered MMIO register 0x%04x", slot);
        } else if (mmap->observers[slot].set == DEFAULT_OBSERVER) {
            mmap->mmio_storage[slot] = value;
        } else {
            mmap->mmio_storage[slot] = mmap->observers[slot].set(mmap->cpu, addr, value);
        }
    }

    /* Make sure to save the master flag */
    if (addr == 0xFFFF) {
        mmap->cpu->irq_mask = value | (mmap->cpu->irq_mask & IF_MASTER);
    }

    if (addr < 0x8000) {
        // FIXME: code can write to rom region once we add MBC support
        panic("write to rom region 0x%llx", addr);
    }

    *bc_mmap_calc(mmap, addr) = value;
}

void bc_mmap_putvalue16(cpu_mmap_t *mmap, uint16_t addr, uint16_t value) {
    if (addr >= 0xFF00 && addr <= 0xFF7F) {
        panic("using put16 on a MMIO register");
    }

    if (addr < 0x8000) {
        panic("write to rom region 0x%llx", addr);
    }

    uint8_t *loc = bc_mmap_calc(mmap, addr);
    *loc = value & 0xff;
    *(loc + 1) = value >> 8;
}

void bc_mmap_putstack16(cpu_mmap_t *mmap, uint16_t value) {
    uint8_t *sp = bc_mmap_calc(mmap, mmap->cpu->regs.SP);
    *(sp - 1) = value >> 8;
    *(sp - 2) = value & 0xff;
    mmap->cpu->regs.SP -= 2;
}

uint16_t bc_mmap_popstack16(cpu_mmap_t *mmap) {
    uint8_t *sp = bc_mmap_calc(mmap, mmap->cpu->regs.SP);
    uint16_t value = *(sp) | (*(sp + 1) << 8);
    mmap->cpu->regs.SP += 2;
    return value;
}

void bc_mmap_release(cpu_mmap_t *target) {
    free(target->all_ram);
    if (target->rom) {
        free(target->rom);
    }
    memset(target, 0, sizeof(cpu_mmap_t));
}