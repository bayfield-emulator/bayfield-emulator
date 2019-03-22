#include "insn.h"

static insn_desc_t instructions[] = {
    INSN_ENTRY(0x0, 1, 4, IMP_nop_0x0),
    INSN_ENTRY(0x1, 1, 12, IMP_ld_0x1),
    INSN_ENTRY(0x2, 1, 8, IMP_ld_0x2),
    INSN_ENTRY(0x3, 1, 8, IMP_inc_0x3),
    INSN_ENTRY(0x4, 1, 4, IMP_inc_0x4),
    INSN_ENTRY(0x5, 1, 4, IMP_dec_0x5),
    INSN_ENTRY(0x6, 1, 8, IMP_ld_0x6),
    INSN_ENTRY(0x7, 1, 4, IMP_rlca_0x7),
    INSN_ENTRY(0x8, 1, 20, IMP_ld_0x8),
    INSN_ENTRY(0x9, 1, 8, IMP_add_0x9),
    INSN_ENTRY(0xa, 1, 8, IMP_ld_0xa),
    INSN_ENTRY(0xb, 1, 8, IMP_dec_0xb),
    INSN_ENTRY(0xc, 1, 4, IMP_inc_0xc),
    INSN_ENTRY(0xd, 1, 4, IMP_dec_0xd),
    INSN_ENTRY(0xe, 1, 8, IMP_ld_0xe),
    INSN_ENTRY(0xf, 1, 4, IMP_rrca_0xf),
    INSN_ENTRY(0x10, 1, 4, IMP_stop_0x10),
    INSN_ENTRY(0x11, 1, 12, IMP_ld_0x11),
    INSN_ENTRY(0x12, 1, 8, IMP_ld_0x12),
    INSN_ENTRY(0x13, 1, 8, IMP_inc_0x13),
    INSN_ENTRY(0x14, 1, 4, IMP_inc_0x14),
    INSN_ENTRY(0x15, 1, 4, IMP_dec_0x15),
    INSN_ENTRY(0x16, 1, 8, IMP_ld_0x16),
    INSN_ENTRY(0x17, 1, 4, IMP_rla_0x17),
    INSN_ENTRY(0x18, 1, 12, IMP_jr_0x18),
    INSN_ENTRY(0x19, 1, 8, IMP_add_0x19),
    INSN_ENTRY(0x1a, 1, 8, IMP_ld_0x1a),
    INSN_ENTRY(0x1b, 1, 8, IMP_dec_0x1b),
    INSN_ENTRY(0x1c, 1, 4, IMP_inc_0x1c),
    INSN_ENTRY(0x1d, 1, 4, IMP_dec_0x1d),
    INSN_ENTRY(0x1e, 1, 8, IMP_ld_0x1e),
    INSN_ENTRY(0x1f, 1, 4, IMP_rra_0x1f),
    INSN_ENTRY(0x20, 1, 12, IMP_jr_0x20),
    INSN_ENTRY(0x21, 1, 12, IMP_ld_0x21),
    INSN_ENTRY(0x22, 1, 8, IMP_ld_0x22),
    INSN_ENTRY(0x23, 1, 8, IMP_inc_0x23),
    INSN_ENTRY(0x24, 1, 4, IMP_inc_0x24),
    INSN_ENTRY(0x25, 1, 4, IMP_dec_0x25),
    INSN_ENTRY(0x26, 1, 8, IMP_ld_0x26),
    INSN_ENTRY(0x27, 1, 4, IMP_daa_0x27),
    INSN_ENTRY(0x28, 1, 12, IMP_jr_0x28),
    INSN_ENTRY(0x29, 1, 8, IMP_add_0x29),
    INSN_ENTRY(0x2a, 1, 8, IMP_ld_0x2a),
    INSN_ENTRY(0x2b, 1, 8, IMP_dec_0x2b),
    INSN_ENTRY(0x2c, 1, 4, IMP_inc_0x2c),
    INSN_ENTRY(0x2d, 1, 4, IMP_dec_0x2d),
    INSN_ENTRY(0x2e, 1, 8, IMP_ld_0x2e),
    INSN_ENTRY(0x2f, 1, 4, IMP_cpl_0x2f),
    INSN_ENTRY(0x30, 1, 12, IMP_jr_0x30),
    INSN_ENTRY(0x31, 1, 12, IMP_ld_0x31),
    INSN_ENTRY(0x32, 1, 8, IMP_ld_0x32),
    INSN_ENTRY(0x33, 1, 8, IMP_inc_0x33),
    INSN_ENTRY(0x34, 1, 12, IMP_inc_0x34),
    INSN_ENTRY(0x35, 1, 12, IMP_dec_0x35),
    INSN_ENTRY(0x36, 1, 12, IMP_ld_0x36),
    INSN_ENTRY(0x37, 1, 4, IMP_scf_0x37),
    INSN_ENTRY(0x38, 1, 12, IMP_jr_0x38),
    INSN_ENTRY(0x39, 1, 8, IMP_add_0x39),
    INSN_ENTRY(0x3a, 1, 8, IMP_ld_0x3a),
    INSN_ENTRY(0x3b, 1, 8, IMP_dec_0x3b),
    INSN_ENTRY(0x3c, 1, 4, IMP_inc_0x3c),
    INSN_ENTRY(0x3d, 1, 4, IMP_dec_0x3d),
    INSN_ENTRY(0x3e, 1, 8, IMP_ld_0x3e),
    INSN_ENTRY(0x3f, 1, 4, IMP_ccf_0x3f),
    INSN_ENTRY(0x40, 1, 4, IMP_ld_0x40),
    INSN_ENTRY(0x41, 1, 4, IMP_ld_0x41),
    INSN_ENTRY(0x42, 1, 4, IMP_ld_0x42),
    INSN_ENTRY(0x43, 1, 4, IMP_ld_0x43),
    INSN_ENTRY(0x44, 1, 4, IMP_ld_0x44),
    INSN_ENTRY(0x45, 1, 4, IMP_ld_0x45),
    INSN_ENTRY(0x46, 1, 8, IMP_ld_0x46),
    INSN_ENTRY(0x47, 1, 4, IMP_ld_0x47),
    INSN_ENTRY(0x48, 1, 4, IMP_ld_0x48),
    INSN_ENTRY(0x49, 1, 4, IMP_ld_0x49),
    INSN_ENTRY(0x4a, 1, 4, IMP_ld_0x4a),
    INSN_ENTRY(0x4b, 1, 4, IMP_ld_0x4b),
    INSN_ENTRY(0x4c, 1, 4, IMP_ld_0x4c),
    INSN_ENTRY(0x4d, 1, 4, IMP_ld_0x4d),
    INSN_ENTRY(0x4e, 1, 8, IMP_ld_0x4e),
    INSN_ENTRY(0x4f, 1, 4, IMP_ld_0x4f),
    INSN_ENTRY(0x50, 1, 4, IMP_ld_0x50),
    INSN_ENTRY(0x51, 1, 4, IMP_ld_0x51),
    INSN_ENTRY(0x52, 1, 4, IMP_ld_0x52),
    INSN_ENTRY(0x53, 1, 4, IMP_ld_0x53),
    INSN_ENTRY(0x54, 1, 4, IMP_ld_0x54),
    INSN_ENTRY(0x55, 1, 4, IMP_ld_0x55),
    INSN_ENTRY(0x56, 1, 8, IMP_ld_0x56),
    INSN_ENTRY(0x57, 1, 4, IMP_ld_0x57),
    INSN_ENTRY(0x58, 1, 4, IMP_ld_0x58),
    INSN_ENTRY(0x59, 1, 4, IMP_ld_0x59),
    INSN_ENTRY(0x5a, 1, 4, IMP_ld_0x5a),
    INSN_ENTRY(0x5b, 1, 4, IMP_ld_0x5b),
    INSN_ENTRY(0x5c, 1, 4, IMP_ld_0x5c),
    INSN_ENTRY(0x5d, 1, 4, IMP_ld_0x5d),
    INSN_ENTRY(0x5e, 1, 8, IMP_ld_0x5e),
    INSN_ENTRY(0x5f, 1, 4, IMP_ld_0x5f),
    INSN_ENTRY(0x60, 1, 4, IMP_ld_0x60),
    INSN_ENTRY(0x61, 1, 4, IMP_ld_0x61),
    INSN_ENTRY(0x62, 1, 4, IMP_ld_0x62),
    INSN_ENTRY(0x63, 1, 4, IMP_ld_0x63),
    INSN_ENTRY(0x64, 1, 4, IMP_ld_0x64),
    INSN_ENTRY(0x65, 1, 4, IMP_ld_0x65),
    INSN_ENTRY(0x66, 1, 8, IMP_ld_0x66),
    INSN_ENTRY(0x67, 1, 4, IMP_ld_0x67),
    INSN_ENTRY(0x68, 1, 4, IMP_ld_0x68),
    INSN_ENTRY(0x69, 1, 4, IMP_ld_0x69),
    INSN_ENTRY(0x6a, 1, 4, IMP_ld_0x6a),
    INSN_ENTRY(0x6b, 1, 4, IMP_ld_0x6b),
    INSN_ENTRY(0x6c, 1, 4, IMP_ld_0x6c),
    INSN_ENTRY(0x6d, 1, 4, IMP_ld_0x6d),
    INSN_ENTRY(0x6e, 1, 8, IMP_ld_0x6e),
    INSN_ENTRY(0x6f, 1, 4, IMP_ld_0x6f),
    INSN_ENTRY(0x70, 1, 8, IMP_ld_0x70),
    INSN_ENTRY(0x71, 1, 8, IMP_ld_0x71),
    INSN_ENTRY(0x72, 1, 8, IMP_ld_0x72),
    INSN_ENTRY(0x73, 1, 8, IMP_ld_0x73),
    INSN_ENTRY(0x74, 1, 8, IMP_ld_0x74),
    INSN_ENTRY(0x75, 1, 8, IMP_ld_0x75),
    INSN_ENTRY(0x76, 1, 4, IMP_halt_0x76),
    INSN_ENTRY(0x77, 1, 8, IMP_ld_0x77),
    INSN_ENTRY(0x78, 1, 4, IMP_ld_0x78),
    INSN_ENTRY(0x79, 1, 4, IMP_ld_0x79),
    INSN_ENTRY(0x7a, 1, 4, IMP_ld_0x7a),
    INSN_ENTRY(0x7b, 1, 4, IMP_ld_0x7b),
    INSN_ENTRY(0x7c, 1, 4, IMP_ld_0x7c),
    INSN_ENTRY(0x7d, 1, 4, IMP_ld_0x7d),
    INSN_ENTRY(0x7e, 1, 8, IMP_ld_0x7e),
    INSN_ENTRY(0x7f, 1, 4, IMP_ld_0x7f),
    INSN_ENTRY(0x80, 1, 4, IMP_insn_add8),
    INSN_ENTRY(0x81, 1, 4, IMP_insn_add8),
    INSN_ENTRY(0x82, 1, 4, IMP_insn_add8),
    INSN_ENTRY(0x83, 1, 4, IMP_insn_add8),
    INSN_ENTRY(0x84, 1, 4, IMP_insn_add8),
    INSN_ENTRY(0x85, 1, 4, IMP_insn_add8),
    INSN_ENTRY(0x86, 1, 8, IMP_insn_add8),
    INSN_ENTRY(0x87, 1, 4, IMP_insn_add8),
    INSN_ENTRY(0x88, 1, 4, IMP_insn_add8),
    INSN_ENTRY(0x89, 1, 4, IMP_insn_add8),
    INSN_ENTRY(0x8a, 1, 4, IMP_insn_add8),
    INSN_ENTRY(0x8b, 1, 4, IMP_insn_add8),
    INSN_ENTRY(0x8c, 1, 4, IMP_insn_add8),
    INSN_ENTRY(0x8d, 1, 4, IMP_insn_add8),
    INSN_ENTRY(0x8e, 1, 8, IMP_insn_add8),
    INSN_ENTRY(0x8f, 1, 4, IMP_insn_add8),
    INSN_ENTRY(0x90, 1, 4, IMP_sub_0x90),
    INSN_ENTRY(0x91, 1, 4, IMP_sub_0x91),
    INSN_ENTRY(0x92, 1, 4, IMP_sub_0x92),
    INSN_ENTRY(0x93, 1, 4, IMP_sub_0x93),
    INSN_ENTRY(0x94, 1, 4, IMP_sub_0x94),
    INSN_ENTRY(0x95, 1, 4, IMP_sub_0x95),
    INSN_ENTRY(0x96, 1, 8, IMP_sub_0x96),
    INSN_ENTRY(0x97, 1, 4, IMP_sub_0x97),
    INSN_ENTRY(0x98, 1, 4, IMP_sbc_0x98),
    INSN_ENTRY(0x99, 1, 4, IMP_sbc_0x99),
    INSN_ENTRY(0x9a, 1, 4, IMP_sbc_0x9a),
    INSN_ENTRY(0x9b, 1, 4, IMP_sbc_0x9b),
    INSN_ENTRY(0x9c, 1, 4, IMP_sbc_0x9c),
    INSN_ENTRY(0x9d, 1, 4, IMP_sbc_0x9d),
    INSN_ENTRY(0x9e, 1, 8, IMP_sbc_0x9e),
    INSN_ENTRY(0x9f, 1, 4, IMP_sbc_0x9f),
    INSN_ENTRY(0xa0, 1, 4, IMP_and_0xa0),
    INSN_ENTRY(0xa1, 1, 4, IMP_and_0xa1),
    INSN_ENTRY(0xa2, 1, 4, IMP_and_0xa2),
    INSN_ENTRY(0xa3, 1, 4, IMP_and_0xa3),
    INSN_ENTRY(0xa4, 1, 4, IMP_and_0xa4),
    INSN_ENTRY(0xa5, 1, 4, IMP_and_0xa5),
    INSN_ENTRY(0xa6, 1, 8, IMP_and_0xa6),
    INSN_ENTRY(0xa7, 1, 4, IMP_and_0xa7),
    INSN_ENTRY(0xa8, 1, 4, IMP_xor_0xa8),
    INSN_ENTRY(0xa9, 1, 4, IMP_xor_0xa9),
    INSN_ENTRY(0xaa, 1, 4, IMP_xor_0xaa),
    INSN_ENTRY(0xab, 1, 4, IMP_xor_0xab),
    INSN_ENTRY(0xac, 1, 4, IMP_xor_0xac),
    INSN_ENTRY(0xad, 1, 4, IMP_xor_0xad),
    INSN_ENTRY(0xae, 1, 8, IMP_xor_0xae),
    INSN_ENTRY(0xaf, 1, 4, IMP_xor_0xaf),
    INSN_ENTRY(0xb0, 1, 4, IMP_or_0xb0),
    INSN_ENTRY(0xb1, 1, 4, IMP_or_0xb1),
    INSN_ENTRY(0xb2, 1, 4, IMP_or_0xb2),
    INSN_ENTRY(0xb3, 1, 4, IMP_or_0xb3),
    INSN_ENTRY(0xb4, 1, 4, IMP_or_0xb4),
    INSN_ENTRY(0xb5, 1, 4, IMP_or_0xb5),
    INSN_ENTRY(0xb6, 1, 8, IMP_or_0xb6),
    INSN_ENTRY(0xb7, 1, 4, IMP_or_0xb7),
    INSN_ENTRY(0xb8, 1, 4, IMP_cp_0xb8),
    INSN_ENTRY(0xb9, 1, 4, IMP_cp_0xb9),
    INSN_ENTRY(0xba, 1, 4, IMP_cp_0xba),
    INSN_ENTRY(0xbb, 1, 4, IMP_cp_0xbb),
    INSN_ENTRY(0xbc, 1, 4, IMP_cp_0xbc),
    INSN_ENTRY(0xbd, 1, 4, IMP_cp_0xbd),
    INSN_ENTRY(0xbe, 1, 8, IMP_cp_0xbe),
    INSN_ENTRY(0xbf, 1, 4, IMP_cp_0xbf),
    INSN_ENTRY(0xc0, 1, 20, IMP_ret_0xc0),
    INSN_ENTRY(0xc1, 1, 12, IMP_pop_0xc1),
    INSN_ENTRY(0xc2, 1, 16, IMP_jp_0xc2),
    INSN_ENTRY(0xc3, 1, 16, IMP_jp_0xc3),
    INSN_ENTRY(0xc4, 1, 24, IMP_call_0xc4),
    INSN_ENTRY(0xc5, 1, 16, IMP_push_0xc5),
    INSN_ENTRY(0xc6, 1, 8, IMP_add_0xc6),
    INSN_ENTRY(0xc7, 1, 16, IMP_rst_0xc7),
    INSN_ENTRY(0xc8, 1, 20, IMP_ret_0xc8),
    INSN_ENTRY(0xc9, 1, 16, IMP_ret_0xc9),
    INSN_ENTRY(0xca, 1, 16, IMP_jp_0xca),
    INSN_ENTRY(0xcb, 1, 4, IMP_prefix_0xcb),
    INSN_ENTRY(0xcc, 1, 24, IMP_call_0xcc),
    INSN_ENTRY(0xcd, 1, 24, IMP_call_0xcd),
    INSN_ENTRY(0xce, 1, 8, IMP_adc_0xce),
    INSN_ENTRY(0xcf, 1, 16, IMP_rst_0xcf),
    INSN_ENTRY(0xd0, 1, 20, IMP_ret_0xd0),
    INSN_ENTRY(0xd1, 1, 12, IMP_pop_0xd1),
    INSN_ENTRY(0xd2, 1, 16, IMP_jp_0xd2),
    INSN_ENTRY(0xd4, 1, 24, IMP_call_0xd4),
    INSN_ENTRY(0xd5, 1, 16, IMP_push_0xd5),
    INSN_ENTRY(0xd6, 1, 8, IMP_sub_0xd6),
    INSN_ENTRY(0xd7, 1, 16, IMP_rst_0xd7),
    INSN_ENTRY(0xd8, 1, 20, IMP_ret_0xd8),
    INSN_ENTRY(0xd9, 1, 16, IMP_reti_0xd9),
    INSN_ENTRY(0xda, 1, 16, IMP_jp_0xda),
    INSN_ENTRY(0xdc, 1, 24, IMP_call_0xdc),
    INSN_ENTRY(0xde, 1, 8, IMP_sbc_0xde),
    INSN_ENTRY(0xdf, 1, 16, IMP_rst_0xdf),
    INSN_ENTRY(0xe0, 1, 12, IMP_ldh_0xe0),
    INSN_ENTRY(0xe1, 1, 12, IMP_pop_0xe1),
    INSN_ENTRY(0xe2, 1, 8, IMP_ld_0xe2),
    INSN_ENTRY(0xe5, 1, 16, IMP_push_0xe5),
    INSN_ENTRY(0xe6, 1, 8, IMP_and_0xe6),
    INSN_ENTRY(0xe7, 1, 16, IMP_rst_0xe7),
    INSN_ENTRY(0xe8, 1, 16, IMP_add_0xe8),
    INSN_ENTRY(0xe9, 1, 4, IMP_jp_0xe9),
    INSN_ENTRY(0xea, 1, 16, IMP_ld_0xea),
    INSN_ENTRY(0xee, 1, 8, IMP_xor_0xee),
    INSN_ENTRY(0xef, 1, 16, IMP_rst_0xef),
    INSN_ENTRY(0xf0, 1, 12, IMP_ldh_0xf0),
    INSN_ENTRY(0xf1, 1, 12, IMP_pop_0xf1),
    INSN_ENTRY(0xf2, 1, 8, IMP_ld_0xf2),
    INSN_ENTRY(0xf3, 1, 4, IMP_di_0xf3),
    INSN_ENTRY(0xf5, 1, 16, IMP_push_0xf5),
    INSN_ENTRY(0xf6, 1, 8, IMP_or_0xf6),
    INSN_ENTRY(0xf7, 1, 16, IMP_rst_0xf7),
    INSN_ENTRY(0xf8, 1, 12, IMP_ld_0xf8),
    INSN_ENTRY(0xf9, 1, 8, IMP_ld_0xf9),
    INSN_ENTRY(0xfa, 1, 16, IMP_ld_0xfa),
    INSN_ENTRY(0xfb, 1, 4, IMP_ei_0xfb),
    INSN_ENTRY(0xfe, 1, 8, IMP_cp_0xfe),
    INSN_ENTRY(0xff, 1, 16, IMP_rst_0xff),
};

