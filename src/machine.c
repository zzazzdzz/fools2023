
typedef struct {
    u8 mem[0x10000];
    u16 regs[4];
    u16 sp;
    u16 ip;
    u8 control;
    u8 dbg;
    const char *fs_base;
} machine_t;

typedef void (*machine_opcode_t)(machine_t*);
machine_opcode_t opcode_callbacks[];
const char *opcode_names[];
machine_opcode_t opcode_callbacks2[];
const char *opcode_names2[];

#define LOW(x) ((x)&0xff)
#define HIGH(x) (((x)>>8)&0xff)
#define READ_U16(x) ((m->mem[(x+1)])*256+(m->mem[(x)]))
#define READ_U8(x) (m->mem[(x)])
#define SFRD_U16(x) (((x)<0x1000 && m->ip>=0x1000) ? 0xff : (READ_U16(x)))
#define SFRD_U8(x) (((x)<0x1000 && m->ip>=0x1000) ? 0xff : (READ_U8(x)))

// brk => 0x00
void machine_opcode_brk(machine_t *machine) {
    machine->sp--;
    machine->mem[machine->sp] = HIGH(machine->ip+1);
    machine->sp--;
    machine->mem[machine->sp] = LOW(machine->ip+1);
    machine->ip = 0xfff0;
}

// mul r0, {imm: u16} => 0x01 @ le(imm)
void machine_opcode_mul_imm(machine_t *machine) {
    machine_t *m = machine;
    u16 result = READ_U16(machine->ip+1) * machine->regs[0];
    machine->regs[0] = result;
    machine->ip += 3;
}

void machine_opcode_mul_r0_r1(machine_t *machine) {
    machine->regs[0] *= machine->regs[1];
    machine->ip += 1;
}

void machine_opcode_div_imm(machine_t *machine) {
    machine_t *m = machine;
    u16 result = machine->regs[0] / READ_U16(machine->ip+1);
    machine->regs[0] = result;
    machine->ip += 3;
}

void machine_opcode_div_r0_r1(machine_t *machine) {
    machine->regs[0] /= machine->regs[1];
    machine->ip += 1;
}

void machine_opcode_ret(machine_t *machine) {
    machine_t *m = machine;
    u16 addr = READ_U16(machine->sp);
    machine->sp += 2;
    machine->ip = addr;
}

#include <math.h>

int sqrt_test_idx = 0;
int sqrt_test_succ = 0;
void machine_opcode_sys(machine_t *m) {
    u8 sysid = READ_U8(m->ip+1);
    if (sysid == 1) {
        putchar(m->regs[0]);
    }
    if (sysid == 2) {
        m->regs[0] = getchar();
    }
    if (sysid == 3) exit(0);
    if (sysid == 4) {
        char *fs_data = malloc(0x1000);
        sprintf(fs_data, "%s%.2x.bin", m->fs_base, m->regs[0]); // oh no
        FILE *fp = fopen(fs_data, "rb");
        memset(fs_data, 0, 0x1000);
        if (fp) {
            fread(fs_data, 0x1000, 1, fp);
        }
        fclose(fp);
        if (m->regs[1] < 0xf000) {
            memcpy(m->mem + m->regs[1], fs_data, 0x1000);
        }
    }
    if (sysid == 5) {
        /*FILE *fp = fopen("/dev/urandom", "rb");
        u16 out;
        fread(&out, 1, 2, fp);
        fclose(fp);*/
        m->regs[0] = 11234;
    }
    if (sysid == 102) {
        sqrt_test_idx = 0;
        sqrt_test_succ = 0;
    }
    if (sysid == 100) {
        if (sqrt_test_idx == 0) { m->regs[0] = 1337; }
        if (sqrt_test_idx == 1) { m->regs[0] = 19490; }
        if (sqrt_test_idx == 2) { m->regs[0] = 40511; }
        if (sqrt_test_idx == 3) { m->regs[0] = 38416; }
        if (sqrt_test_idx == 4) { m->regs[0] = 0; }
        if (sqrt_test_idx == 5) { m->regs[0] = 16445; }
        if (sqrt_test_idx == 6) { m->regs[0] = 12321; }
        if (sqrt_test_idx == 7) { m->regs[0] = 7777; }
    }
    if (sysid == 101) {
        //printf("SQRT TEST idx=%i r=%i\n", sqrt_test_idx, m->regs[0]);
        int resp = m->regs[0];
        m->regs[0] = 2;
        if (sqrt_test_idx == 0 && resp == (int)sqrt(1337)) { sqrt_test_succ++;m->regs[0] = 1; }
        if (sqrt_test_idx == 1 && resp == (int)sqrt(19490)) { sqrt_test_succ++;m->regs[0] = 1; } 
        if (sqrt_test_idx == 2 && resp == (int)sqrt(40511)) { sqrt_test_succ++;m->regs[0] = 1; } 
        if (sqrt_test_idx == 3 && resp == (int)sqrt(38416)) { sqrt_test_succ++;m->regs[0] = 1; } 
        if (sqrt_test_idx == 4 && resp == 0) { sqrt_test_succ++;m->regs[0] = 1; } 
        if (sqrt_test_idx == 5 && resp == (int)sqrt(16445)) { sqrt_test_succ++;m->regs[0] = 1; } 
        if (sqrt_test_idx == 6 && resp == (int)sqrt(12321)) { sqrt_test_succ++;m->regs[0] = 1; } 
        if (sqrt_test_idx == 7 && resp == (int)sqrt(7777)) { sqrt_test_succ++;m->regs[0] = 0;
            if (sqrt_test_succ == 8) {
                puts("Test successful! FOOLS2023_{UnknownArchitectureMath}");
            }
        }
        //printf("succ is %i reg=%i\n", sqrt_test_succ, m->regs[0]);
        sqrt_test_idx++;
    }
    
    m->ip += 2;
}

int current_mix = 0;

u32 rndseed = 0;
void srand2(u32 x) { rndseed = x; }
u16 rand2() {
	rndseed = 214013*rndseed + 2531011;
	return (rndseed >> 16) & 0x7fff;
}

void machine_opcode_mix(machine_t *machine) {
    int i,j;
    srand2(current_mix);
    current_mix++;
    for (i=255; i>0; i--) {
        j = rand2() % i;
        void *temp;
        temp = opcode_callbacks[i];
        opcode_callbacks[i] = opcode_callbacks[j];
        opcode_callbacks[j] = temp;
        temp = opcode_names[i];
        opcode_names[i] = opcode_names[j];
        opcode_names[j] = temp;
    }
    /*puts("new opcodes:");
    for (i=0; i<256; i++) {
        printf("%.2x -> %s\n", i, opcode_names[i]);
    }*/
    machine->ip += 1;
}

void machine_opcode_nop(machine_t *machine) {
    machine->ip += 1;
}

void machine_opcode_mov_sp_imm(machine_t *machine) {
    machine_t *m = machine;
    machine->sp = READ_U16(machine->ip+1);
    machine->ip += 3;
}

void machine_opcode_mov_r0_sp(machine_t *machine) {
    machine->regs[0] = machine->sp;
    machine->ip += 1;
}

void machine_opcode_mov_sp_r0(machine_t *machine) {
    machine->sp = machine->regs[0];
    machine->ip += 1;
}

void machine_opcode_unmix(machine_t *machine) {
    int i;
    for (i=0; i<256; i++) {
        opcode_names[i] = opcode_names2[i];
        opcode_callbacks[i] = opcode_callbacks2[i];
    }
    current_mix=0;
    machine->ip += 1;
}

void machine_opcode_ill(machine_t *machine) {
    puts("illegal opcode!"); for(;;);
    // todo
}

void machine_opcode_shl(machine_t *machine) {
    machine->regs[0] <<= 1;
    machine->ip += 1;
}

void machine_opcode_shr(machine_t *machine) {
    machine->regs[0] >>= 1;
    machine->ip += 1;
}

void machine_opcode_mov_r0_imm(machine_t *machine) {
    machine_t *m = machine;
    machine->regs[0] = READ_U16(machine->ip+1); machine->ip += 3;
}
void machine_opcode_mov_r1_imm(machine_t *machine) {
    machine_t *m = machine;
    machine->regs[1] = READ_U16(machine->ip+1); machine->ip += 3;
}
void machine_opcode_mov_r2_imm(machine_t *machine) {
    machine_t *m = machine;
    machine->regs[2] = READ_U16(machine->ip+1); machine->ip += 3;
}
void machine_opcode_mov_r3_imm(machine_t *machine) {
    machine_t *m = machine;
    machine->regs[3] = READ_U16(machine->ip+1); machine->ip += 3;
}

void machine_debug_state2(machine_t *machine) {
    puts("-----");
    printf(">> R0=%.4X R1=%.4X R2=%.4X R3=%.4X\n", machine->regs[0], machine->regs[1], machine->regs[2], machine->regs[3]);
    printf(">> SP=%.4X IP=%.4X SW=%.2X\n", machine->sp, machine->ip, machine->control);
    puts("-----");
}
void machine_opcode_mov_r0_r0(machine_t *m) { m->regs[0] = m->regs[0]; m->ip++;
    //machine_debug_state2(m);
    //char x;scanf("%c", &x);
}
void machine_opcode_mov_r0_r1(machine_t *m) { m->regs[0] = m->regs[1]; m->ip++; }
void machine_opcode_mov_r0_r2(machine_t *m) { m->regs[0] = m->regs[2]; m->ip++; }
void machine_opcode_mov_r0_r3(machine_t *m) { m->regs[0] = m->regs[3]; m->ip++; }
void machine_opcode_mov_r1_r0(machine_t *m) { m->regs[1] = m->regs[0]; m->ip++; }
void machine_opcode_mov_r1_r1(machine_t *m) { m->regs[1] = m->regs[1]; m->ip++; }
void machine_opcode_mov_r1_r2(machine_t *m) { m->regs[1] = m->regs[2]; m->ip++; }
void machine_opcode_mov_r1_r3(machine_t *m) { m->regs[1] = m->regs[3]; m->ip++; }
void machine_opcode_mov_r2_r0(machine_t *m) { m->regs[2] = m->regs[0]; m->ip++; }
void machine_opcode_mov_r2_r1(machine_t *m) { m->regs[2] = m->regs[1]; m->ip++; }
void machine_opcode_mov_r2_r2(machine_t *m) { m->regs[2] = m->regs[2]; m->ip++; }
void machine_opcode_mov_r2_r3(machine_t *m) { m->regs[2] = m->regs[3]; m->ip++; }
void machine_opcode_mov_r3_r0(machine_t *m) { m->regs[3] = m->regs[0]; m->ip++; }
void machine_opcode_mov_r3_r1(machine_t *m) { m->regs[3] = m->regs[1]; m->ip++; }
void machine_opcode_mov_r3_r2(machine_t *m) { m->regs[3] = m->regs[2]; m->ip++; }
void machine_opcode_mov_r3_r3(machine_t *m) { m->regs[3] = m->regs[3]; m->ip++; }

