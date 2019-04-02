#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "emucore.h"

static bc_cpu_t *global_cpu;
static char serial_port[41];
static int serial_ptr = 0;

const char *convert_cpsr(int cpsr) {
    static char s[5] = {0};
    if (!s[0]) {
        memcpy(s, "ZNHC", 4);
    }
    s[0] = (cpsr & FLAG_ZERO)? 'Z' : 'z';
    s[1] = (cpsr & FLAG_NEGATIVE)? 'N' : 'n';
    s[2] = (cpsr & FLAG_HALF_CARRY)? 'H' : 'h';
    s[3] = (cpsr & FLAG_CARRY)? 'C' : 'c';
    return s;
}

void dump_regs() {
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
    printf("\tSP: $%04x\tPC: $%04x\tCPSR: $%02x (%s)\n",
        global_cpu->regs.SP, global_cpu->regs.PC, global_cpu->regs.CPSR,
        convert_cpsr(global_cpu->regs.CPSR));
}

uint8_t write_serial(bc_cpu_t *cpu, void *context, uint16_t addr, uint8_t write_val) {
    static int did_first_header;

    if (write_val != 0x81) {
        return 0;
    }

    serial_port[serial_ptr++] = bc_mmap_getvalue(&cpu->mem, 0xFF01);
    if (serial_ptr == 40 || serial_port[serial_ptr - 1] == '\n') {
        if (!did_first_header) {
            printf("SERIAL OUT: ");
            did_first_header = 1;
        }
        for (int i = 0; i < 40; ++i) {
            if (serial_port[i] == '\n') {
                printf("\nSERIAL OUT: ");
                break;
            }
            printf("%c", (unsigned char)serial_port[i]);
        }
        serial_ptr = 0;
    }

    return 0;
}

uint8_t write_serial_data(bc_cpu_t *cpu, void *context, uint16_t addr, uint8_t write_val) {
    // printf("serial: %02x %c\n", write_val, write_val & 0x7f);
    // dump_regs();
    // getchar();
    return write_val;
}

void panic(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    printf("\npanic(");
    vprintf(fmt, ap);
    printf(")\n");

    va_end(ap);

    dump_regs();

    FILE *f = fopen("emumem.dmp", "wb");
    fwrite(global_cpu->mem.all_ram, 32768, 1, f);
    fclose(f);

    printf("Dumped emucore's ram to emumem.dmp!\n");
    exit(1);
}

void setup_rom(bc_cpu_t *cpu, int fd) {
    // Get entire image
    size_t sz = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, 0);
    uint8_t *mem = malloc(sz); 
    read(fd, mem, sz);

    cpu->mem.rom.image_size = sz;
    cpu->mem.rom.rom = mem;
    cpu->mem.rom.bank1 = mem;
    cpu->mem.rom.bankx = mem + 16384;

    int mbc = mem[0x0147];
    printf("setup_rom:\n");
    printf("\ttitle: %16s\n", mem + 0x0134);
    printf("\tmbc: %d\n", mem[0x0147]);

    cpu->mem.rom.mbc_type = mbc;
}

void setup_vram(bc_cpu_t *cpu) {
    cpu->mem.vram = malloc(0x20A0);
    cpu->mem.oam = cpu->mem.vram + 0x2000;
}

int main(int argc, char const *argv[]) {
    global_cpu = bc_cpu_init();

    int fd = open(argv[1], O_RDONLY);
    if (!fd) {
        fputs("Usage: cputest <rom file>", stderr);
        return 1;
    }
    setup_rom(global_cpu, fd);
    setup_vram(global_cpu);
    close(fd);

    bc_cpu_reset(global_cpu);
    bc_mmap_add_mmio_observer(&global_cpu->mem, 0xFF01, write_serial_data, NULL, NULL);
    bc_mmap_add_mmio_observer(&global_cpu->mem, 0xFF02, write_serial, NULL, NULL);

    int stepping = 0;
    uint16_t pc = global_cpu->regs.PC;
    while (!global_cpu->stop) {
        bc_cpu_step(global_cpu, 4);

        if (global_cpu->regs.PC == 0xc2d6) {
            // stepping = 1;
        }
        
        if (pc != global_cpu->regs.PC && stepping) {
            const char *s = "";
            uint8_t is = bc_mmap_getvalue(&global_cpu->mem, global_cpu->regs.PC);
            if (is == 0xcb) {
                is = bc_mmap_getvalue(&global_cpu->mem, global_cpu->regs.PC);
                s = "CB ";
            }
            printf("\nNext instruction: %s%x\n", s, is);
            dump_regs();
            getchar();
            pc = global_cpu->regs.PC;
        }
    }

    bc_cpu_release(global_cpu);
    return 0;
}