/* 8-bit transfer. */

#define LOAD_REG(REG) transfer = ( cpu->regs.##REG ); break
#define LOAD_IPR(HI, LO) transfer = ( bc_mmap_getvalue(&cpu->mem, ((cpu->regs.##HI << 8) | cpu->regs.##LO)) ); break
#define STORE_REG(REG) cpu->regs.##REG = transfer
#define STORE_IPR(HI, LO) bc_mmap_putvalue(&cpu->mem, ((cpu->regs.##HI << 8) | cpu->regs.##LO), transfer)

/* opcodes: 0x[4567][0123456789ABCDEF] */
static void IMP_insn_ld8(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    int dst = (opcode >> 4) & 0xF;
    int src = opcode        & 0xF;
    uint8_t transfer;

    switch(src) {
    case 0x0: LOAD_REG(B);
    case 0x1: LOAD_REG(C);
    case 0x2: LOAD_REG(D);
    case 0x3: LOAD_REG(E);
    case 0x4: LOAD_REG(H);
    case 0x5: LOAD_REG(L);
    case 0x6: LOAD_IPR(H, L);
    case 0x7: LOAD_REG(A);
    case 0x8: LOAD_REG(B);
    case 0x9: LOAD_REG(C);
    case 0xA: LOAD_REG(D);
    case 0xB: LOAD_REG(E);
    case 0xC: LOAD_REG(H);
    case 0xD: LOAD_REG(L);
    case 0xE: LOAD_IPR(H, L);
    }

    switch(dst) {
    case 0x4:
        if (src < 0x8) {
            STORE_REG(B);
        } else {
            STORE_REG(C);
        }; break;
    case 0x5:
        if (src < 0x8) {
            STORE_REG(D);
        } else {
            STORE_REG(E);
        }; break;
    case 0x6:
        if (src < 0x8) {
            STORE_REG(H);
        } else {
            STORE_REG(L);
        }; break;
    case 0x7:
        if (src < 0x8) {
            STORE_IPR(H, L);
        } else {
            STORE_REG(A);
        }; break;
    }
}