void machine_opcode_add_r0_r0(machine_t *m) { m->regs[0] += m->regs[0]; m->ip++; }
void machine_opcode_add_r0_r1(machine_t *m) { m->regs[0] += m->regs[1]; m->ip++; }
void machine_opcode_add_r0_r2(machine_t *m) { m->regs[0] += m->regs[2]; m->ip++; }
void machine_opcode_add_r0_r3(machine_t *m) { m->regs[0] += m->regs[3]; m->ip++; }
void machine_opcode_add_r1_r0(machine_t *m) { m->regs[1] += m->regs[0]; m->ip++; }
void machine_opcode_add_r1_r1(machine_t *m) { m->regs[1] += m->regs[1]; m->ip++; }
void machine_opcode_add_r1_r2(machine_t *m) { m->regs[1] += m->regs[2]; m->ip++; }
void machine_opcode_add_r1_r3(machine_t *m) { m->regs[1] += m->regs[3]; m->ip++; }
void machine_opcode_add_r2_r0(machine_t *m) { m->regs[2] += m->regs[0]; m->ip++; }
void machine_opcode_add_r2_r1(machine_t *m) { m->regs[2] += m->regs[1]; m->ip++; }
void machine_opcode_add_r2_r2(machine_t *m) { m->regs[2] += m->regs[2]; m->ip++; }
void machine_opcode_add_r2_r3(machine_t *m) { m->regs[2] += m->regs[3]; m->ip++; }
void machine_opcode_add_r3_r0(machine_t *m) { m->regs[3] += m->regs[0]; m->ip++; }
void machine_opcode_add_r3_r1(machine_t *m) { m->regs[3] += m->regs[1]; m->ip++; }
void machine_opcode_add_r3_r2(machine_t *m) { m->regs[3] += m->regs[2]; m->ip++; }
void machine_opcode_add_r3_r3(machine_t *m) { m->regs[3] += m->regs[3]; m->ip++; }

void machine_opcode_mov_r0_br0(machine_t *m) { m->regs[0] = SFRD_U8(m->regs[0]); m->ip++; }
void machine_opcode_mov_r0_br1(machine_t *m) { m->regs[0] = SFRD_U8(m->regs[1]); m->ip++; }
void machine_opcode_mov_r0_br2(machine_t *m) { m->regs[0] = SFRD_U8(m->regs[2]); m->ip++; }
void machine_opcode_mov_r0_br3(machine_t *m) { m->regs[0] = SFRD_U8(m->regs[3]); m->ip++; }
void machine_opcode_mov_r1_br0(machine_t *m) { m->regs[1] = SFRD_U8(m->regs[0]); m->ip++; }
void machine_opcode_mov_r1_br1(machine_t *m) { m->regs[1] = SFRD_U8(m->regs[1]); m->ip++; }
void machine_opcode_mov_r1_br2(machine_t *m) { m->regs[1] = SFRD_U8(m->regs[2]); m->ip++; }
void machine_opcode_mov_r1_br3(machine_t *m) { m->regs[1] = SFRD_U8(m->regs[3]); m->ip++; }
void machine_opcode_mov_r2_br0(machine_t *m) { m->regs[2] = SFRD_U8(m->regs[0]); m->ip++; }
void machine_opcode_mov_r2_br1(machine_t *m) { m->regs[2] = SFRD_U8(m->regs[1]); m->ip++; }
void machine_opcode_mov_r2_br2(machine_t *m) { m->regs[2] = SFRD_U8(m->regs[2]); m->ip++; }
void machine_opcode_mov_r2_br3(machine_t *m) { m->regs[2] = SFRD_U8(m->regs[3]); m->ip++; }
void machine_opcode_mov_r3_br0(machine_t *m) { m->regs[3] = SFRD_U8(m->regs[0]); m->ip++; }
void machine_opcode_mov_r3_br1(machine_t *m) { m->regs[3] = SFRD_U8(m->regs[1]); m->ip++; }
void machine_opcode_mov_r3_br2(machine_t *m) { m->regs[3] = SFRD_U8(m->regs[2]); m->ip++; }
void machine_opcode_mov_r3_br3(machine_t *m) { m->regs[3] = SFRD_U8(m->regs[3]); m->ip++; }

void machine_opcode_mov_r0_wr0(machine_t *m) { m->regs[0] = SFRD_U16(m->regs[0]); m->ip++; }
void machine_opcode_mov_r0_wr1(machine_t *m) { m->regs[0] = SFRD_U16(m->regs[1]); m->ip++; }
void machine_opcode_mov_r0_wr2(machine_t *m) { m->regs[0] = SFRD_U16(m->regs[2]); m->ip++; }
void machine_opcode_mov_r0_wr3(machine_t *m) { m->regs[0] = SFRD_U16(m->regs[3]); m->ip++; }
void machine_opcode_mov_r1_wr0(machine_t *m) { m->regs[1] = SFRD_U16(m->regs[0]); m->ip++; }
void machine_opcode_mov_r1_wr1(machine_t *m) { m->regs[1] = SFRD_U16(m->regs[1]); m->ip++; }
void machine_opcode_mov_r1_wr2(machine_t *m) { m->regs[1] = SFRD_U16(m->regs[2]); m->ip++; }
void machine_opcode_mov_r1_wr3(machine_t *m) { m->regs[1] = SFRD_U16(m->regs[3]); m->ip++; }
void machine_opcode_mov_r2_wr0(machine_t *m) { m->regs[2] = SFRD_U16(m->regs[0]); m->ip++; }
void machine_opcode_mov_r2_wr1(machine_t *m) { m->regs[2] = SFRD_U16(m->regs[1]); m->ip++; }
void machine_opcode_mov_r2_wr2(machine_t *m) { m->regs[2] = SFRD_U16(m->regs[2]); m->ip++; }
void machine_opcode_mov_r2_wr3(machine_t *m) { m->regs[2] = SFRD_U16(m->regs[3]); m->ip++; }
void machine_opcode_mov_r3_wr0(machine_t *m) { m->regs[3] = SFRD_U16(m->regs[0]); m->ip++; }
void machine_opcode_mov_r3_wr1(machine_t *m) { m->regs[3] = SFRD_U16(m->regs[1]); m->ip++; }
void machine_opcode_mov_r3_wr2(machine_t *m) { m->regs[3] = SFRD_U16(m->regs[2]); m->ip++; }
void machine_opcode_mov_r3_wr3(machine_t *m) { m->regs[3] = SFRD_U16(m->regs[3]); m->ip++; }

void machine_opcode_mov_br0_r0(machine_t *m) { if (m->regs[0] >= 0x1000) m->mem[m->regs[0]] = LOW(m->regs[0]); m->ip++; }
void machine_opcode_mov_br0_r1(machine_t *m) { if (m->regs[0] >= 0x1000) m->mem[m->regs[0]] = LOW(m->regs[1]); m->ip++; }
void machine_opcode_mov_br0_r2(machine_t *m) { if (m->regs[0] >= 0x1000) m->mem[m->regs[0]] = LOW(m->regs[2]); m->ip++; }
void machine_opcode_mov_br0_r3(machine_t *m) { if (m->regs[0] >= 0x1000) m->mem[m->regs[0]] = LOW(m->regs[3]); m->ip++; }
void machine_opcode_mov_br1_r0(machine_t *m) { if (m->regs[1] >= 0x1000) m->mem[m->regs[1]] = LOW(m->regs[0]); m->ip++; }
void machine_opcode_mov_br1_r1(machine_t *m) { if (m->regs[1] >= 0x1000) m->mem[m->regs[1]] = LOW(m->regs[1]); m->ip++; }
void machine_opcode_mov_br1_r2(machine_t *m) { if (m->regs[1] >= 0x1000) m->mem[m->regs[1]] = LOW(m->regs[2]); m->ip++; }
void machine_opcode_mov_br1_r3(machine_t *m) { if (m->regs[1] >= 0x1000) m->mem[m->regs[1]] = LOW(m->regs[3]); m->ip++; }
void machine_opcode_mov_br2_r0(machine_t *m) { if (m->regs[2] >= 0x1000) m->mem[m->regs[2]] = LOW(m->regs[0]); m->ip++; }
void machine_opcode_mov_br2_r1(machine_t *m) { if (m->regs[2] >= 0x1000) m->mem[m->regs[2]] = LOW(m->regs[1]); m->ip++; }
void machine_opcode_mov_br2_r2(machine_t *m) { if (m->regs[2] >= 0x1000) m->mem[m->regs[2]] = LOW(m->regs[2]); m->ip++; }
void machine_opcode_mov_br2_r3(machine_t *m) { if (m->regs[2] >= 0x1000) m->mem[m->regs[2]] = LOW(m->regs[3]); m->ip++; }
void machine_opcode_mov_br3_r0(machine_t *m) { if (m->regs[3] >= 0x1000) m->mem[m->regs[3]] = LOW(m->regs[0]); m->ip++; }
void machine_opcode_mov_br3_r1(machine_t *m) { if (m->regs[3] >= 0x1000) m->mem[m->regs[3]] = LOW(m->regs[1]); m->ip++; }
void machine_opcode_mov_br3_r2(machine_t *m) { if (m->regs[3] >= 0x1000) m->mem[m->regs[3]] = LOW(m->regs[2]); m->ip++; }
void machine_opcode_mov_br3_r3(machine_t *m) { if (m->regs[3] >= 0x1000) m->mem[m->regs[3]] = LOW(m->regs[3]); m->ip++; }

