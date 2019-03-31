#include "emucore.h"
#include "insn.h"

static void IMP_undefined_instruction(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    panic("Undefined opcode 0x%02x", opcode);
}

static void IMP_nop(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    return;
}

/* 8-bit transfer. */

#define LOAD_REG(REG) transfer = ( cpu->regs.REG ); break
#define LOAD_IPR(HI, LO) transfer = ( bc_mmap_getvalue(&cpu->mem, ((cpu->regs.HI << 8) | cpu->regs.LO)) );
#define STORE_REG(REG) cpu->regs.REG = transfer
#define STORE_IPR(HI, LO) bc_mmap_putvalue(&cpu->mem, ((cpu->regs.HI << 8) | cpu->regs.LO), transfer)

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
    case 0x6: LOAD_IPR(H, L); break;
    case 0x7: LOAD_REG(A);
    case 0x8: LOAD_REG(B);
    case 0x9: LOAD_REG(C);
    case 0xA: LOAD_REG(D);
    case 0xB: LOAD_REG(E);
    case 0xC: LOAD_REG(H);
    case 0xD: LOAD_REG(L);
    case 0xE: LOAD_IPR(H, L); break;
    case 0xF: LOAD_REG(A);
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
    int hi = opcode >> 4;
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
    int hi = opcode >> 4;
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

static void IMP_insn_ld_16s(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    switch(opcode) {
    case 0x01: bc_regs_putpairvalue(&cpu->regs, PAIR_BC, param); break;
    case 0x11: bc_regs_putpairvalue(&cpu->regs, PAIR_DE, param); break;
    case 0x21: bc_regs_putpairvalue(&cpu->regs, PAIR_HL, param); break;
    case 0x31: cpu->regs.SP = param; break;

    case 0x08: bc_mmap_putvalue16(&cpu->mem, param, cpu->regs.SP); break;
    case 0xf9: cpu->regs.SP = bc_regs_getpairvalue(&cpu->regs, PAIR_HL); break;
    }
}

/* Data Processing */

#define LOAD_REG(DST, REG) DST = ( cpu->regs.REG ); break
#define GET_HL_INDIRECT() bc_mmap_getvalue(&cpu->mem, bc_regs_getpairvalue(&cpu->regs, PAIR_HL))
static void IMP_insn_add8(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    int lo = opcode & 0xF;

    int add_val;
    switch(lo & 0x7) {
    case 0x0: LOAD_REG(add_val, B);
    case 0x1: LOAD_REG(add_val, C);
    case 0x2: LOAD_REG(add_val, D);
    case 0x3: LOAD_REG(add_val, E);
    case 0x4: LOAD_REG(add_val, H);
    case 0x5: LOAD_REG(add_val, L);
    case 0x6: add_val = ((opcode >> 4) > 0xb)? param : GET_HL_INDIRECT(); break;
    case 0x7: LOAD_REG(add_val, A);
    }

    if (lo >= 0x8) {
        // this is an ADC inst
        int cflag = bc_regs_test_cpsr_flag(&cpu->regs, FLAG_CARRY);
        if (((cpu->regs.A & 0xf) + (add_val & 0xf) + cflag) & 0x10) {
            cpu->regs.CPSR |= FLAG_HALF_CARRY;
        } else {
            cpu->regs.CPSR &= ~FLAG_HALF_CARRY;
        }
        add_val += cpu->regs.A + cflag;
    } else {
        if (((cpu->regs.A & 0xf) + (add_val & 0xf)) & 0x10) {
            cpu->regs.CPSR |= FLAG_HALF_CARRY;
        } else {
            cpu->regs.CPSR &= ~FLAG_HALF_CARRY;
        }
        add_val += cpu->regs.A;
    }

    if (add_val > 255) {
        cpu->regs.CPSR |= FLAG_CARRY;
    } else {
        cpu->regs.CPSR &= ~FLAG_CARRY;
    }

    if ((add_val & 0xFF) == 0) {
        cpu->regs.CPSR |= FLAG_ZERO;
    } else {
        cpu->regs.CPSR &= ~FLAG_ZERO;
    }

    cpu->regs.CPSR &= ~FLAG_NEGATIVE;
    cpu->regs.A = add_val;
}

