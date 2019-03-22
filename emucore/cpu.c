#include <stdlib.h>
#include <string.h>
#include "emucore.h"

bc_cpu_t *bc_cpu_init(void) {
    bc_cpu_t *ret = malloc(sizeof(bc_cpu_t));
    memset(&ret->regs, 0, sizeof(cpu_regs_t));
    bc_mmap_alloc(&ret->mem);
    return ret;
}

void bc_cpu_reset(void) {
    
}