void machine_opcode_mov_wr0_r0(machine_t *m) {
    if (m->regs[0] < 0x1000 && m->ip>=0x1000) { m->ip++; return; }
    m->mem[m->regs[0]] = LOW(m->regs[0]);
    m->mem[m->regs[0]+1] = HIGH(m->regs[0]);
    m->ip++;
}
void machine_opcode_mov_wr0_r1(machine_t *m) {
    if (m->regs[0] < 0x1000 && m->ip>=0x1000) { m->ip++; return; }
    m->mem[m->regs[0]] = LOW(m->regs[1]);
    m->mem[m->regs[0]+1] = HIGH(m->regs[1]);
    m->ip++;
}
void machine_opcode_mov_wr0_r2(machine_t *m) {
    if (m->regs[0] < 0x1000 && m->ip>=0x1000) { m->ip++; return; }
    m->mem[m->regs[0]] = LOW(m->regs[2]);
    m->mem[m->regs[0]+1] = HIGH(m->regs[2]);
    m->ip++;
}
void machine_opcode_mov_wr0_r3(machine_t *m) {
    if (m->regs[0] < 0x1000 && m->ip>=0x1000) { m->ip++; return; }
    m->mem[m->regs[0]] = LOW(m->regs[3]);
    m->mem[m->regs[0]+1] = HIGH(m->regs[3]);
    m->ip++;
}

void machine_opcode_mov_wr1_r0(machine_t *m) {
    if (m->regs[1] < 0x1000 && m->ip>=0x1000) { m->ip++; return; }
    m->mem[m->regs[1]] = LOW(m->regs[0]);
    m->mem[m->regs[1]+1] = HIGH(m->regs[0]);
    m->ip++;
}
void machine_opcode_mov_wr1_r1(machine_t *m) {
    if (m->regs[1] < 0x1000 && m->ip>=0x1000) { m->ip++; return; }
    m->mem[m->regs[1]] = LOW(m->regs[1]);
    m->mem[m->regs[1]+1] = HIGH(m->regs[1]);
    m->ip++;
}
void machine_opcode_mov_wr1_r2(machine_t *m) {
    if (m->regs[1] < 0x1000 && m->ip>=0x1000) { m->ip++; return; }
    m->mem[m->regs[1]] = LOW(m->regs[2]);
    m->mem[m->regs[1]+1] = HIGH(m->regs[2]);
    m->ip++;
}
void machine_opcode_mov_wr1_r3(machine_t *m) {
    if (m->regs[1] < 0x1000 && m->ip>=0x1000) { m->ip++; return; }
    m->mem[m->regs[1]] = LOW(m->regs[3]);
    m->mem[m->regs[1]+1] = HIGH(m->regs[3]);
    m->ip++;
}

void machine_opcode_mov_wr2_r0(machine_t *m) {
    if (m->regs[2] < 0x1000 && m->ip>=0x1000) { m->ip++; return; }
    m->mem[m->regs[2]] = LOW(m->regs[0]);
    m->mem[m->regs[2]+1] = HIGH(m->regs[0]);
    m->ip++;
}
void machine_opcode_mov_wr2_r1(machine_t *m) {
    if (m->regs[2] < 0x1000 && m->ip>=0x1000) { m->ip++; return; }
    m->mem[m->regs[2]] = LOW(m->regs[1]);
    m->mem[m->regs[2]+1] = HIGH(m->regs[1]);
    m->ip++;
}
void machine_opcode_mov_wr2_r2(machine_t *m) {
    if (m->regs[2] < 0x1000 && m->ip>=0x1000) { m->ip++; return; }
    m->mem[m->regs[2]] = LOW(m->regs[2]);
    m->mem[m->regs[2]+1] = HIGH(m->regs[2]);
    m->ip++;
}
void machine_opcode_mov_wr2_r3(machine_t *m) {
    if (m->regs[2] < 0x1000 && m->ip>=0x1000) { m->ip++; return; }
    m->mem[m->regs[2]] = LOW(m->regs[3]);
    m->mem[m->regs[2]+1] = HIGH(m->regs[3]);
    m->ip++;
}

void machine_opcode_mov_wr3_r0(machine_t *m) {
    if (m->regs[3] < 0x1000 && m->ip>=0x1000) { m->ip++; return; }
    m->mem[m->regs[3]] = LOW(m->regs[0]);
    m->mem[m->regs[3]+1] = HIGH(m->regs[0]);
    m->ip++;
}
void machine_opcode_mov_wr3_r1(machine_t *m) {
    if (m->regs[3] < 0x1000 && m->ip>=0x1000) { m->ip++; return; }
    m->mem[m->regs[3]] = LOW(m->regs[1]);
    m->mem[m->regs[3]+1] = HIGH(m->regs[1]);
    m->ip++;
}
void machine_opcode_mov_wr3_r2(machine_t *m) {
    if (m->regs[3] < 0x1000 && m->ip>=0x1000) { m->ip++; return; }
    m->mem[m->regs[3]] = LOW(m->regs[2]);
    m->mem[m->regs[3]+1] = HIGH(m->regs[2]);
    m->ip++;
}
void machine_opcode_mov_wr3_r3(machine_t *m) {
    if (m->regs[3] < 0x1000 && m->ip>=0x1000) { m->ip++; return; }
    m->mem[m->regs[3]] = LOW(m->regs[3]);
    m->mem[m->regs[3]+1] = HIGH(m->regs[3]);
    m->ip++;
}

#define CTRL_EQU 1
#define CTRL_LT 2
#define CTRL_GT 4

void _op_test_common(machine_t *m, u16 x, u16 y) {
    m->control = 0;
    if (x == y) m->control |= CTRL_EQU;
    if (x < y) m->control |= CTRL_LT;
    if (x > y) m->control |= CTRL_GT;
}

void machine_opcode_test_r0_r0(machine_t *m) { _op_test_common(m, m->regs[0], m->regs[0]); m->ip++; }
void machine_opcode_test_r0_r1(machine_t *m) { _op_test_common(m, m->regs[0], m->regs[1]); m->ip++; }
void machine_opcode_test_r0_r2(machine_t *m) { _op_test_common(m, m->regs[0], m->regs[2]); m->ip++; }
void machine_opcode_test_r0_r3(machine_t *m) { _op_test_common(m, m->regs[0], m->regs[3]); m->ip++; }
void machine_opcode_test_r1_r0(machine_t *m) { _op_test_common(m, m->regs[1], m->regs[0]); m->ip++; }
void machine_opcode_test_r1_r1(machine_t *m) { _op_test_common(m, m->regs[1], m->regs[1]); m->ip++; }
void machine_opcode_test_r1_r2(machine_t *m) { _op_test_common(m, m->regs[1], m->regs[2]); m->ip++; }
void machine_opcode_test_r1_r3(machine_t *m) { _op_test_common(m, m->regs[1], m->regs[3]); m->ip++; }
void machine_opcode_test_r2_r0(machine_t *m) { _op_test_common(m, m->regs[2], m->regs[0]); m->ip++; }
void machine_opcode_test_r2_r1(machine_t *m) { _op_test_common(m, m->regs[2], m->regs[1]); m->ip++; }
void machine_opcode_test_r2_r2(machine_t *m) { _op_test_common(m, m->regs[2], m->regs[2]); m->ip++; }
void machine_opcode_test_r2_r3(machine_t *m) { _op_test_common(m, m->regs[2], m->regs[3]); m->ip++; }
void machine_opcode_test_r3_r0(machine_t *m) { _op_test_common(m, m->regs[3], m->regs[0]); m->ip++; }
void machine_opcode_test_r3_r1(machine_t *m) { _op_test_common(m, m->regs[3], m->regs[1]); m->ip++; }
void machine_opcode_test_r3_r2(machine_t *m) { _op_test_common(m, m->regs[3], m->regs[2]); m->ip++; }
void machine_opcode_test_r3_r3(machine_t *m) { _op_test_common(m, m->regs[3], m->regs[3]); m->ip++; }

void machine_opcode_push_sp(machine_t *machine) { 
    machine_t *m = machine;
    u16 cursp = machine->sp;
    if (machine->sp < 0x1000 && m->ip>=0x1000) { machine->ip++;return; }
    machine->sp--;
    machine->mem[machine->sp] = HIGH(cursp);
    machine->sp--;
    machine->mem[machine->sp] = LOW(cursp);
    machine->ip++;
}

void machine_opcode_push_ip(machine_t *machine) { 
    machine_t *m = machine;
    u16 cursp = machine->ip;
    if (machine->sp < 0x1000 && m->ip>=0x1000) { machine->ip++;return; }
    machine->sp--;
    machine->mem[machine->sp] = HIGH(cursp);
    machine->sp--;
    machine->mem[machine->sp] = LOW(cursp);
    machine->ip++;
}

