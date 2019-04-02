#include "emucore.h"
#include "insn.h"

/* Sets up the fake insn_desc_t for a class of extended instructions. */
#define DECL_CB_INSTRUCTION_PAIR_EX(name, func, ncycles_short, ncycles_long) \
    static void func(bc_cpu_t *cpu, int opcode, int cycle, int real_opcode); \
    const insn_desc_t CB_INSN_LONG_##name = (insn_desc_t){0xcb, 1, ncycles_long, &func}; \
    const insn_desc_t CB_INSN_SHORT_##name = (insn_desc_t){0xcb, 1, ncycles_short, &func}
#define DECL_CB_INSTRUCTION_PAIR(name, func) DECL_CB_INSTRUCTION_PAIR_EX(name, func, 8, 16);

#define SELECT_CB_INSTRUCTION_PAIR(name, is_long) \
    ((is_long)? &CB_INSN_LONG_##name : &CB_INSN_SHORT_##name)

DECL_CB_INSTRUCTION_PAIR(bit_set_or_clr, IMP_insn_set_res);
DECL_CB_INSTRUCTION_PAIR_EX(bit_select, IMP_insn_bit_sel, 8, 12);
DECL_CB_INSTRUCTION_PAIR(rotate_right, IMP_rot_right_e);
DECL_CB_INSTRUCTION_PAIR(rotate_left, IMP_rot_left_e);
DECL_CB_INSTRUCTION_PAIR(rotate_right_replicate, IMP_rot_right_rep);
DECL_CB_INSTRUCTION_PAIR(rotate_left_zfill, IMP_rot_left_fill);
DECL_CB_INSTRUCTION_PAIR(swap, IMP_swap_nibble);

const insn_desc_t *select_extended_instruction(int opcode) {
    int hi = opcode >> 4;
    int lo = opcode & 0xf;

    int is_long = 0;
    if (lo == 0x6 || lo == 0xE) {
        // All insns ending in 6 or E operate on (HL), so they take double clocks.
        is_long = 1;
    }

    if (hi >= 0x8) {
        return SELECT_CB_INSTRUCTION_PAIR(bit_set_or_clr, is_long);
    }
    if (hi >= 0x4 && hi < 0x8) {
        return SELECT_CB_INSTRUCTION_PAIR(bit_select, is_long);
    }
    if (hi == 0x3 && lo < 0x8) {
        return SELECT_CB_INSTRUCTION_PAIR(swap, is_long);
    }
    if (hi == 0x3 && lo >= 0x8) {
        return SELECT_CB_INSTRUCTION_PAIR(rotate_right_replicate, is_long);
    }
    if (hi == 0x2 && lo < 0x8) {
        return SELECT_CB_INSTRUCTION_PAIR(rotate_left_zfill, is_long);
    }
    if (hi == 0x2 && lo >= 0x8) {
        return SELECT_CB_INSTRUCTION_PAIR(rotate_right_replicate, is_long);
    }
    if (hi == 0x1 || hi == 0x0) {
        if (lo >= 0x8) {
            return SELECT_CB_INSTRUCTION_PAIR(rotate_right, is_long);
        } else {
            return SELECT_CB_INSTRUCTION_PAIR(rotate_left, is_long);
        }
    }

    panic("no extended insn for that opcode?");
}

static uint8_t get_reg_by_cb_encoding_num(bc_cpu_t *cpu, int regnum) {
    #define GET_REG_VAL(reg) cpu->regs.reg

    switch (regnum) {
    case 0: return GET_REG_VAL(B);
    case 1: return GET_REG_VAL(C);
    case 2: return GET_REG_VAL(D);
    case 3: return GET_REG_VAL(E);
    case 4: return GET_REG_VAL(H);
    case 5: return GET_REG_VAL(L);
    case 6:
        return bc_mmap_getvalue(&cpu->mem, bc_regs_getpairvalue(&cpu->regs, PAIR_HL));
    case 7: return GET_REG_VAL(A);
    }

    panic("get_reg_by_cb_encoding_num: invalid register number");
    #undef GET_REG_VAL
}

static void put_reg_by_cb_encoding_num(bc_cpu_t *cpu, int regnum, uint8_t val) {
    #define PUT_REG_VAL(reg, v) cpu->regs.reg = v

    switch (regnum) {
    case 0: PUT_REG_VAL(B, val); return;
    case 1: PUT_REG_VAL(C, val); return;
    case 2: PUT_REG_VAL(D, val); return;
    case 3: PUT_REG_VAL(E, val); return;
    case 4: PUT_REG_VAL(H, val); return;
    case 5: PUT_REG_VAL(L, val); return;
    case 6:
        bc_mmap_putvalue(&cpu->mem, bc_regs_getpairvalue(&cpu->regs, PAIR_HL), val); return;
    case 7: PUT_REG_VAL(A, val); return;
    }

    panic("put_reg_by_cb_encoding_num: invalid register number");
    #undef PUT_REG_VAL
}

// rlc, rl
static void IMP_rot_left_e(bc_cpu_t *cpu, int opcode, int cycle, int real_opcode) {
    int regnum = real_opcode % 8;
    int thru_carry = !!((real_opcode >> 4) == 1);
    uint8_t operand = get_reg_by_cb_encoding_num(cpu, regnum);

    uint8_t fin = operand << 1;
    if (thru_carry) {
        fin |= (cpu->regs.CPSR & FLAG_CARRY) >> 4;
    } else {
        fin |= (operand & 0x80) >> 7;
    }
    cpu->regs.CPSR = ((operand & 0x80) >> 3) | (fin? 0 : FLAG_ZERO);
    put_reg_by_cb_encoding_num(cpu, regnum, fin);
}

// rrc, rr
static void IMP_rot_right_e(bc_cpu_t *cpu, int opcode, int cycle, int real_opcode) {
    int regnum = real_opcode % 8;
    int thru_carry = !!((real_opcode >> 4) == 1);
    uint8_t operand = get_reg_by_cb_encoding_num(cpu, regnum);

    uint8_t fin = operand >> 1;
    if (thru_carry) {
        fin |= (cpu->regs.CPSR & FLAG_CARRY) << 3;
    } else {
        fin |= (operand & 1) << 7;
    }
    cpu->regs.CPSR = ((operand & 1) << 4) | (fin? 0 : FLAG_ZERO);
    put_reg_by_cb_encoding_num(cpu, regnum, fin);
}

// sla
static void IMP_rot_left_fill(bc_cpu_t *cpu, int opcode, int cycle, int real_opcode) {
    int regnum = real_opcode % 8;
    uint8_t operand = get_reg_by_cb_encoding_num(cpu, regnum);
    // make sure bottom bit is clear
    uint8_t fin = (operand << 1) & 0xfe;
    // top bit into carry
    cpu->regs.CPSR = ((operand & 0x80)? FLAG_CARRY : 0) | (fin? 0 : FLAG_ZERO);
    put_reg_by_cb_encoding_num(cpu, regnum, fin);
}

// sra, srl
static void IMP_rot_right_rep(bc_cpu_t *cpu, int opcode, int cycle, int real_opcode) {
    int regnum = real_opcode % 8;
    uint8_t operand = get_reg_by_cb_encoding_num(cpu, regnum);

    int rep;
    if (real_opcode >> 4 == 3) {
        rep = 0;
    } else {
        rep = operand & 0x80;
    }
    
    uint8_t fin = ((operand >> 1) & 0x7f) | rep;
    // bottom bit into carry
    cpu->regs.CPSR = ((operand & 1)? FLAG_CARRY : 0) | (fin? 0 : FLAG_ZERO);
    put_reg_by_cb_encoding_num(cpu, regnum, fin);
}

static void IMP_swap_nibble(bc_cpu_t *cpu, int opcode, int cycle, int real_opcode) {
    int regnum = real_opcode % 8;
    uint8_t operand = get_reg_by_cb_encoding_num(cpu, regnum);

    uint8_t fin = ((operand & 0xf) << 4) | (operand >> 4);
    
    cpu->regs.CPSR = (fin? 0 : FLAG_ZERO);
    put_reg_by_cb_encoding_num(cpu, regnum, fin);
}

static void IMP_insn_bit_sel(bc_cpu_t *cpu, int opcode, int cycle, int real_opcode) {
    int bit_num = (real_opcode - 0x40) / 8;
    int regnum = real_opcode % 8;
    uint8_t cpsr = FLAG_HALF_CARRY | (cpu->regs.CPSR & FLAG_CARRY);
    uint8_t regval = get_reg_by_cb_encoding_num(cpu, regnum);

    if (!(regval & (1 << bit_num))) {
        cpsr |= FLAG_ZERO;
    }
    cpu->regs.CPSR = cpsr;
}

#define DO_BSET(reg) \
    if (is_set) {                        \
        cpu->regs.reg |= (1 << bit_num); \
    } else {                             \
        cpu->regs.reg &= ~(1 << bit_num);  \
    }
static void IMP_insn_set_res(bc_cpu_t *cpu, int opcode, int cycle, int real_opcode) {
    int hi = real_opcode >> 4;
    int is_set = !!(hi > 0xb);

    int bit_num = (real_opcode / 8) % 8;
    int regnum = real_opcode % 8;
    switch (regnum) {
    case 0: DO_BSET(B); break;
    case 1: DO_BSET(C); break;
    case 2: DO_BSET(D); break;
    case 3: DO_BSET(E); break;
    case 4: DO_BSET(H); break;
    case 5: DO_BSET(L); break;
    case 6: {
        uint16_t adr = bc_regs_getpairvalue(&cpu->regs, PAIR_HL);
        uint8_t val = bc_mmap_getvalue(&cpu->mem, adr);
        if (is_set) {
            bc_mmap_putvalue(&cpu->mem, adr, val |= (1 << bit_num));
        } else {
            bc_mmap_putvalue(&cpu->mem, adr, val &= ~(1 << bit_num));
        }
        break;
    }
    case 7: DO_BSET(A); break;
    }
}
#undef DO_BSET