#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include "emucore/emucore.h"

static bc_cpu_t *global_cpu;

void panic(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    printf("\npanic(");
    vprintf(fmt, ap);
    printf(")\n");

    va_end(ap);

    printf("registers:\n");
    printf("\tA: $%02x (%d)\tB: $%02x (%d)\tC: $%02x (%d)\n",
        global_cpu->regs.A, global_cpu->regs.A,
        global_cpu->regs.B, global_cpu->regs.B,
        global_cpu->regs.C, global_cpu->regs.C);
    printf("\tD: $%02x (%d)\tE: $%02x (%d)\tH: $%02x (%d)\n",
        global_cpu->regs.D, global_cpu->regs.D,
        global_cpu->regs.E, global_cpu->regs.E,
        global_cpu->regs.H, global_cpu->regs.H);
    printf("\tL: $%02x (%d)\n", global_cpu->regs.L, global_cpu->regs.L);
    printf("\tSP: $%04x\tPC: $%04x\tCPSR: $%02x\n",
        global_cpu->regs.SP, global_cpu->regs.PC, global_cpu->regs.CPSR);

    exit(1);
}

int main(int argc, char const *argv[]) {
    global_cpu = bc_cpu_init();
    bc_cpu_reset(global_cpu);

    while (!global_cpu->stop) {
        bc_cpu_step(global_cpu, 16384);
        usleep(16667);
    }

    bc_cpu_release(global_cpu);
    return 0;
}