/* opcodes: 0x[0123]2 */
static void IMP_insn_store_A(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    int lo = (opcode >> 4) & 0xF;
    int hi = opcode        & 0xF;
    uint8_t transfer = cpu->regs.A;
    switch(hi) {
    case 0x0: STORE_IPR(B, C); break;
    case 0x1: STORE_IPR(D, E); break;
    case 0x2:
        STORE_IPR(H, L);
        bc_regs_putpairvalue(&cpu->regs, PAIR_HL, bc_regs_getpairvalue(&cpu->regs, PAIR_HL) + 1);
        break;
    case 0x3:
        STORE_IPR(H, L);
        bc_regs_putpairvalue(&cpu->regs, PAIR_HL, bc_regs_getpairvalue(&cpu->regs, PAIR_HL) - 1);
        break;
    }
}

/* opcodes: 0x[0123]A */
static void IMP_insn_ld_indirect(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    int lo = (opcode >> 4) & 0xF;
    int hi = opcode        & 0xF;
    uint8_t transfer;
    switch(hi) {
    case 0x0: LOAD_IPR(B, C); break;
    case 0x1: LOAD_IPR(D, E); break;
    case 0x2:
        LOAD_IPR(H, L);
        bc_regs_putpairvalue(&cpu->regs, PAIR_HL, bc_regs_getpairvalue(&cpu->regs, PAIR_HL) + 1);
        break;
    case 0x3:
        LOAD_IPR(H, L);
        bc_regs_putpairvalue(&cpu->regs, PAIR_HL, bc_regs_getpairvalue(&cpu->regs, PAIR_HL) - 1);
        break;
    }

    cpu->regs.A = transfer;
}