void machine_opcode_push_r0(machine_t *machine) { 
    machine_t *m = machine;
    if (machine->sp < 0x1000 && m->ip>=0x1000) { machine->ip++;return; }
    machine->sp--;
    machine->mem[machine->sp] = HIGH(machine->regs[0]);
    machine->sp--;
    machine->mem[machine->sp] = LOW(machine->regs[0]);
    machine->ip++;
}
void machine_opcode_push_r1(machine_t *machine) { 
    machine_t *m = machine;
    if (machine->sp < 0x1000 && m->ip>=0x1000) { machine->ip++;return; }
    machine->sp--;
    machine->mem[machine->sp] = HIGH(machine->regs[1]);
    machine->sp--;
    machine->mem[machine->sp] = LOW(machine->regs[1]);
    machine->ip++;
}
void machine_opcode_push_r2(machine_t *machine) { 
    machine_t *m = machine;
    if (machine->sp < 0x1000 && m->ip>=0x1000) { machine->ip++;return; }
    machine->sp--;
    machine->mem[machine->sp] = HIGH(machine->regs[2]);
    machine->sp--;
    machine->mem[machine->sp] = LOW(machine->regs[2]);
    machine->ip++;
}
void machine_opcode_push_r3(machine_t *machine) { 
    machine_t *m = machine;
    if (machine->sp < 0x1000 && m->ip>=0x1000) { machine->ip++;return; }
    machine->sp--;
    machine->mem[machine->sp] = HIGH(machine->regs[3]);
    machine->sp--;
    machine->mem[machine->sp] = LOW(machine->regs[3]);
    machine->ip++;
}

void machine_opcode_pop_r0(machine_t *machine) { 
    machine_t *m = machine;
    if (machine->sp < 0x1000 && m->ip>=0x1000) { machine->ip++;return; }
    machine->regs[0] = READ_U16(machine->sp);
    machine->sp += 2;
    m->ip++;
}
void machine_opcode_pop_r1(machine_t *machine) { 
    machine_t *m = machine;
    if (machine->sp < 0x1000 && m->ip>=0x1000) { machine->ip++;return; }
    machine->regs[1] = READ_U16(machine->sp);
    machine->sp += 2;
    m->ip++;
}
void machine_opcode_pop_r2(machine_t *machine) { 
    machine_t *m = machine;
    if (machine->sp < 0x1000 && m->ip>=0x1000) { machine->ip++;return; }
    machine->regs[2] = READ_U16(machine->sp);
    machine->sp += 2;
    m->ip++;
}
void machine_opcode_pop_r3(machine_t *machine) { 
    machine_t *m = machine;
    if (machine->sp < 0x1000 && m->ip>=0x1000) { machine->ip++;return; }
    machine->regs[3] = READ_U16(machine->sp);
    machine->sp += 2;
    m->ip++;
}

void machine_opcode_jmp_imm(machine_t *m) {
    m->ip = READ_U16(m->ip+1);
}

void machine_opcode_call_imm(machine_t *m) {
    if (m->sp < 0x1000 && m->ip>=0x1000) { return; }
    m->sp--; m->mem[m->sp] = HIGH(m->ip+3);
    m->sp--; m->mem[m->sp] = LOW(m->ip+3);
    m->ip = READ_U16(m->ip+1);
}

void machine_opcode_jlt(machine_t *m) {
    if (m->control & CTRL_LT) return machine_opcode_jmp_imm(m);
    m->ip += 3;
}
void machine_opcode_jgt(machine_t *m) {
    if (m->control & CTRL_GT) return machine_opcode_jmp_imm(m);
    m->ip += 3;
}
void machine_opcode_jeq(machine_t *m) {
    if (m->control & CTRL_EQU) return machine_opcode_jmp_imm(m);
    m->ip += 3;
}
void machine_opcode_jne(machine_t *m) {
    if (!(m->control & CTRL_EQU)) return machine_opcode_jmp_imm(m);
    m->ip += 3;
}

void machine_opcode_clt(machine_t *m) {
    if (m->control & CTRL_LT) return machine_opcode_call_imm(m);
    m->ip += 3;
}
void machine_opcode_cgt(machine_t *m) {
    if (m->control & CTRL_GT) return machine_opcode_call_imm(m);
    m->ip += 3;
}
void machine_opcode_ceq(machine_t *m) {
    if (m->control & CTRL_EQU) return machine_opcode_call_imm(m);
    m->ip += 3;
}
void machine_opcode_cne(machine_t *m) {
    if (!(m->control & CTRL_EQU)) return machine_opcode_call_imm(m);
    m->ip += 3;
}

void machine_opcode_test_r0_imm(machine_t *m) { _op_test_common(m,m->regs[0],READ_U16(m->ip+1)); m->ip+=3; }
void machine_opcode_test_r1_imm(machine_t *m) { _op_test_common(m,m->regs[1],READ_U16(m->ip+1)); m->ip+=3; }
void machine_opcode_test_r2_imm(machine_t *m) { _op_test_common(m,m->regs[2],READ_U16(m->ip+1)); m->ip+=3; }
void machine_opcode_test_r3_imm(machine_t *m) { _op_test_common(m,m->regs[3],READ_U16(m->ip+1)); m->ip+=3; }

void machine_opcode_inc_r0(machine_t *m) { m->regs[0]++; m->ip++; }
void machine_opcode_inc_r1(machine_t *m) { m->regs[1]++; m->ip++; }
void machine_opcode_inc_r2(machine_t *m) { m->regs[2]++; m->ip++; }
void machine_opcode_inc_r3(machine_t *m) { m->regs[3]++; m->ip++; }

void machine_opcode_dec_r0(machine_t *m) { m->regs[0]--; m->ip++; }
void machine_opcode_dec_r1(machine_t *m) { m->regs[1]--; m->ip++; }
void machine_opcode_dec_r2(machine_t *m) { m->regs[2]--; m->ip++; }
void machine_opcode_dec_r3(machine_t *m) { m->regs[3]--; m->ip++; }

void machine_opcode_mov_r0_bimm(machine_t *m) {
    u16 addr = READ_U16(m->ip+1);
    m->regs[0] = SFRD_U8(addr);
    m->ip += 3;
}
void machine_opcode_mov_r1_bimm(machine_t *m) {
    u16 addr = READ_U16(m->ip+1);
    m->regs[1] = SFRD_U8(addr);
    m->ip += 3;
}
void machine_opcode_mov_r2_bimm(machine_t *m) {
    u16 addr = READ_U16(m->ip+1);
    m->regs[2] = SFRD_U8(addr);
    m->ip += 3;
}
void machine_opcode_mov_r3_bimm(machine_t *m) {
    u16 addr = READ_U16(m->ip+1);
    m->regs[3] = SFRD_U8(addr);
    m->ip += 3;
}

void machine_opcode_mov_r0_wimm(machine_t *m) {
    u16 addr = READ_U16(m->ip+1);
    m->regs[0] = SFRD_U16(addr);
    m->ip += 3;
}
void machine_opcode_mov_r1_wimm(machine_t *m) {
    u16 addr = READ_U16(m->ip+1);
    m->regs[1] = SFRD_U16(addr);
    m->ip += 3;
}
void machine_opcode_mov_r2_wimm(machine_t *m) {
    u16 addr = READ_U16(m->ip+1);
    m->regs[2] = SFRD_U16(addr);
    m->ip += 3;
}
void machine_opcode_mov_r3_wimm(machine_t *m) {
    u16 addr = READ_U16(m->ip+1);
    m->regs[3] = SFRD_U16(addr);
    m->ip += 3;
}

void machine_opcode_mov_bimm_r0(machine_t *m) {
    if (READ_U16(m->ip+1) < 0x1000 && m->ip>=0x1000) { m->ip += 3; return; }
    u8 v = LOW(m->regs[0]); m->mem[READ_U16(m->ip+1)] = v; m->ip += 3;
}
void machine_opcode_mov_bimm_r1(machine_t *m) {
    if (READ_U16(m->ip+1) < 0x1000 && m->ip>=0x1000) { m->ip += 3; return; }
    u8 v = LOW(m->regs[1]); m->mem[READ_U16(m->ip+1)] = v; m->ip += 3;
}
void machine_opcode_mov_bimm_r2(machine_t *m) {
    if (READ_U16(m->ip+1) < 0x1000 && m->ip>=0x1000) { m->ip += 3; return; }
    u8 v = LOW(m->regs[2]); m->mem[READ_U16(m->ip+1)] = v; m->ip += 3;
}
void machine_opcode_mov_bimm_r3(machine_t *m) {
    if (READ_U16(m->ip+1) < 0x1000 && m->ip>=0x1000) { m->ip += 3; return; }
    u8 v = LOW(m->regs[3]); m->mem[READ_U16(m->ip+1)] = v; m->ip += 3;
}

void machine_opcode_mov_wimm_r0(machine_t *m) {
    if (READ_U16(m->ip+1) < 0x1000 && m->ip>=0x1000) { m->ip += 3; return; }
    u16 v = m->regs[0]; u16 l = READ_U16(m->ip+1); if (l<0x1000){ m->ip += 3;return; }
    m->mem[l] = LOW(v); m->mem[l+1] = HIGH(v); m->ip += 3;
}
void machine_opcode_mov_wimm_r1(machine_t *m) {
    if (READ_U16(m->ip+1) < 0x1000 && m->ip>=0x1000) { m->ip += 3; return; }
    u16 v = m->regs[1]; u16 l = READ_U16(m->ip+1); if (l<0x1000){ m->ip += 3;return; }
    m->mem[l] = LOW(v); m->mem[l+1] = HIGH(v); m->ip += 3;
}
void machine_opcode_mov_wimm_r2(machine_t *m) {
    if (READ_U16(m->ip+1) < 0x1000 && m->ip>=0x1000) { m->ip += 3; return; }
    u16 v = m->regs[2]; u16 l = READ_U16(m->ip+1); if (l<0x1000){ m->ip += 3;return; }
    m->mem[l] = LOW(v); m->mem[l+1] = HIGH(v); m->ip += 3;
}
void machine_opcode_mov_wimm_r3(machine_t *m) {
    if (READ_U16(m->ip+1) < 0x1000 && m->ip>=0x1000) { m->ip += 3; return; }
    u16 v = m->regs[3]; u16 l = READ_U16(m->ip+1); if (l<0x1000){ m->ip += 3;return; }
    m->mem[l] = LOW(v); m->mem[l+1] = HIGH(v); m->ip += 3;
}

void machine_opcode_push_imm(machine_t *m) {
    u16 v = READ_U16(m->ip+1);
    m->sp--;
    m->mem[m->sp] = HIGH(v);
    m->sp--;
    m->mem[m->sp] = LOW(v);
}

