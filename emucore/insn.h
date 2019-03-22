#include "emucore.h"
#include <stdint.h>

#define INSN_ENTRY(opcode, nparam, ncycle, executor) ((insn_desc_t){opcode, nparam, ncycle, executor})

typedef void (*insn_execute_t)(bc_cpu_t *, insn_desc_t *, uint16_t);
typedef struct instruction {
    uint8_t opcode;
    uint8_t param_count;
    uint8_t ncycles;
    insn_execute_t executor;
} insn_desc_t;