/* opcodes: 0x[0123][6E] */
static void IMP_insn_ld_immediate(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    uint8_t transfer;
    switch(opcode) {
    case 0x06: cpu->regs.B = param; break;
    case 0x16: cpu->regs.D = param; break;
    case 0x26: cpu->regs.H = param; break;
    case 0x36: transfer = param; STORE_IPR(H, L); break;
    case 0x0E: cpu->regs.C = param; break;
    case 0x1E: cpu->regs.E = param; break;
    case 0x2E: cpu->regs.L = param; break;
    case 0x3E: cpu->regs.A = param; break;
    }

    cpu->regs.A = transfer;
}

static void IMP_insn_ld_absolute(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    switch(opcode) {
    case 0xFA:
        cpu->regs.A = bc_mmap_getvalue(&cpu->mem, param & 0xFFFF);
        break;
    case 0xEA:
        bc_mmap_putvalue(&cpu->mem, param & 0xFFFF, cpu->regs.A);
        break;
    case 0xF0:
        cpu->regs.A = bc_mmap_getvalue(&cpu->mem, 0xFF00 + (param & 0xFF));
        break;
    case 0xE0:
        bc_mmap_putvalue(&cpu->mem, 0xFF00 + (param & 0xFF), cpu->regs.A);
        break;
    case 0xF2:
        cpu->regs.A = bc_mmap_getvalue(&cpu->mem, 0xFF00 + cpu->regs.C);
        break;
    case 0xE2:
        bc_mmap_putvalue(&cpu->mem, 0xFF00 + cpu->regs.C, cpu->regs.A);
        break;
    }
}
#undef STORE_IPR
#undef STORE_REG
#undef LOAD_IPR
#undef LOAD_REG