void machine_opcode_jmp_r0(machine_t *m) { m->ip = m->regs[0]; }
void machine_opcode_jmp_r1(machine_t *m) { m->ip = m->regs[1]; }
void machine_opcode_jmp_r2(machine_t *m) { m->ip = m->regs[2]; }
void machine_opcode_jmp_r3(machine_t *m) { m->ip = m->regs[3]; }

void machine_opcode_call_r0(machine_t *m) { 
    if (m->sp < 0x1000 && m->ip>=0x1000) { return; }
    m->sp--; m->mem[m->sp] = HIGH(m->ip+1);
    m->sp--; m->mem[m->sp] = LOW(m->ip+1);
    m->ip = m->regs[0];
}
void machine_opcode_call_r1(machine_t *m) { 
    if (m->sp < 0x1000 && m->ip>=0x1000) { return; }
    m->sp--; m->mem[m->sp] = HIGH(m->ip+1);
    m->sp--; m->mem[m->sp] = LOW(m->ip+1);
    m->ip = m->regs[1];
}
void machine_opcode_call_r2(machine_t *m) { 
    if (m->sp < 0x1000 && m->ip>=0x1000) { return; }
    m->sp--; m->mem[m->sp] = HIGH(m->ip+1);
    m->sp--; m->mem[m->sp] = LOW(m->ip+1);
    m->ip = m->regs[2];
}
void machine_opcode_call_r3(machine_t *m) { 
    if (m->sp < 0x1000 && m->ip>=0x1000) { return; }
    m->sp--; m->mem[m->sp] = HIGH(m->ip+1);
    m->sp--; m->mem[m->sp] = LOW(m->ip+1);
    m->ip = m->regs[3];
}

void machine_opcode_and_r0_imm(machine_t *m) {
    m->regs[0] &= READ_U16(m->ip+1);
    m->ip += 3;
}
void machine_opcode_or_r0_imm(machine_t *m) {
    m->regs[0] |= READ_U16(m->ip+1);
    m->ip += 3;
}
void machine_opcode_xor_r0_imm(machine_t *m) {
    m->regs[0] ^= READ_U16(m->ip+1);
    m->ip += 3;
}

void machine_opcode_add_r0_imm(machine_t *m) {
    m->regs[0] += READ_U16(m->ip+1);
    m->ip += 3;
}
void machine_opcode_add_r1_imm(machine_t *m) {
    m->regs[1] += READ_U16(m->ip+1);
    m->ip += 3;
}
void machine_opcode_add_r2_imm(machine_t *m) {
    m->regs[2] += READ_U16(m->ip+1);
    m->ip += 3;
}
void machine_opcode_add_r3_imm(machine_t *m) {
    m->regs[3] += READ_U16(m->ip+1);
    m->ip += 3;
}

void machine_opcode_and_r0_r0(machine_t *m) { m->regs[0] &= m->regs[0]; m->ip++; }
void machine_opcode_and_r0_r1(machine_t *m) { m->regs[0] &= m->regs[1]; m->ip++; }
void machine_opcode_and_r0_r2(machine_t *m) { m->regs[0] &= m->regs[2]; m->ip++; }
void machine_opcode_and_r0_r3(machine_t *m) { m->regs[0] &= m->regs[3]; m->ip++; }

void machine_opcode_or_r0_r0(machine_t *m) { m->regs[0] |= m->regs[0]; m->ip++; }
void machine_opcode_or_r0_r1(machine_t *m) { m->regs[0] |= m->regs[1]; m->ip++; }
void machine_opcode_or_r0_r2(machine_t *m) { m->regs[0] |= m->regs[2]; m->ip++; }
void machine_opcode_or_r0_r3(machine_t *m) { m->regs[0] |= m->regs[3]; m->ip++; }

void machine_opcode_xor_r0_r0(machine_t *m) { m->regs[0] ^= m->regs[0]; m->ip++; }
void machine_opcode_xor_r0_r1(machine_t *m) { m->regs[0] ^= m->regs[1]; m->ip++; }
void machine_opcode_xor_r0_r2(machine_t *m) { m->regs[0] ^= m->regs[2]; m->ip++; }
void machine_opcode_xor_r0_r3(machine_t *m) { m->regs[0] ^= m->regs[3]; m->ip++; }

void machine_opcode_bswap_r0(machine_t *m) {
    u8 l = LOW(m->regs[0]);
    u8 h = HIGH(m->regs[0]);
    m->regs[0] = (l<<8) | h;
    m->ip++;
}
void machine_opcode_bswap_r1(machine_t *m) {
    u8 l = LOW(m->regs[0]);
    u8 h = HIGH(m->regs[0]);
    m->regs[0] = (l<<8) | h;
    m->ip++;
}
void machine_opcode_bswap_r2(machine_t *m) {
    u8 l = LOW(m->regs[0]);
    u8 h = HIGH(m->regs[0]);
    m->regs[0] = (l<<8) | h;
    m->ip++;
}
void machine_opcode_bswap_r3(machine_t *m) {
    u8 l = LOW(m->regs[0]);
    u8 h = HIGH(m->regs[0]);
    m->regs[0] = (l<<8) | h;
    m->ip++;
}

machine_opcode_t opcode_callbacks[] = {
    machine_opcode_brk,
    machine_opcode_mul_imm,
    machine_opcode_mul_r0_r1,
    machine_opcode_div_imm,
    machine_opcode_div_r0_r1,
    machine_opcode_ret,
    machine_opcode_sys,
    machine_opcode_mix,
    machine_opcode_nop,
    machine_opcode_mov_sp_imm,
    machine_opcode_mov_r0_sp,
    machine_opcode_mov_sp_r0,
    machine_opcode_unmix,
    machine_opcode_ill,
    machine_opcode_shl,
    machine_opcode_shr,
    machine_opcode_mov_r0_imm,
    machine_opcode_mov_r1_imm,
    machine_opcode_mov_r2_imm,
    machine_opcode_mov_r3_imm,

    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,

    machine_opcode_mov_r0_r0,
    machine_opcode_mov_r0_r1,
    machine_opcode_mov_r0_r2,
    machine_opcode_mov_r0_r3,
    machine_opcode_mov_r1_r0,
    machine_opcode_mov_r1_r1,
    machine_opcode_mov_r1_r2,
    machine_opcode_mov_r1_r3,
    machine_opcode_mov_r2_r0,
    machine_opcode_mov_r2_r1,
    machine_opcode_mov_r2_r2,
    machine_opcode_mov_r2_r3,
    machine_opcode_mov_r3_r0,
    machine_opcode_mov_r3_r1,
    machine_opcode_mov_r3_r2,
    machine_opcode_mov_r3_r3,

    machine_opcode_add_r0_r0,
    machine_opcode_add_r0_r1,
    machine_opcode_add_r0_r2,
    machine_opcode_add_r0_r3,
    machine_opcode_add_r1_r0,
    machine_opcode_add_r1_r1,
    machine_opcode_add_r1_r2,
    machine_opcode_add_r1_r3,
    machine_opcode_add_r2_r0,
    machine_opcode_add_r2_r1,
    machine_opcode_add_r2_r2,
    machine_opcode_add_r2_r3,
    machine_opcode_add_r3_r0,
    machine_opcode_add_r3_r1,
    machine_opcode_add_r3_r2,
    machine_opcode_add_r3_r3,
    
    machine_opcode_mov_r0_br0,
    machine_opcode_mov_r0_br1,
    machine_opcode_mov_r0_br2,
    machine_opcode_mov_r0_br3,
    machine_opcode_mov_r1_br0,
    machine_opcode_mov_r1_br1,
    machine_opcode_mov_r1_br2,
    machine_opcode_mov_r1_br3,
    machine_opcode_mov_r2_br0,
    machine_opcode_mov_r2_br1,
    machine_opcode_mov_r2_br2,
    machine_opcode_mov_r2_br3,
    machine_opcode_mov_r3_br0,
    machine_opcode_mov_r3_br1,
    machine_opcode_mov_r3_br2,
    machine_opcode_mov_r3_br3,

    machine_opcode_mov_r0_wr0,
    machine_opcode_mov_r0_wr1,
    machine_opcode_mov_r0_wr2,
    machine_opcode_mov_r0_wr3,
    machine_opcode_mov_r1_wr0,
    machine_opcode_mov_r1_wr1,
    machine_opcode_mov_r1_wr2,
    machine_opcode_mov_r1_wr3,
    machine_opcode_mov_r2_wr0,
    machine_opcode_mov_r2_wr1,
    machine_opcode_mov_r2_wr2,
    machine_opcode_mov_r2_wr3,
    machine_opcode_mov_r3_wr0,
    machine_opcode_mov_r3_wr1,
    machine_opcode_mov_r3_wr2,
    machine_opcode_mov_r3_wr3,

    machine_opcode_mov_br0_r0,
    machine_opcode_mov_br0_r1,
    machine_opcode_mov_br0_r2,
    machine_opcode_mov_br0_r3,
    machine_opcode_mov_br1_r0,
    machine_opcode_mov_br1_r1,
    machine_opcode_mov_br1_r2,
    machine_opcode_mov_br1_r3,
    machine_opcode_mov_br2_r0,
    machine_opcode_mov_br2_r1,
    machine_opcode_mov_br2_r2,
    machine_opcode_mov_br2_r3,
    machine_opcode_mov_br3_r0,
    machine_opcode_mov_br3_r1,
    machine_opcode_mov_br3_r2,
    machine_opcode_mov_br3_r3,
    
    machine_opcode_mov_wr0_r0,
    machine_opcode_mov_wr0_r1,
    machine_opcode_mov_wr0_r2,
    machine_opcode_mov_wr0_r3,
    machine_opcode_mov_wr1_r0,
    machine_opcode_mov_wr1_r1,
    machine_opcode_mov_wr1_r2,
    machine_opcode_mov_wr1_r3,
    machine_opcode_mov_wr2_r0,
    machine_opcode_mov_wr2_r1,
    machine_opcode_mov_wr2_r2,
    machine_opcode_mov_wr2_r3,
    machine_opcode_mov_wr3_r0,
    machine_opcode_mov_wr3_r1,
    machine_opcode_mov_wr3_r2,
    machine_opcode_mov_wr3_r3,

    machine_opcode_test_r0_r0,
    machine_opcode_test_r0_r1,
    machine_opcode_test_r0_r2,
    machine_opcode_test_r0_r3,
    machine_opcode_test_r1_r0,
    machine_opcode_test_r1_r1,
    machine_opcode_test_r1_r2,
    machine_opcode_test_r1_r3,
    machine_opcode_test_r2_r0,
    machine_opcode_test_r2_r1,
    machine_opcode_test_r2_r2,
    machine_opcode_test_r2_r3,
    machine_opcode_test_r3_r0,
    machine_opcode_test_r3_r1,
    machine_opcode_test_r3_r2,
    machine_opcode_test_r3_r3,
    
    machine_opcode_push_r0,
    machine_opcode_push_r1,
    machine_opcode_push_r2,
    machine_opcode_push_r3,
    machine_opcode_pop_r0,
    machine_opcode_pop_r1,
    machine_opcode_pop_r2,
    machine_opcode_pop_r3,
    
    machine_opcode_jmp_imm,
    machine_opcode_call_imm,
    machine_opcode_jlt,
    machine_opcode_jgt,
    machine_opcode_jeq,
    machine_opcode_jne,
    machine_opcode_clt,
    machine_opcode_cgt,
    machine_opcode_ceq,
    machine_opcode_cne,

    machine_opcode_test_r0_imm,
    machine_opcode_test_r1_imm,
    machine_opcode_test_r2_imm,
    machine_opcode_test_r3_imm,
    machine_opcode_push_sp,
    machine_opcode_push_ip,
    
    machine_opcode_inc_r0,
    machine_opcode_inc_r1,
    machine_opcode_inc_r2,
    machine_opcode_inc_r3,
    machine_opcode_dec_r0,
    machine_opcode_dec_r1,
    machine_opcode_dec_r2,
    machine_opcode_dec_r3,
    
    machine_opcode_mov_r0_bimm,
    machine_opcode_mov_r1_bimm,
    machine_opcode_mov_r2_bimm,
    machine_opcode_mov_r3_bimm,
    machine_opcode_mov_r0_wimm,
    machine_opcode_mov_r1_wimm,
    machine_opcode_mov_r2_wimm,
    machine_opcode_mov_r3_wimm,
    machine_opcode_mov_bimm_r0,
    machine_opcode_mov_bimm_r1,
    machine_opcode_mov_bimm_r2,
    machine_opcode_mov_bimm_r3,
    machine_opcode_mov_wimm_r0,
    machine_opcode_mov_wimm_r1,
    machine_opcode_mov_wimm_r2,
    machine_opcode_mov_wimm_r3,
    
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,

    machine_opcode_push_imm,
    machine_opcode_jmp_r0,
    machine_opcode_jmp_r1,
    machine_opcode_jmp_r2,
    machine_opcode_jmp_r3,
    machine_opcode_call_r0,
    machine_opcode_call_r1,
    machine_opcode_call_r2,
    machine_opcode_call_r3,

    machine_opcode_and_r0_imm,
    machine_opcode_or_r0_imm,
    machine_opcode_xor_r0_imm,
    
    machine_opcode_and_r0_r0,
    machine_opcode_and_r0_r1,
    machine_opcode_and_r0_r2,
    machine_opcode_and_r0_r3,
    machine_opcode_or_r0_r0,
    machine_opcode_or_r0_r1,
    machine_opcode_or_r0_r2,
    machine_opcode_or_r0_r3,
    machine_opcode_xor_r0_r0,
    machine_opcode_xor_r0_r1,
    machine_opcode_xor_r0_r2,
    machine_opcode_xor_r0_r3,
    
    machine_opcode_bswap_r0,
    machine_opcode_bswap_r1,
    machine_opcode_bswap_r2,
    machine_opcode_bswap_r3,
    
    machine_opcode_add_r0_imm,
    machine_opcode_add_r1_imm,
    machine_opcode_add_r2_imm,
    machine_opcode_add_r3_imm,

    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill
};

