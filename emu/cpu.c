#include "cpu.h"
#include <unistd.h>
#include <stdint.h>

uint16_t pc, sp;
extern unsigned char mem[0x10000], reg[3];
unsigned char *pca = (unsigned char *)&pc;

void PushStack(uint8_t *byte) {
    mem[sp] = *byte;
    sp++;
}

void PullStack(uint8_t *byte) {
    sp--;
    *byte = mem[sp];
}

void NMI() {
    // Dereference the little-endian pointer at 0xFFFE-0xFFFF and set PC
    pc = mem[0xFFFE] | (mem[0xFFFF] << 8); // Load PC from NMI vector
}

int RunCPU() {
    switch (mem[pc]) {
        case 0x0: // NOP
            break;
        case 0x1: // ADD
            reg[2] = reg[0] + mem[pc + 1];
            pc++;
            break;
        case 0x2: // OUTA
            write(1, &reg[0], 1); // Output to stdout
            break;
        case 0x3: // JMP
            pc = (mem[pc + 2] << 8) | mem[pc + 1] - 1; // Jump using little-endian
            break;
        case 0x4: // JZ
            if (reg[2] == 0) {
                pc = (mem[pc + 2] << 8) | mem[pc + 1] - 1; // Jump if zero, little-endian
            }
            break;
        case 0x5: // SUB
            reg[2] = reg[0] - mem[pc + 1];
            pc++;
            break;
        case 0x6: // JR
            pc += (int16_t)((mem[pc + 2] << 8) | mem[pc + 1]) - 1; // Relative jump
            break;
        case 0x7: // CALL
            PushStack(&pca[1]); // Push return address
            PushStack(&pca[0]);
            pc = (mem[pc + 2] << 8) | mem[pc + 1] - 1; // Jump to subroutine
            break;
        case 0x8: // SETSP
            sp = (mem[pc + 2] << 8) | mem[pc + 1]; // Set Stack Pointer
            pc += 2;
            break;
        case 0x9: // TBA
            reg[0] = reg[1]; // Copy B to A
            break;
        case 0xA: // TAB
            reg[1] = reg[0]; // Copy A to B
            break;
        case 0xB: // LDA immediate
            reg[0] = mem[pc + 1]; // Load A with immediate value
            pc++;
            break;
        case 0xC: // LDB immediate
            reg[1] = mem[pc + 1]; // Load B with immediate value
            pc++;
            break;
        case 0xD: // LDA (mem)
            {
                uint16_t addr = (mem[pc + 2] << 8) | mem[pc + 1]; // Little-endian address
                reg[0] = mem[addr]; // Load A with value from memory
                pc += 2;
            }
            break;
        case 0xE: // LDB (mem)
            {
                uint16_t addr = (mem[pc + 2] << 8) | mem[pc + 1]; // Little-endian address
                reg[1] = mem[addr]; // Load B with value from memory
                pc += 2;
            }
            break;
        case 0xF: // HLT
            _exit(reg[0]); // Halt the clock
            break;
        case 0x10: // STA
            {
                uint16_t addr = (mem[pc + 2] << 8) | mem[pc + 1]; // Little-endian address
                mem[addr] = reg[0]; // Store A in memory
                pc += 2;
            }
            break;
        case 0x11: // STB
            {
                uint16_t addr = (mem[pc + 2] << 8) | mem[pc + 1]; // Little-endian address
                mem[addr] = reg[1]; // Store B in memory
                pc += 2;
            }
            break;
        case 0x12: // PHA
            PushStack(&reg[0]); // Push A to stack
            break;
        case 0x13: // PLA
            PullStack(&reg[0]); // Pull A from stack
            break;
        case 0x14: // RTS
            PullStack(&pca[0]); // Pull return address from stack
            PullStack(&pca[1]);
            break;
        // Handle unknown opcodes with NMI
        default:
            pc = (mem[0xFFFE] | (mem[0xFFFF] << 8)); // NMI vector
            break;
    }
    return 0;
}