/* Data Processing */

#define LOAD_REG(DST, REG) DST = ( cpu->regs.##REG ); break
#define GET_HL_INDIRECT() bc_mmap_getvalue(&cpu->mem, bc_regs_getpairvalue(&cpu->regs, PAIR_HL))
static void IMP_insn_add8(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    int lo = (opcode >> 4) & 0xF;
    int hi = opcode        & 0xF;

    int add_val;
    switch(lo & 0x7) {
    case 0x0: LOAD_REG(add_val, B);
    case 0x1: LOAD_REG(add_val, C);
    case 0x2: LOAD_REG(add_val, D);
    case 0x3: LOAD_REG(add_val, E);
    case 0x4: LOAD_REG(add_val, H);
    case 0x5: LOAD_REG(add_val, L);
    case 0x6: add_val = GET_HL_INDIRECT(); break;
    case 0x7: LOAD_REG(add_val, A);
    }

    if (lo & 0x8) {
        // this is an ADC inst
        int cflag = bc_regs_test_cpsr_flag(&cpu->regs, FLAG_CARRY);
        if (((cpu->regs.A & 0xf) + (add_val & 0xf) + cflag) & 0x10) {
            cpu->regs.CPSR |= FLAG_HALF_CARRY;
        } else {
            cpu->regs.CPSR &= FLAG_HALF_CARRY;
        }
        add_val += cpu->regs.A + cflag;
    } else {
        if (((cpu->regs.A & 0xf) + (add_val & 0xf)) & 0x10) {
            cpu->regs.CPSR |= FLAG_HALF_CARRY;
        } else {
            cpu->regs.CPSR &= FLAG_HALF_CARRY;
        }
        add_val += cpu->regs.A;
    }

    if (add_val > 255) {
        cpu->regs.CPSR |= FLAG_CARRY;
    } else {
        cpu->regs.CPSR &= FLAG_CARRY;
    }

    if ((add_val & 0xFF) == 0) {
        cpu->regs.CPSR |= FLAG_ZERO;
    } else {
        cpu->regs.CPSR &= FLAG_ZERO;
    }

    cpu->regs.CPSR &= FLAG_NEGATIVE;
    cpu->regs.A = add_val;
}