const char *opcode_names[] = {
    "opcode_brk",
    "opcode_mul_imm",
    "opcode_mul_r0_r1",
    "opcode_div_imm",
    "opcode_div_r0_r1",
    "opcode_ret",
    "opcode_sys",
    "opcode_mix",
    "opcode_nop",
    "opcode_mov_sp_imm",
    "opcode_mov_r0_sp",
    "opcode_mov_sp_r0",
    "opcode_unmix",
    "opcode_ill",
    "opcode_shl",
    "opcode_shr",
    "opcode_mov_r0_imm",
    "opcode_mov_r1_imm",
    "opcode_mov_r2_imm",
    "opcode_mov_r3_imm",

    "opcode_ill","opcode_ill","opcode_ill",
    "opcode_ill","opcode_ill","opcode_ill",
    "opcode_ill","opcode_ill","opcode_ill",
    "opcode_ill","opcode_ill","opcode_ill",

    "opcode_mov_r0_r0",
    "opcode_mov_r0_r1",
    "opcode_mov_r0_r2",
    "opcode_mov_r0_r3",
    "opcode_mov_r1_r0",
    "opcode_mov_r1_r1",
    "opcode_mov_r1_r2",
    "opcode_mov_r1_r3",
    "opcode_mov_r2_r0",
    "opcode_mov_r2_r1",
    "opcode_mov_r2_r2",
    "opcode_mov_r2_r3",
    "opcode_mov_r3_r0",
    "opcode_mov_r3_r1",
    "opcode_mov_r3_r2",
    "opcode_mov_r3_r3",

    "opcode_add_r0_r0",
    "opcode_add_r0_r1",
    "opcode_add_r0_r2",
    "opcode_add_r0_r3",
    "opcode_add_r1_r0",
    "opcode_add_r1_r1",
    "opcode_add_r1_r2",
    "opcode_add_r1_r3",
    "opcode_add_r2_r0",
    "opcode_add_r2_r1",
    "opcode_add_r2_r2",
    "opcode_add_r2_r3",
    "opcode_add_r3_r0",
    "opcode_add_r3_r1",
    "opcode_add_r3_r2",
    "opcode_add_r3_r3",
    
    "opcode_mov_r0_br0",
    "opcode_mov_r0_br1",
    "opcode_mov_r0_br2",
    "opcode_mov_r0_br3",
    "opcode_mov_r1_br0",
    "opcode_mov_r1_br1",
    "opcode_mov_r1_br2",
    "opcode_mov_r1_br3",
    "opcode_mov_r2_br0",
    "opcode_mov_r2_br1",
    "opcode_mov_r2_br2",
    "opcode_mov_r2_br3",
    "opcode_mov_r3_br0",
    "opcode_mov_r3_br1",
    "opcode_mov_r3_br2",
    "opcode_mov_r3_br3",

    "opcode_mov_r0_wr0",
    "opcode_mov_r0_wr1",
    "opcode_mov_r0_wr2",
    "opcode_mov_r0_wr3",
    "opcode_mov_r1_wr0",
    "opcode_mov_r1_wr1",
    "opcode_mov_r1_wr2",
    "opcode_mov_r1_wr3",
    "opcode_mov_r2_wr0",
    "opcode_mov_r2_wr1",
    "opcode_mov_r2_wr2",
    "opcode_mov_r2_wr3",
    "opcode_mov_r3_wr0",
    "opcode_mov_r3_wr1",
    "opcode_mov_r3_wr2",
    "opcode_mov_r3_wr3",

    "opcode_mov_br0_r0",
    "opcode_mov_br0_r1",
    "opcode_mov_br0_r2",
    "opcode_mov_br0_r3",
    "opcode_mov_br1_r0",
    "opcode_mov_br1_r1",
    "opcode_mov_br1_r2",
    "opcode_mov_br1_r3",
    "opcode_mov_br2_r0",
    "opcode_mov_br2_r1",
    "opcode_mov_br2_r2",
    "opcode_mov_br2_r3",
    "opcode_mov_br3_r0",
    "opcode_mov_br3_r1",
    "opcode_mov_br3_r2",
    "opcode_mov_br3_r3",
    
    "opcode_mov_wr0_r0",
    "opcode_mov_wr0_r1",
    "opcode_mov_wr0_r2",
    "opcode_mov_wr0_r3",
    "opcode_mov_wr1_r0",
    "opcode_mov_wr1_r1",
    "opcode_mov_wr1_r2",
    "opcode_mov_wr1_r3",
    "opcode_mov_wr2_r0",
    "opcode_mov_wr2_r1",
    "opcode_mov_wr2_r2",
    "opcode_mov_wr2_r3",
    "opcode_mov_wr3_r0",
    "opcode_mov_wr3_r1",
    "opcode_mov_wr3_r2",
    "opcode_mov_wr3_r3",

    "opcode_test_r0_r0",
    "opcode_test_r0_r1",
    "opcode_test_r0_r2",
    "opcode_test_r0_r3",
    "opcode_test_r1_r0",
    "opcode_test_r1_r1",
    "opcode_test_r1_r2",
    "opcode_test_r1_r3",
    "opcode_test_r2_r0",
    "opcode_test_r2_r1",
    "opcode_test_r2_r2",
    "opcode_test_r2_r3",
    "opcode_test_r3_r0",
    "opcode_test_r3_r1",
    "opcode_test_r3_r2",
    "opcode_test_r3_r3",
    
    "opcode_push_r0",
    "opcode_push_r1",
    "opcode_push_r2",
    "opcode_push_r3",
    "opcode_pop_r0",
    "opcode_pop_r1",
    "opcode_pop_r2",
    "opcode_pop_r3",
    
    "opcode_jmp_imm",
    "opcode_call_imm",
    "opcode_jlt",
    "opcode_jgt",
    "opcode_jeq",
    "opcode_jne",
    "opcode_clt",
    "opcode_cgt",
    "opcode_ceq",
    "opcode_cne",

    "opcode_test_r0_imm",
    "opcode_test_r1_imm",
    "opcode_test_r2_imm",
    "opcode_test_r3_imm",
    "opcode_push_sp",
    "opcode_ill",
    
    "opcode_inc_r0",
    "opcode_inc_r1",
    "opcode_inc_r2",
    "opcode_inc_r3",
    "opcode_dec_r0",
    "opcode_dec_r1",
    "opcode_dec_r2",
    "opcode_dec_r3",
    
    "opcode_mov_r0_bimm",
    "opcode_mov_r1_bimm",
    "opcode_mov_r2_bimm",
    "opcode_mov_r3_bimm",
    "opcode_mov_r0_wimm",
    "opcode_mov_r1_wimm",
    "opcode_mov_r2_wimm",
    "opcode_mov_r3_wimm",
    "opcode_mov_bimm_r0",
    "opcode_mov_bimm_r1",
    "opcode_mov_bimm_r2",
    "opcode_mov_bimm_r3",
    "opcode_mov_wimm_r0",
    "opcode_mov_wimm_r1",
    "opcode_mov_wimm_r2",
    "opcode_mov_wimm_r3",

    "(bepis)", "(bepis)", "(bepis)", "(bepis)",
    
    "opcode_push_imm",
    "opcode_jmp_r0",
    "opcode_jmp_r1",
    "opcode_jmp_r2",
    "opcode_jmp_r3",
    "opcode_call_r0",
    "opcode_call_r1",
    "opcode_call_r2",
    "opcode_call_r3",

    "opcode_and_r0_imm",
    "opcode_or_r0_imm",
    "opcode_xor_r0_imm",
    
    "opcode_and_r0_r0",
    "opcode_and_r0_r1",
    "opcode_and_r0_r2",
    "opcode_and_r0_r3",
    "opcode_or_r0_r0",
    "opcode_or_r0_r1",
    "opcode_or_r0_r2",
    "opcode_or_r0_r3",
    "opcode_xor_r0_r0",
    "opcode_xor_r0_r1",
    "opcode_xor_r0_r2",
    "opcode_xor_r0_r3",
    
    "opcode_bswap_r0",
    "opcode_bswap_r1",
    "opcode_bswap_r2",
    "opcode_bswap_r3",
    
    "opcode_add_r0_imm",
    "opcode_add_r1_imm",
    "opcode_add_r2_imm",
    "opcode_add_r3_imm",
	
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)"
};

