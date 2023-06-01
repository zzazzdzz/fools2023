#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef char s8;
typedef short s16;
typedef int s32;

#include "machine.c" /* notlikethis */

void machine_debug_state(machine_t *machine) {
    puts("-----");
    printf(">> R0=%.4X R1=%.4X R2=%.4X R3=%.4X\n", machine->regs[0], machine->regs[1], machine->regs[2], machine->regs[3]);
    printf(">> SP=%.4X IP=%.4X SW=%.2X\n", machine->sp, machine->ip, machine->control);
    puts("-----");
}

void machine_run(machine_t *machine, int cycles) {
    for (int i=0; i<cycles; i++) {
        u8 opcode = machine->mem[machine->ip];
        if (machine->dbg) printf("** executing opcode 0x%.2x [%s] ip=%.4x\n", opcode, opcode_names[opcode], machine->ip);
        opcode_callbacks[opcode](machine);
        if (machine->dbg) machine_debug_state(machine);
    }
}

machine_t main_machine;

int main(int argc, char **argv) {

    // load initial state
    FILE *fp;
    fp = fopen("challenges/infsystem.bin", "rb");
    fread(main_machine.mem+0xf000, 0x1000, 1, fp);
    fclose(fp);
    fp = fopen("challenges/bios.bin", "rb");
    fread(main_machine.mem+0x0000, 0x1000, 1, fp);
    fclose(fp);

    main_machine.dbg = 0;
    if (argc > 1) main_machine.dbg = 1;

    main_machine.ip = 0xf000;
    main_machine.fs_base = "infsystem_fs/";
    for (;;) {
        machine_run(&main_machine, 1000);
        usleep(1000);
    }
}