#include "emucore.h"
#include "mbc.h"

enum mbc_type bc_determine_mbc_type_from_header(uint8_t *romdata) {
    switch(romdata[0x0147]) {
    case 0x01:
    case 0x02:
    case 0x03:
        return MBC_TYPE_1;
    case 0x05:
    case 0x06:
        return MBC_TYPE_2;
    case 0x08:
    case 0x09:
        return MBC_TYPE_JUST_RAM;
    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
        return MBC_TYPE_3;
    case 0x19:
    case 0x1A:
    case 0x1B:
    case 0x1C:
    case 0x1D:
    case 0x1E:
        return MBC_TYPE_5;
    }

    return 0;
}

void normal_extram_write(cpu_mmap_t *mem, uint16_t addr, uint8_t write_val) {
    if (addr - 0xa000 >= mem->rom.extram_usable_size || !mem->rom.mbc_context->enable_ram) {
        return;
    }
    mem->rom.extram[addr - 0xA000] = write_val;
}

void stupid_extram_write(cpu_mmap_t *mem, uint16_t addr, uint8_t write_val) {
    if (addr - 0xa000 >= mem->rom.extram_usable_size || !mem->rom.mbc_context->enable_ram) {
        return;
    }
    mem->rom.extram[addr - 0xA000] = write_val & 0xf;
}

void mbc1_control(cpu_mmap_t *mem, uint16_t addr, uint8_t write_val) {
    if (addr < 0x2000) {
        mem->rom.mbc_context->enable_ram = ((write_val & 0xf) == 0xa);
        return;
    }

    if (addr < 0x4000) {
        int real_banknum = write_val & 0x1f;
        if (real_banknum == 0) {
            real_banknum++;
        }
        if (mem->rom.mbc_context->mode_sel == 0) {
            real_banknum |= (mem->rom.mbc_context->multi_bits << 5);
        }
        mem->rom.mbc_context->rom_bank = real_banknum;
        mem->rom.bankx = mem->rom.rom + (0x4000 * real_banknum);
        return;
    }

    if (addr < 0x6000) {
        mem->rom.mbc_context->multi_bits = write_val & 0x3;
        if (mem->rom.mbc_context->mode_sel == 1) {
            mem->rom.extram = mem->rom.mbc_context->real_extram_base + (0x2000 * (write_val & 0x3));
        } else {
            int real_banknum = ((write_val & 0x3) << 5) | (mem->rom.mbc_context->rom_bank & 0x1f);
            mem->rom.mbc_context->rom_bank = real_banknum;
            mem->rom.bankx = mem->rom.rom + (0x4000 * real_banknum);
        }
        return;
    }

    if (addr < 0x8000) {
        mem->rom.mbc_context->mode_sel = write_val & 0x1;
        if (write_val & 1) {
            mem->rom.extram = mem->rom.mbc_context->real_extram_base + (0x2000 * mem->rom.mbc_context->multi_bits);
            int real_banknum = mem->rom.mbc_context->rom_bank & 0x1f;
            mem->rom.mbc_context->rom_bank = real_banknum;
            mem->rom.bankx = mem->rom.rom + (0x4000 * real_banknum);
        } else {
            mem->rom.extram = mem->rom.mbc_context->real_extram_base;
            int real_banknum = ((mem->rom.mbc_context->multi_bits & 0x3) << 5) | (mem->rom.mbc_context->rom_bank & 0x1f);
            mem->rom.mbc_context->rom_bank = real_banknum;
            mem->rom.bankx = mem->rom.rom + (0x4000 * real_banknum);
        }
        return;
    }
}

void mbc2_control(cpu_mmap_t *mem, uint16_t addr, uint8_t write_val) {
    if (addr < 0x2000 && !(addr & 0x0100)) {
        mem->rom.mbc_context->enable_ram = ((write_val & 0xf) == 0xa);
        return;
    }

    if (addr >= 0x2000 && addr < 0x4000 && (addr & 0x0100)) {
        int real_banknum = write_val & 0xf;
        if (real_banknum == 0) {
            real_banknum++;
        }
        mem->rom.mbc_context->rom_bank = real_banknum;
        mem->rom.bankx = mem->rom.rom + (0x4000 * real_banknum);
        return;
    }
}

void mbc3_control(cpu_mmap_t *mem, uint16_t addr, uint8_t write_val) {
    if (addr < 0x2000) {
        mem->rom.mbc_context->enable_ram = ((write_val & 0xf) == 0xa);
        return;
    }

    if (addr < 0x4000) {
        int real_banknum = write_val & 0x7f;
        if (real_banknum == 0) {
            real_banknum++;
        }
        mem->rom.mbc_context->rom_bank = real_banknum;
        mem->rom.bankx = mem->rom.rom + (0x4000 * real_banknum);
        return;
    }

    if (addr < 0x6000) {
        mem->rom.mbc_context->multi_bits = write_val & 0xf;
        if ((write_val & 0xf) < 0x07) {
            mem->rom.extram = mem->rom.mbc_context->real_extram_base + (0x2000 * (write_val & 0xf));
        } else {
            panic("someone needs to implement RTC support here");
        }
        return;
    }

    panic("someone needs to implement RTC support here");
}

void mbc5_control(cpu_mmap_t *mem, uint16_t addr, uint8_t write_val) {
    if (addr < 0x2000) {
        mem->rom.mbc_context->enable_ram = ((write_val & 0xf) == 0xa);
        return;
    }

    if (addr < 0x3000) {
        int bank = (mem->rom.mbc_context->rom_bank & 0x100) | write_val;
        mem->rom.mbc_context->rom_bank = bank;
        mem->rom.bankx = mem->rom.rom + (0x4000 * bank);
        return;
    }

    if (addr < 0x4000) {
        int bank = (mem->rom.mbc_context->rom_bank & 0xff) | (write_val & 1);
        mem->rom.mbc_context->rom_bank = bank;
        mem->rom.bankx = mem->rom.rom + (0x4000 * bank);
        return;
    }

    if (addr < 0x6000) {
        mem->rom.mbc_context->multi_bits = write_val & 0xf;
        mem->rom.extram = mem->rom.mbc_context->real_extram_base + (0x2000 * (write_val & 0xf));
        return;
    }
}