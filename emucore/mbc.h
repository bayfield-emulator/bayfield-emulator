#ifndef MBC_H
#define MBC_H
#include "emucore.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ==== CART STUFF ================================================================ */

enum mbc_type {
    MBC_TYPE_DONTKNOW = 0,
    MBC_TYPE_JUST_RAM,
    MBC_TYPE_1,
    MBC_TYPE_2,
    MBC_TYPE_3,
    MBC_TYPE_5,
};

typedef struct mbc_context {
    uint16_t rom_bank;
    uint16_t enable_ram:1, /* all mbcs */
             mode_sel:1, /* mbc1 only */
    /* used for ram bank num and top two bits of rom select (for MBC1) */
             multi_bits:4;
    int rtc_s;
    int rtc_m;
    int rtc_h;
    int rtc_d;
    int rtc_latch;
    uint64_t rtc_epoch;
    uint8_t *extram_base;
} mbc_context_t;

enum mbc_type bc_determine_mbc_type_from_header(uint8_t *romdata);
void mbc1_control(cpu_mmap_t *mem, uint16_t addr, uint8_t write_val);
void mbc2_control(cpu_mmap_t *mem, uint16_t addr, uint8_t write_val);
void mbc3_control(cpu_mmap_t *mem, uint16_t addr, uint8_t write_val);
void mbc5_control(cpu_mmap_t *mem, uint16_t addr, uint8_t write_val);

#ifdef __cplusplus
}
#endif

#endif