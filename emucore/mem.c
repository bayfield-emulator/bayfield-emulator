#include <stdlib.h>
#include <string.h>
#include "emucore.h"

#define DEFAULT_OBSERVER (bc_mmio_observe_t)0x1
#define DEFAULT_FETCHER (bc_mmio_fetch_t)0x1

#define ALL_RAM_SIZE (WRAM_SIZE + HRAM_SIZE)
#define WRAM_SIZE 0x2000
#define HRAM_SIZE 127

void bc_mmap_alloc(cpu_mmap_t *target) {
    uint8_t *ram = calloc(1, ALL_RAM_SIZE);
    target->all_ram = ram;
    target->wram = ram;
    target->hram = ram + WRAM_SIZE;
}

void bc_mmap_add_mmio_observer(cpu_mmap_t *mmap, uint16_t addr, bc_mmio_observe_t write_proc, bc_mmio_fetch_t read_proc, void *context) {
    int slot = addr - 0xFF00;
    mmap->observers[slot].ctx = context;
    mmap->observers[slot].get = read_proc? read_proc : DEFAULT_FETCHER;
    mmap->observers[slot].set = write_proc? write_proc : DEFAULT_OBSERVER;
}

uint8_t *bc_mmap_calc(cpu_mmap_t *mem, uint16_t addr) {
    if (addr < 0x4000) {
        return mem->rom.bank1 + addr;
    } else if (addr < 0x8000) {
        return mem->rom.bankx + (addr - 0x4000);
    }

    // vram
    if (addr < 0xA000) {
        return mem->vram + (addr - 0x8000);
    }
    if (addr < 0xC000) {
        // cart ram
        panic("should have called extram handler for this: %x", addr);
    }
    if (addr < 0xE000) {
        // work ram
        return mem->wram + (addr - 0xC000);
    }
    if (addr < 0xFE00) {
        // copy of 0xC000+
        return mem->wram + (addr - 0xE000);
    }
    if (addr < 0xFEA0) {
        // copy of 0xC000+
        return mem->oam + (addr - 0xFE00);
    }
    if (addr >= 0xFF80) {
        return mem->hram + (addr - 0xFF80);
    }

    panic("shouldn't get here: fault addr %x", addr);
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
            return mmap->observers[slot].get(mmap->cpu, mmap->observers[slot].ctx, addr, mmap->mmio_storage[slot]);
        }
    }

    /* this is the interrupt mask register, outside of normal MMIO space.
     * We combine the master interrupt enable and the mask registers, so remove if_master
     * before returning to program. */
    if (addr == 0xFFFF) {
        return mmap->cpu->irq_mask & (~IF_MASTER);
    }

    if (addr >= 0xFEA0 && addr <= 0xFEFF) {
        debug_log("Program trying to read from unusable region");
        return 0xAA;
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
            mmap->mmio_storage[slot] = mmap->observers[slot].set(mmap->cpu, mmap->observers[slot].ctx, addr, value);
        }
        return;
    }

    if (addr >= 0xA000 && addr <= 0xBFFF) {
        debug_assert(mmap->rom.extram_handler, "writing to extram but there was no handler registered");
        mmap->rom.extram_handler(mmap, addr, value);
    }

    /* Make sure to save the master flag */
    if (addr == 0xFFFF) {
        debug_log("irq mask put %x", value);
        mmap->cpu->irq_mask = value | (mmap->cpu->irq_mask & IF_MASTER);
        return;
    }

    if (addr < 0x8000) {
        debug_assert(mmap->rom.mbc_handler, "writing to cart control but there was no handler registered");
        mmap->rom.mbc_handler(mmap, addr, value);
        return;
    }

    if (addr >= 0xFEA0 && addr <= 0xFEFF) {
        return;
    }

    *bc_mmap_calc(mmap, addr) = value;
}

void bc_mmap_putvalue16(cpu_mmap_t *mmap, uint16_t addr, uint16_t value) {
    if ((addr >= 0xFF00 && addr <= 0xFF7F) || addr == 0xFFFF) {
        panic("using put16 on a MMIO register");
    }

    if (addr < 0x8000) {
        panic("using put16 on MBC");
    }

    if (addr >= 0xFEA0 && addr <= 0xFEFF) {
        return;
    }

    /* Optimize some extram edge cases so we never calc more than once. */
    if (addr == 0x9FFF) {
        mmap->vram[0x1FFF] = value & 0xff;
        mmap->rom.extram_handler(mmap, addr + 1, value >> 8);
    } else if (addr == 0xBFFF) {
        mmap->rom.extram_handler(mmap, addr, value & 0xff);
        mmap->wram[0] = value & 0xff;
    } else if (addr >= 0xA000 && addr < 0xBFFF) {
        mmap->rom.extram_handler(mmap, addr, value & 0xff);
        mmap->rom.extram_handler(mmap, addr + 1, value >> 8);
    } else {
        uint8_t *loc = bc_mmap_calc(mmap, addr);
        *loc = value & 0xff;
        *(loc + 1) = value >> 8;
    }
}

void bc_mmap_putstack16(cpu_mmap_t *mmap, uint16_t value) {
    bc_mmap_putvalue16(mmap, mmap->cpu->regs.SP - 2, value);
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
}