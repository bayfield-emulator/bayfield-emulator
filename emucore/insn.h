#ifndef EMUCORE_INSN_H
#define EMUCORE_INSN_H

#include "emucore.h"
#include <stdint.h>

#define INSN_blank(opcode, nparam, ncycle, executor) ((insn_desc_t){opcode, 0, ncycle, IMP_undefined_instruction})
#define INSN_ENTRY(opcode, nparam, ncycle, executor) ((insn_desc_t){opcode, nparam, ncycle, executor})

typedef struct lr35902 bc_cpu_t;
typedef struct instruction insn_desc_t;

typedef void (*insn_execute_t)(bc_cpu_t *, int, int, int);
typedef struct instruction {
    uint8_t opcode;
    uint8_t param_count;
    uint8_t ncycles;
    insn_execute_t executor;
} insn_desc_t;

// The master instruction array
insn_desc_t const instructions[256];

const insn_desc_t *select_extended_instruction(int opcode);

#endif