machine_opcode_t opcode_callbacks2[] = {
    machine_opcode_brk,
    machine_opcode_mul_imm,
    machine_opcode_mul_r0_r1,
    machine_opcode_div_imm,
    machine_opcode_div_r0_r1,
    machine_opcode_ret,
    machine_opcode_sys,
    machine_opcode_mix,
    machine_opcode_nop,
    machine_opcode_mov_sp_imm,
    machine_opcode_mov_r0_sp,
    machine_opcode_mov_sp_r0,
    machine_opcode_unmix,
    machine_opcode_ill,
    machine_opcode_shl,
    machine_opcode_shr,
    machine_opcode_mov_r0_imm,
    machine_opcode_mov_r1_imm,
    machine_opcode_mov_r2_imm,
    machine_opcode_mov_r3_imm,

    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,

    machine_opcode_mov_r0_r0,
    machine_opcode_mov_r0_r1,
    machine_opcode_mov_r0_r2,
    machine_opcode_mov_r0_r3,
    machine_opcode_mov_r1_r0,
    machine_opcode_mov_r1_r1,
    machine_opcode_mov_r1_r2,
    machine_opcode_mov_r1_r3,
    machine_opcode_mov_r2_r0,
    machine_opcode_mov_r2_r1,
    machine_opcode_mov_r2_r2,
    machine_opcode_mov_r2_r3,
    machine_opcode_mov_r3_r0,
    machine_opcode_mov_r3_r1,
    machine_opcode_mov_r3_r2,
    machine_opcode_mov_r3_r3,

    machine_opcode_add_r0_r0,
    machine_opcode_add_r0_r1,
    machine_opcode_add_r0_r2,
    machine_opcode_add_r0_r3,
    machine_opcode_add_r1_r0,
    machine_opcode_add_r1_r1,
    machine_opcode_add_r1_r2,
    machine_opcode_add_r1_r3,
    machine_opcode_add_r2_r0,
    machine_opcode_add_r2_r1,
    machine_opcode_add_r2_r2,
    machine_opcode_add_r2_r3,
    machine_opcode_add_r3_r0,
    machine_opcode_add_r3_r1,
    machine_opcode_add_r3_r2,
    machine_opcode_add_r3_r3,
    
    machine_opcode_mov_r0_br0,
    machine_opcode_mov_r0_br1,
    machine_opcode_mov_r0_br2,
    machine_opcode_mov_r0_br3,
    machine_opcode_mov_r1_br0,
    machine_opcode_mov_r1_br1,
    machine_opcode_mov_r1_br2,
    machine_opcode_mov_r1_br3,
    machine_opcode_mov_r2_br0,
    machine_opcode_mov_r2_br1,
    machine_opcode_mov_r2_br2,
    machine_opcode_mov_r2_br3,
    machine_opcode_mov_r3_br0,
    machine_opcode_mov_r3_br1,
    machine_opcode_mov_r3_br2,
    machine_opcode_mov_r3_br3,

    machine_opcode_mov_r0_wr0,
    machine_opcode_mov_r0_wr1,
    machine_opcode_mov_r0_wr2,
    machine_opcode_mov_r0_wr3,
    machine_opcode_mov_r1_wr0,
    machine_opcode_mov_r1_wr1,
    machine_opcode_mov_r1_wr2,
    machine_opcode_mov_r1_wr3,
    machine_opcode_mov_r2_wr0,
    machine_opcode_mov_r2_wr1,
    machine_opcode_mov_r2_wr2,
    machine_opcode_mov_r2_wr3,
    machine_opcode_mov_r3_wr0,
    machine_opcode_mov_r3_wr1,
    machine_opcode_mov_r3_wr2,
    machine_opcode_mov_r3_wr3,

    machine_opcode_mov_br0_r0,
    machine_opcode_mov_br0_r1,
    machine_opcode_mov_br0_r2,
    machine_opcode_mov_br0_r3,
    machine_opcode_mov_br1_r0,
    machine_opcode_mov_br1_r1,
    machine_opcode_mov_br1_r2,
    machine_opcode_mov_br1_r3,
    machine_opcode_mov_br2_r0,
    machine_opcode_mov_br2_r1,
    machine_opcode_mov_br2_r2,
    machine_opcode_mov_br2_r3,
    machine_opcode_mov_br3_r0,
    machine_opcode_mov_br3_r1,
    machine_opcode_mov_br3_r2,
    machine_opcode_mov_br3_r3,
    
    machine_opcode_mov_wr0_r0,
    machine_opcode_mov_wr0_r1,
    machine_opcode_mov_wr0_r2,
    machine_opcode_mov_wr0_r3,
    machine_opcode_mov_wr1_r0,
    machine_opcode_mov_wr1_r1,
    machine_opcode_mov_wr1_r2,
    machine_opcode_mov_wr1_r3,
    machine_opcode_mov_wr2_r0,
    machine_opcode_mov_wr2_r1,
    machine_opcode_mov_wr2_r2,
    machine_opcode_mov_wr2_r3,
    machine_opcode_mov_wr3_r0,
    machine_opcode_mov_wr3_r1,
    machine_opcode_mov_wr3_r2,
    machine_opcode_mov_wr3_r3,

    machine_opcode_test_r0_r0,
    machine_opcode_test_r0_r1,
    machine_opcode_test_r0_r2,
    machine_opcode_test_r0_r3,
    machine_opcode_test_r1_r0,
    machine_opcode_test_r1_r1,
    machine_opcode_test_r1_r2,
    machine_opcode_test_r1_r3,
    machine_opcode_test_r2_r0,
    machine_opcode_test_r2_r1,
    machine_opcode_test_r2_r2,
    machine_opcode_test_r2_r3,
    machine_opcode_test_r3_r0,
    machine_opcode_test_r3_r1,
    machine_opcode_test_r3_r2,
    machine_opcode_test_r3_r3,
    
    machine_opcode_push_r0,
    machine_opcode_push_r1,
    machine_opcode_push_r2,
    machine_opcode_push_r3,
    machine_opcode_pop_r0,
    machine_opcode_pop_r1,
    machine_opcode_pop_r2,
    machine_opcode_pop_r3,
    
    machine_opcode_jmp_imm,
    machine_opcode_call_imm,
    machine_opcode_jlt,
    machine_opcode_jgt,
    machine_opcode_jeq,
    machine_opcode_jne,
    machine_opcode_clt,
    machine_opcode_cgt,
    machine_opcode_ceq,
    machine_opcode_cne,

    machine_opcode_test_r0_imm,
    machine_opcode_test_r1_imm,
    machine_opcode_test_r2_imm,
    machine_opcode_test_r3_imm,
    machine_opcode_push_sp,
    machine_opcode_push_ip,
    
    machine_opcode_inc_r0,
    machine_opcode_inc_r1,
    machine_opcode_inc_r2,
    machine_opcode_inc_r3,
    machine_opcode_dec_r0,
    machine_opcode_dec_r1,
    machine_opcode_dec_r2,
    machine_opcode_dec_r3,
    
    machine_opcode_mov_r0_bimm,
    machine_opcode_mov_r1_bimm,
    machine_opcode_mov_r2_bimm,
    machine_opcode_mov_r3_bimm,
    machine_opcode_mov_r0_wimm,
    machine_opcode_mov_r1_wimm,
    machine_opcode_mov_r2_wimm,
    machine_opcode_mov_r3_wimm,
    machine_opcode_mov_bimm_r0,
    machine_opcode_mov_bimm_r1,
    machine_opcode_mov_bimm_r2,
    machine_opcode_mov_bimm_r3,
    machine_opcode_mov_wimm_r0,
    machine_opcode_mov_wimm_r1,
    machine_opcode_mov_wimm_r2,
    machine_opcode_mov_wimm_r3,
    
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,

    machine_opcode_push_imm,
    machine_opcode_jmp_r0,
    machine_opcode_jmp_r1,
    machine_opcode_jmp_r2,
    machine_opcode_jmp_r3,
    machine_opcode_call_r0,
    machine_opcode_call_r1,
    machine_opcode_call_r2,
    machine_opcode_call_r3,

    machine_opcode_and_r0_imm,
    machine_opcode_or_r0_imm,
    machine_opcode_xor_r0_imm,
    
    machine_opcode_and_r0_r0,
    machine_opcode_and_r0_r1,
    machine_opcode_and_r0_r2,
    machine_opcode_and_r0_r3,
    machine_opcode_or_r0_r0,
    machine_opcode_or_r0_r1,
    machine_opcode_or_r0_r2,
    machine_opcode_or_r0_r3,
    machine_opcode_xor_r0_r0,
    machine_opcode_xor_r0_r1,
    machine_opcode_xor_r0_r2,
    machine_opcode_xor_r0_r3,
    
    machine_opcode_bswap_r0,
    machine_opcode_bswap_r1,
    machine_opcode_bswap_r2,
    machine_opcode_bswap_r3,
    
    machine_opcode_add_r0_imm,
    machine_opcode_add_r1_imm,
    machine_opcode_add_r2_imm,
    machine_opcode_add_r3_imm,

    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill,
    machine_opcode_ill, machine_opcode_ill, machine_opcode_ill, machine_opcode_ill
};