static void IMP_insn_sub8(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    int lo = (opcode >> 4) & 0xF;
    int hi = opcode        & 0xF;

    int add_val;
    switch(lo & 0x7) {
    case 0x0: LOAD_REG(add_val, B);
    case 0x1: LOAD_REG(add_val, C);
    case 0x2: LOAD_REG(add_val, D);
    case 0x3: LOAD_REG(add_val, E);
    case 0x4: LOAD_REG(add_val, H);
    case 0x5: LOAD_REG(add_val, L);
    case 0x6: add_val = GET_HL_INDIRECT(); break;
    case 0x7: LOAD_REG(add_val, A);
    }

    if (lo & 0x8) {
        // this is an SBC inst
        int cflag = bc_regs_test_cpsr_flag(&cpu->regs, FLAG_CARRY);
        if (((cpu->regs.A & 0xf) - (add_val & 0xf) - cflag) & 0x10) {
            cpu->regs.CPSR |= FLAG_HALF_CARRY;
        } else {
            cpu->regs.CPSR &= FLAG_HALF_CARRY;
        }
        add_val -= cpu->regs.A - cflag;
    } else {
        if (((cpu->regs.A & 0xf) - (add_val & 0xf)) & 0x10) {
            cpu->regs.CPSR |= FLAG_HALF_CARRY;
        } else {
            cpu->regs.CPSR &= FLAG_HALF_CARRY;
        }
        add_val -= cpu->regs.A;
    }

    if (add_val < -128) {
        cpu->regs.CPSR |= FLAG_CARRY;
    } else {
        cpu->regs.CPSR &= FLAG_CARRY;
    }

    if ((add_val & 0xFF) == 0) {
        cpu->regs.CPSR |= FLAG_ZERO;
    } else {
        cpu->regs.CPSR &= FLAG_ZERO;
    }

    cpu->regs.CPSR |= FLAG_NEGATIVE;
    if (hi != 0xB) {
        cpu->regs.A = add_val;
    }
}

