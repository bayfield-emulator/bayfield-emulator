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
#define LOAD_IPR(HI, LO) transfer = ( bc_mmap_getvalue(&cpu->mem, ((cpu->regs.HI << 8) | cpu->regs.LO)) ); break
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

#define LOAD_REG(DST, REG) DST = ( cpu->regs.REG ); break
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
        cpu->regs.A = bc_convertsignedvalue(add_val);
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

static void IMP_insn_incr8(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    int add_val;
    switch(opcode) {
    case 0x04: LOAD_REG(add_val, B);
    case 0x14: LOAD_REG(add_val, D);
    case 0x24: LOAD_REG(add_val, H);
    case 0x34: add_val = GET_HL_INDIRECT(); break;
    case 0x0c: LOAD_REG(add_val, C);
    case 0x1c: LOAD_REG(add_val, E);
    case 0x2c: LOAD_REG(add_val, L);
    case 0x3c: LOAD_REG(add_val, A);
    }

    if ((1 + (add_val & 0xf)) & 0x10) {
        cpu->regs.CPSR |= FLAG_HALF_CARRY;
    } else {
        cpu->regs.CPSR &= FLAG_HALF_CARRY;
    }

    add_val++;

    if ((add_val & 0xFF) == 0) {
        cpu->regs.CPSR |= FLAG_ZERO;
    } else {
        cpu->regs.CPSR &= FLAG_ZERO;
    }

    cpu->regs.CPSR &= FLAG_NEGATIVE;
    cpu->regs.A = add_val;
    switch(opcode) {
    case 0x04: cpu->regs.B = add_val;
    case 0x14: cpu->regs.D = add_val;
    case 0x24: cpu->regs.H = add_val;
    case 0x34: bc_mmap_putvalue(&cpu->mem, bc_regs_getpairvalue(&cpu->regs, PAIR_HL), add_val); break;
    case 0x0c: cpu->regs.C = add_val;
    case 0x1c: cpu->regs.E = add_val;
    case 0x2c: cpu->regs.L = add_val;
    case 0x3c: cpu->regs.A = add_val;
    }
}

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

static void IMP_edi(bc_cpu_t *cpu, int opcode, int cycle, int param) {
    if (opcode == 0xf3) {
        cpu->irq_mask &= ~IF_MASTER;
    } else {
        cpu->irq_mask |= IF_MASTER;
    }
}

#define INSTRUCTION_TABLE 1
#include "insn_table.h"