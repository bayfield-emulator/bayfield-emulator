#include "emucore.h"
#include "mbc.h"

enum mbc_type bc_determine_mbc_type_from_header(uint8_t *romdata) {
    switch(romdata[0x0147]) {
    case 0x0:
        return MBC_TYPE_JUST_RAM;
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

void rtc_extram_write(cpu_mmap_t *mem, uint16_t addr, uint8_t write_val) {
    if (mem->rom.mbc_context->multi_bits >= 0x08) {
        // printf("set clock register: %x (%hx) %d\n", mem->rom.mbc_context->multi_bits, addr, (int)write_val);
        return;
    }

    if (addr - 0xa000 >= mem->rom.extram_usable_size || !mem->rom.mbc_context->enable_ram) {
        return;
    }
    mem->rom.extram[addr - 0xA000] = write_val;
}

// WIP
// This handler is only called when a RTC register is selected in mbc3_control
uint8_t rtc_extram_read(cpu_mmap_t *mem, uint16_t addr) {
    // printf("rtc_extram_read: %x (%hx)\n", mem->rom.mbc_context->multi_bits, addr);
    return 0x00;
}

void mbc_bankswitch_only_control(cpu_mmap_t *mem, uint16_t addr, uint8_t write_val) {
    if (addr < 0x2000) {
        mem->rom.mbc_context->enable_ram = ((write_val & 0xf) == 0xa);
        return;
    }

    if (addr < 0x4000) {
        int real_banknum = write_val & 0xf;
        if (real_banknum == 0) {
            real_banknum++;
        }
        mem->rom.mbc_context->rom_bank = real_banknum;
        mem->rom.bankx = mem->rom.rom + (0x4000 * real_banknum);
        return;
    }
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
            mem->rom.extram = mem->rom.extram_base + (0x2000 * (write_val & 0x3));
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
            mem->rom.extram = mem->rom.extram_base + (0x2000 * mem->rom.mbc_context->multi_bits);
            int real_banknum = mem->rom.mbc_context->rom_bank & 0x1f;
            mem->rom.mbc_context->rom_bank = real_banknum;
            mem->rom.bankx = mem->rom.rom + (0x4000 * real_banknum);
        } else {
            mem->rom.extram = mem->rom.extram_base;
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
    } else if (addr < 0x4000) {
        int real_banknum = write_val & 0x7f;
        if (real_banknum == 0) {
            real_banknum++;
        }
        mem->rom.mbc_context->rom_bank = real_banknum;
        mem->rom.bankx = mem->rom.rom + (0x4000 * real_banknum);
        return;
    } else if (addr < 0x6000) {
        // printf("ram bank select %d -> %hx\n", (int)write_val, addr);
        int mb = write_val & 0xf;
        mem->rom.mbc_context->multi_bits = mb;

        if (mb < 0x04) {
            mem->rom.extram = mem->rom.extram_base + (0x2000 * (write_val & 0xf));
            mem->rom.extram_read_handler = NULL;
        } else if (mb >= 0x08 && mb <= 0x0C) {
            // printf("rtc reg selected\n");
            mem->rom.extram_read_handler = &rtc_extram_read;
        }
        return;
    } else {
        // printf("RTC latch register %d -> %hx\n", (int)write_val, addr);
        int s = mem->rom.mbc_context->rtc_l_state;

        if (s == 0 && write_val == 0) {
            mem->rom.mbc_context->rtc_l_state = 1;
        } else if (s == 1 && write_val == 1) {
            mem->rom.mbc_context->rtc_l_state = 2;
            // printf("RTC is now latched\n");
        } else if (s == 2 && write_val == 0) {
            mem->rom.mbc_context->rtc_l_state = 3;
        } else if (s == 3 && write_val == 1) {
            mem->rom.mbc_context->rtc_l_state = 0;
            // printf("RTC is now unlatched\n");
        } else {
            mem->rom.mbc_context->rtc_l_state = (s & 2);
        }
    }

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
        mem->rom.extram = mem->rom.extram_base + (0x2000 * (write_val & 0xf));
        return;
    }
}