static void IMP_insn_and8(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    uint8_t add_val;
    switch(opcode & 0x7) {
    case 0x0: LOAD_REG(add_val, B);
    case 0x1: LOAD_REG(add_val, C);
    case 0x2: LOAD_REG(add_val, D);
    case 0x3: LOAD_REG(add_val, E);
    case 0x4: LOAD_REG(add_val, H);
    case 0x5: LOAD_REG(add_val, L);
    case 0x6: add_val = GET_HL_INDIRECT(); break;
    case 0x7: LOAD_REG(add_val, A);
    }

    uint8_t wb = cpu->regs.A & add_val;
    if (!wb) {
        cpu->regs.CPSR |= FLAG_ZERO;
    } else {
        cpu->regs.CPSR &= 0;
    }

    cpu->regs.A = wb;
}

static void IMP_insn_xor8(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    uint8_t add_val;
    switch(opcode & 0x7) {
    case 0x0: LOAD_REG(add_val, B);
    case 0x1: LOAD_REG(add_val, C);
    case 0x2: LOAD_REG(add_val, D);
    case 0x3: LOAD_REG(add_val, E);
    case 0x4: LOAD_REG(add_val, H);
    case 0x5: LOAD_REG(add_val, L);
    case 0x6: add_val = GET_HL_INDIRECT(); break;
    case 0x7: LOAD_REG(add_val, A);
    }

    uint8_t wb = cpu->regs.A ^ add_val;
    if (!wb) {
        cpu->regs.CPSR = FLAG_ZERO;
    } else {
        cpu->regs.CPSR = 0;
    }

    cpu->regs.A = wb;
}

static void IMP_insn_or8(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    uint8_t add_val;
    switch(opcode & 0x7) {
    case 0x0: LOAD_REG(add_val, B);
    case 0x1: LOAD_REG(add_val, C);
    case 0x2: LOAD_REG(add_val, D);
    case 0x3: LOAD_REG(add_val, E);
    case 0x4: LOAD_REG(add_val, H);
    case 0x5: LOAD_REG(add_val, L);
    case 0x6: add_val = GET_HL_INDIRECT(); break;
    case 0x7: LOAD_REG(add_val, A);
    }

    uint8_t wb = cpu->regs.A | add_val;
    if (!wb) {
        cpu->regs.CPSR = FLAG_ZERO;
    } else {
        cpu->regs.CPSR = 0;
    }

    cpu->regs.A = wb;
}