// half carry handling based on this thread:
// https://www.reddit.com/r/EmuDev/comments/4ycoix/a_guide_to_the_gameboys_halfcarry_flag/d6nqcc6/
static void IMP_insn_sub8(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    int hi = (opcode >> 4) & 0xF;
    int lo = opcode        & 0xF;

    int add_val;
    switch(lo & 0x7) {
    case 0x0: LOAD_REG(add_val, B);
    case 0x1: LOAD_REG(add_val, C);
    case 0x2: LOAD_REG(add_val, D);
    case 0x3: LOAD_REG(add_val, E);
    case 0x4: LOAD_REG(add_val, H);
    case 0x5: LOAD_REG(add_val, L);
    case 0x6: add_val = ((opcode >> 4) > 0xb)? param : GET_HL_INDIRECT(); break;
    case 0x7: LOAD_REG(add_val, A);
    }

    int answer;
    int with_carry = 0;
    if ((hi == 0x9 && lo >= 0x8) || opcode == 0xDE) {
        with_carry = 1;
    }
    int is_cp = 0;
    if (hi == 0xB || opcode == 0xFE) {
        is_cp = 1;
    }

    if (with_carry) {
        // this is an SBC inst
        int cflag = bc_regs_test_cpsr_flag(&cpu->regs, FLAG_CARRY);
        if (((cpu->regs.A & 0xf) - (add_val & 0xf) - cflag) < 0) {
            cpu->regs.CPSR |= FLAG_HALF_CARRY;
        } else {
            cpu->regs.CPSR &= ~FLAG_HALF_CARRY;
        }
        answer = cpu->regs.A - add_val - cflag;
    } else {
        if (((cpu->regs.A & 0xf) - (add_val & 0xf)) < 0) {
            cpu->regs.CPSR |= FLAG_HALF_CARRY;
        } else {
            cpu->regs.CPSR &= ~FLAG_HALF_CARRY;
        }
        answer = cpu->regs.A - add_val;
    }

    if (answer < 0) {
        cpu->regs.CPSR |= FLAG_CARRY;
    } else {
        cpu->regs.CPSR &= ~FLAG_CARRY;
    }

    if ((answer & 0xFF) == 0) {
        cpu->regs.CPSR |= FLAG_ZERO;
    } else {
        cpu->regs.CPSR &= ~FLAG_ZERO;
    }

    cpu->regs.CPSR |= FLAG_NEGATIVE;
    if (!is_cp) {
        cpu->regs.A = bc_convertsignedvalue(answer);
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
    case 0x6: add_val = (opcode == 0xe6)? param : GET_HL_INDIRECT(); break;
    case 0x7: LOAD_REG(add_val, A);
    }

    uint8_t wb = cpu->regs.A & add_val;
    if (!wb) {
        cpu->regs.CPSR = FLAG_ZERO | FLAG_HALF_CARRY;
    } else {
        cpu->regs.CPSR = FLAG_HALF_CARRY;
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
    case 0x6: add_val = (opcode == 0xee)? param : GET_HL_INDIRECT(); break;
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
    case 0x6: add_val = (opcode == 0xf6)? param : GET_HL_INDIRECT(); break;
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

static void IMP_insn_incr8(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    int add_val;
    uint8_t ic = (opcode & 1)? ((~1) + 1) : 1;

    switch(opcode & (~1)) {
    case 0x04: LOAD_REG(add_val, B);
    case 0x14: LOAD_REG(add_val, D);
    case 0x24: LOAD_REG(add_val, H);
    case 0x34: add_val = GET_HL_INDIRECT(); break;
    case 0x0c: LOAD_REG(add_val, C);
    case 0x1c: LOAD_REG(add_val, E);
    case 0x2c: LOAD_REG(add_val, L);
    case 0x3c: LOAD_REG(add_val, A);
    }

    if (((add_val & 0xf) + ic) & 0x10) {
        cpu->regs.CPSR |= FLAG_HALF_CARRY;
    } else {
        cpu->regs.CPSR &= ~FLAG_HALF_CARRY;
    }

    add_val += ic;

    if ((add_val & 0xFF) == 0) {
        cpu->regs.CPSR |= FLAG_ZERO;
    } else {
        cpu->regs.CPSR &= ~FLAG_ZERO;
    }

    if (opcode & 1) {
        cpu->regs.CPSR |= FLAG_NEGATIVE;
    } else {
        cpu->regs.CPSR &= ~FLAG_NEGATIVE;
    }

    switch(opcode & (~1)) {
    case 0x04: cpu->regs.B = add_val; break;
    case 0x14: cpu->regs.D = add_val; break;
    case 0x24: cpu->regs.H = add_val; break;
    case 0x34: bc_mmap_putvalue(&cpu->mem, bc_regs_getpairvalue(&cpu->regs, PAIR_HL), add_val); break;
    case 0x0c: cpu->regs.C = add_val; break;
    case 0x1c: cpu->regs.E = add_val; break;
    case 0x2c: cpu->regs.L = add_val; break;
    case 0x3c: cpu->regs.A = add_val; break;
    }
}
#undef LOAD_REG
#undef GET_HL_INDIRECT

// Now based on http://forums.nesdev.com/viewtopic.php?t=9088 and actually passes tests
static void IMP_insn_daa(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    uint8_t cpsr = cpu->regs.CPSR;
    uint8_t new_cpsr = cpsr & ~(FLAG_HALF_CARRY | FLAG_ZERO);
    int val = cpu->regs.A;

    if (cpsr & FLAG_NEGATIVE) {
        if (cpsr & FLAG_HALF_CARRY) {
            val = (val - 6) & 0xff;
        }
        if (cpsr & FLAG_CARRY) {
            val -= 0x60;
        }
    } else {
        if (cpsr & FLAG_HALF_CARRY || (val & 0xf) > 9) {
            val += 0x6;
        }
        if (cpsr & FLAG_CARRY || val > 0x9f) {
            val += 0x60;
        }
    }

    if (val > 0xFF) {
        new_cpsr |= FLAG_CARRY;
    }
    if ((val & 0xFF) == 0) {
        new_cpsr |= FLAG_ZERO;
    }

    cpu->regs.CPSR = new_cpsr;
    cpu->regs.A = val & 0xFF;
}

/* 16-Bit Arithmetic */

static void IMP_insn_incdec16(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    int ic = ((opcode & 0xf) == 0xB)? -1 : 1;
    int pair;

    switch(opcode >> 4) {
    case 0x0: pair = PAIR_BC; break;
    case 0x1: pair = PAIR_DE; break;
    case 0x2: pair = PAIR_HL; break;
    case 0x3: pair = -1; break;
    }

    if (pair >= 0) {
        int add_val = bc_regs_getpairvalue(&cpu->regs, pair);
        bc_regs_putpairvalue(&cpu->regs, pair, add_val + ic);
    } else {
        cpu->regs.SP += ic;
    }
}

static void IMP_insn_add_regs(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    int add_val;
    int dst = bc_regs_getpairvalue(&cpu->regs, PAIR_HL);
    switch(opcode >> 4) {
    case 0x0: add_val = bc_regs_getpairvalue(&cpu->regs, PAIR_BC); break;
    case 0x1: add_val = bc_regs_getpairvalue(&cpu->regs, PAIR_DE); break;
    case 0x2: add_val = bc_regs_getpairvalue(&cpu->regs, PAIR_HL); break;
    case 0x3: add_val = cpu->regs.SP; break;
    }

    uint8_t new_cpsr = cpu->regs.CPSR & FLAG_ZERO;

    if (((dst & 0xfff) + (add_val & 0xfff)) & 0x1000) {
        new_cpsr |= FLAG_HALF_CARRY;
    }

    add_val += dst;

    if (add_val > 0xFFFF) {
        new_cpsr |= FLAG_CARRY;
    }
    
    cpu->regs.CPSR = new_cpsr;
    bc_regs_putpairvalue(&cpu->regs, PAIR_HL, add_val);
}

static void IMP_insn_add_sp_imm8(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    int add_val = param; // bc_convertunsignedvalue(param);
    uint8_t new_cpsr = 0;

    if (add_val < 0) {
        if (((int)(cpu->regs.SP & 0xfff) + add_val) < 0) {
            new_cpsr |= FLAG_HALF_CARRY;
        }
    } else {
        if (((cpu->regs.SP & 0xfff) + add_val) & 0x1000) {
            new_cpsr |= FLAG_HALF_CARRY;
        }
    }

    add_val = cpu->regs.SP + add_val;

    if (add_val > 0xFFFF || add_val < 0) {
        new_cpsr |= FLAG_CARRY;
    }
    
    cpu->regs.CPSR = new_cpsr;
    cpu->regs.SP = bc_convertsignedvalue16(add_val);
}

static void IMP_insn_mov_sp_to_hl(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    int is_sub = 0;
    int add_val = bc_convertunsignedvalue(param);
    if (add_val < 0) {
        add_val = -add_val;
        is_sub = 1;
    }

    uint8_t new_cpsr = 0;

    if (is_sub) {
        if (((int)(cpu->regs.SP & 0xfff) - add_val) < 0) {
            new_cpsr |= FLAG_HALF_CARRY;
        }
        add_val = cpu->regs.SP - add_val;
    } else {
        if (((cpu->regs.SP & 0xfff) + add_val) & 0x1000) {
            new_cpsr |= FLAG_HALF_CARRY;
        }
        add_val = cpu->regs.SP + add_val;
    }

    if (add_val > 0xFFFF || add_val < 0) {
        new_cpsr |= FLAG_CARRY;
    }

    cpu->regs.CPSR = new_cpsr;
    bc_regs_putpairvalue(&cpu->regs, PAIR_HL, add_val & 0xFFFF);
}

/* Push/pop */

static void IMP_push_pair(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    int pair; 

    switch(opcode >> 4) {
    case 0xc: pair = PAIR_BC; break;
    case 0xd: pair = PAIR_DE; break;
    case 0xe: pair = PAIR_HL; break;
    case 0xf: pair = PAIR_AF; break;
    }

    bc_mmap_putstack16(&cpu->mem, bc_regs_getpairvalue(&cpu->regs, pair));
}

static void IMP_pop_pair(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    int pair; 

    switch(opcode >> 4) {
    case 0xc: pair = PAIR_BC; break;
    case 0xd: pair = PAIR_DE; break;
    case 0xe: pair = PAIR_HL; break;
    case 0xf: pair = PAIR_AF; break;
    }

    bc_regs_putpairvalue(&cpu->regs, pair, bc_mmap_popstack16(&cpu->mem));
}

/* Rotates and bitsets */

static void IMP_cpl(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    cpu->regs.CPSR |= FLAG_NEGATIVE | FLAG_HALF_CARRY;
    cpu->regs.A ^= 0xFF;
}

static void IMP_ccf(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    uint8_t new_cpsr = cpu->regs.CPSR & (FLAG_ZERO | FLAG_CARRY);
    new_cpsr ^= FLAG_CARRY;
    cpu->regs.CPSR = new_cpsr;
}

static void IMP_scf(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    uint8_t new_cpsr = cpu->regs.CPSR & FLAG_ZERO;
    new_cpsr |= FLAG_CARRY;
    cpu->regs.CPSR = new_cpsr;
}

static void IMP_rot_left(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    int thru_carry = ((opcode >> 4) == 1);

    uint8_t fin = cpu->regs.A << 1;
    if (thru_carry) {
        fin |= (cpu->regs.CPSR & FLAG_CARRY) >> 4;
    } else {
        fin |= (cpu->regs.A & 0x80) >> 7;
    }
    cpu->regs.CPSR = (cpu->regs.A & 0x80) >> 3;
    cpu->regs.A = fin;
}

static void IMP_rot_right(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    int thru_carry = ((opcode >> 4) == 1);

    uint8_t fin = cpu->regs.A >> 1;
    if (thru_carry) {
        fin |= (cpu->regs.CPSR & FLAG_CARRY) << 3;
    } else {
        fin |= (cpu->regs.A & 1) << 7;
    }
    cpu->regs.CPSR = (cpu->regs.A & 1) << 4;
    cpu->regs.A = fin;
}

/* Jumps */

static void IMP_rst_vec(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    uint16_t target;
    switch (opcode) {
    case 0xc7:
        target = 0x00; break;
    case 0xd7:
        target = 0x10; break;
    case 0xe7:
        target = 0x20; break;
    case 0xf7:
        target = 0x30; break;
    case 0xcf:
        target = 0x08; break;
    case 0xdf:
        target = 0x18; break;
    case 0xef:
        target = 0x28; break;
    case 0xff:
        target = 0x38; break;
    default:
        panic("should never get here");
    }

    bc_mmap_putstack16(&cpu->mem, cpu->regs.PC);
    cpu->regs.PC = target;
}

#define MATCH_CPSR(m, w) ({ mask = m; what = w; }); break
static void IMP_jump_rel(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    int mask;
    int what;

    switch(opcode) {
        case 0x20: MATCH_CPSR(FLAG_ZERO, 0);
        case 0x30: MATCH_CPSR(FLAG_CARRY, 0);

        case 0x18: MATCH_CPSR(0, 0); // unconditional
        case 0x28: MATCH_CPSR(FLAG_ZERO, FLAG_ZERO);
        case 0x38: MATCH_CPSR(FLAG_CARRY, FLAG_CARRY);
    }

    if ((cpu->regs.CPSR & mask) == what) {
        int8_t go = bc_convertunsignedvalue(param);
        cpu->regs.PC += go;
        // If a branch is taken, this instruction takes 12 clocks.
        // If not, it only takes 8.
        cpu->cycles_for_stall = 4;
    
        // debug_log("%x %x", opcode, go);

        if (go == -2) {
            // FIXME: remove this once done debugging!
            panic("infinite loop");
        }
        // getchar();
    }
}

static void IMP_jump_absolute(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    int mask;
    int what;

    switch(opcode) {
        case 0xc2: MATCH_CPSR(FLAG_ZERO, 0);
        case 0xd2: MATCH_CPSR(FLAG_CARRY, 0);
        case 0xe9: // mov pc, hl
        case 0xc3: MATCH_CPSR(0, 0); // unconditional
        case 0xca: MATCH_CPSR(FLAG_ZERO, FLAG_ZERO);
        case 0xda: MATCH_CPSR(FLAG_CARRY, FLAG_CARRY);
    }

    // debug_log("%x -> %x", opcode, param);
    // getchar();
    if (opcode == 0xe9) {
        cpu->regs.PC = bc_regs_getpairvalue(&cpu->regs, PAIR_HL);
    } else if ((cpu->regs.CPSR & mask) == what) {
        cpu->regs.PC = param;
        // If a branch is taken, this instruction takes 16 clocks.
        // If not, it only takes 12.
        cpu->cycles_for_stall = 4;
    }
}

static void IMP_call_ret_absolute(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    int mask;
    int what;
    int is_ret = 0;

    switch(opcode) {
        case 0xc0: is_ret = 1; // fall through
        case 0xc4: MATCH_CPSR(FLAG_ZERO, 0);
        case 0xd0: is_ret = 1;
        case 0xd4: MATCH_CPSR(FLAG_CARRY, 0);
        case 0xc9: is_ret = 1;
        case 0xcd: MATCH_CPSR(0, 0); // unconditional
        case 0xc8: is_ret = 1;
        case 0xcc: MATCH_CPSR(FLAG_ZERO, FLAG_ZERO);
        case 0xd8: is_ret = 1;
        case 0xdc: MATCH_CPSR(FLAG_CARRY, FLAG_CARRY);
    }

    if ((cpu->regs.CPSR & mask) == what) {
        if (is_ret) {
            int addr = bc_mmap_popstack16(&cpu->mem);
            cpu->regs.PC = addr;
        } else {
            bc_mmap_putstack16(&cpu->mem, cpu->regs.PC);
            cpu->regs.PC = param;
        }
        // If a branch is taken, this instruction takes 24/20 clocks.
        // If not, it only takes 12/8.
        // debug_log("%x %x; PC now %x", opcode, param, cpu->regs.PC);
        // getchar();
        cpu->cycles_for_stall = 12;
    }
}
#undef MATCH_CPSR

static void IMP_reti(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    cpu->irq_mask |= IF_MASTER;
    int addr = bc_mmap_popstack16(&cpu->mem);
    cpu->regs.PC = addr;
}

static void IMP_edi(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    if (opcode == 0xf3) {
        cpu->irq_mask &= ~IF_MASTER;
    } else {
        cpu->irq_mask |= IF_MASTER;
    }
}

static void IMP_stop(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    if (opcode == 0x10) {
        panic("CPU stopped");
    } else {
        cpu->halt = 1;
    }
}

#define INSTRUCTION_TABLE 1
#include "insn_table.h"