const char *opcode_names2[] = {
    "opcode_brk",
    "opcode_mul_imm",
    "opcode_mul_r0_r1",
    "opcode_div_imm",
    "opcode_div_r0_r1",
    "opcode_ret",
    "opcode_sys",
    "opcode_mix",
    "opcode_nop",
    "opcode_mov_sp_imm",
    "opcode_mov_r0_sp",
    "opcode_mov_sp_r0",
    "opcode_unmix",
    "opcode_ill",
    "opcode_shl",
    "opcode_shr",
    "opcode_mov_r0_imm",
    "opcode_mov_r1_imm",
    "opcode_mov_r2_imm",
    "opcode_mov_r3_imm",

    "opcode_ill","opcode_ill","opcode_ill",
    "opcode_ill","opcode_ill","opcode_ill",
    "opcode_ill","opcode_ill","opcode_ill",
    "opcode_ill","opcode_ill","opcode_ill",

    "opcode_mov_r0_r0",
    "opcode_mov_r0_r1",
    "opcode_mov_r0_r2",
    "opcode_mov_r0_r3",
    "opcode_mov_r1_r0",
    "opcode_mov_r1_r1",
    "opcode_mov_r1_r2",
    "opcode_mov_r1_r3",
    "opcode_mov_r2_r0",
    "opcode_mov_r2_r1",
    "opcode_mov_r2_r2",
    "opcode_mov_r2_r3",
    "opcode_mov_r3_r0",
    "opcode_mov_r3_r1",
    "opcode_mov_r3_r2",
    "opcode_mov_r3_r3",

    "opcode_add_r0_r0",
    "opcode_add_r0_r1",
    "opcode_add_r0_r2",
    "opcode_add_r0_r3",
    "opcode_add_r1_r0",
    "opcode_add_r1_r1",
    "opcode_add_r1_r2",
    "opcode_add_r1_r3",
    "opcode_add_r2_r0",
    "opcode_add_r2_r1",
    "opcode_add_r2_r2",
    "opcode_add_r2_r3",
    "opcode_add_r3_r0",
    "opcode_add_r3_r1",
    "opcode_add_r3_r2",
    "opcode_add_r3_r3",
    
    "opcode_mov_r0_br0",
    "opcode_mov_r0_br1",
    "opcode_mov_r0_br2",
    "opcode_mov_r0_br3",
    "opcode_mov_r1_br0",
    "opcode_mov_r1_br1",
    "opcode_mov_r1_br2",
    "opcode_mov_r1_br3",
    "opcode_mov_r2_br0",
    "opcode_mov_r2_br1",
    "opcode_mov_r2_br2",
    "opcode_mov_r2_br3",
    "opcode_mov_r3_br0",
    "opcode_mov_r3_br1",
    "opcode_mov_r3_br2",
    "opcode_mov_r3_br3",

    "opcode_mov_r0_wr0",
    "opcode_mov_r0_wr1",
    "opcode_mov_r0_wr2",
    "opcode_mov_r0_wr3",
    "opcode_mov_r1_wr0",
    "opcode_mov_r1_wr1",
    "opcode_mov_r1_wr2",
    "opcode_mov_r1_wr3",
    "opcode_mov_r2_wr0",
    "opcode_mov_r2_wr1",
    "opcode_mov_r2_wr2",
    "opcode_mov_r2_wr3",
    "opcode_mov_r3_wr0",
    "opcode_mov_r3_wr1",
    "opcode_mov_r3_wr2",
    "opcode_mov_r3_wr3",

    "opcode_mov_br0_r0",
    "opcode_mov_br0_r1",
    "opcode_mov_br0_r2",
    "opcode_mov_br0_r3",
    "opcode_mov_br1_r0",
    "opcode_mov_br1_r1",
    "opcode_mov_br1_r2",
    "opcode_mov_br1_r3",
    "opcode_mov_br2_r0",
    "opcode_mov_br2_r1",
    "opcode_mov_br2_r2",
    "opcode_mov_br2_r3",
    "opcode_mov_br3_r0",
    "opcode_mov_br3_r1",
    "opcode_mov_br3_r2",
    "opcode_mov_br3_r3",
    
    "opcode_mov_wr0_r0",
    "opcode_mov_wr0_r1",
    "opcode_mov_wr0_r2",
    "opcode_mov_wr0_r3",
    "opcode_mov_wr1_r0",
    "opcode_mov_wr1_r1",
    "opcode_mov_wr1_r2",
    "opcode_mov_wr1_r3",
    "opcode_mov_wr2_r0",
    "opcode_mov_wr2_r1",
    "opcode_mov_wr2_r2",
    "opcode_mov_wr2_r3",
    "opcode_mov_wr3_r0",
    "opcode_mov_wr3_r1",
    "opcode_mov_wr3_r2",
    "opcode_mov_wr3_r3",

    "opcode_test_r0_r0",
    "opcode_test_r0_r1",
    "opcode_test_r0_r2",
    "opcode_test_r0_r3",
    "opcode_test_r1_r0",
    "opcode_test_r1_r1",
    "opcode_test_r1_r2",
    "opcode_test_r1_r3",
    "opcode_test_r2_r0",
    "opcode_test_r2_r1",
    "opcode_test_r2_r2",
    "opcode_test_r2_r3",
    "opcode_test_r3_r0",
    "opcode_test_r3_r1",
    "opcode_test_r3_r2",
    "opcode_test_r3_r3",
    
    "opcode_push_r0",
    "opcode_push_r1",
    "opcode_push_r2",
    "opcode_push_r3",
    "opcode_pop_r0",
    "opcode_pop_r1",
    "opcode_pop_r2",
    "opcode_pop_r3",
    
    "opcode_jmp_imm",
    "opcode_call_imm",
    "opcode_jlt",
    "opcode_jgt",
    "opcode_jeq",
    "opcode_jne",
    "opcode_clt",
    "opcode_cgt",
    "opcode_ceq",
    "opcode_cne",

    "opcode_test_r0_imm",
    "opcode_test_r1_imm",
    "opcode_test_r2_imm",
    "opcode_test_r3_imm",
    "opcode_push_sp",
    "opcode_ill",
    
    "opcode_inc_r0",
    "opcode_inc_r1",
    "opcode_inc_r2",
    "opcode_inc_r3",
    "opcode_dec_r0",
    "opcode_dec_r1",
    "opcode_dec_r2",
    "opcode_dec_r3",
    
    "opcode_mov_r0_bimm",
    "opcode_mov_r1_bimm",
    "opcode_mov_r2_bimm",
    "opcode_mov_r3_bimm",
    "opcode_mov_r0_wimm",
    "opcode_mov_r1_wimm",
    "opcode_mov_r2_wimm",
    "opcode_mov_r3_wimm",
    "opcode_mov_bimm_r0",
    "opcode_mov_bimm_r1",
    "opcode_mov_bimm_r2",
    "opcode_mov_bimm_r3",
    "opcode_mov_wimm_r0",
    "opcode_mov_wimm_r1",
    "opcode_mov_wimm_r2",
    "opcode_mov_wimm_r3",

    "(bepis)", "(bepis)", "(bepis)", "(bepis)",
    
    "opcode_push_imm",
    "opcode_jmp_r0",
    "opcode_jmp_r1",
    "opcode_jmp_r2",
    "opcode_jmp_r3",
    "opcode_call_r0",
    "opcode_call_r1",
    "opcode_call_r2",
    "opcode_call_r3",

    "opcode_and_r0_imm",
    "opcode_or_r0_imm",
    "opcode_xor_r0_imm",
    
    "opcode_and_r0_r0",
    "opcode_and_r0_r1",
    "opcode_and_r0_r2",
    "opcode_and_r0_r3",
    "opcode_or_r0_r0",
    "opcode_or_r0_r1",
    "opcode_or_r0_r2",
    "opcode_or_r0_r3",
    "opcode_xor_r0_r0",
    "opcode_xor_r0_r1",
    "opcode_xor_r0_r2",
    "opcode_xor_r0_r3",
    
    "opcode_bswap_r0",
    "opcode_bswap_r1",
    "opcode_bswap_r2",
    "opcode_bswap_r3",
    
    "opcode_add_r0_imm",
    "opcode_add_r1_imm",
    "opcode_add_r2_imm",
    "opcode_add_r3_imm",
	
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", 
	"(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)", "(bepis)"
};