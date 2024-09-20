#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define START_ADDRESS 0x8000
#define END_ADDRESS 0xFFFF
#define MAX_LINE_LENGTH 128

// Function to disassemble the binary
void disassemble(const char* input_file) {
    unsigned char memory[END_ADDRESS - START_ADDRESS + 1];
    int bin_fd = open(input_file, O_RDONLY);
    if (bin_fd < 0) {
        perror("Error opening binary file");
        exit(1);
    }

    // Read the binary file into memory
    ssize_t bytes_read = read(bin_fd, memory, sizeof(memory));
    close(bin_fd);

    // Disassemble the binary
    int pc = START_ADDRESS;
    ssize_t index = 0;

    while (index < bytes_read) {
        unsigned char opcode = memory[index];
        printf("%04X: ", pc);

        switch (opcode) {
            case 0x00: // NOP
                printf("NOP\n");
                pc++;
                index++;
                break;
            case 0x01: // ADD
                printf("ADD #$%02X\n", memory[index + 1]);
                pc += 2;
                index += 2;
                break;
            case 0x02: // OUTA
                printf("OUTA\n");
                pc++;
                index++;
                break;
            case 0x03: // JMP
                printf("JMP $%04X\n", (memory[index + 2] << 8) | memory[index + 1]);
                pc += 3;
                index += 3;
                break;
            case 0x04: // JZ
                printf("JZ $%04X\n", (memory[index + 2] << 8) | memory[index + 1]);
                pc += 3;
                index += 3;
                break;
            case 0x05: // SUB
                printf("SUB #$%02X\n", memory[index + 1]);
                pc += 2;
                index += 2;
                break;
            case 0x06: // JR
                printf("JR $%04X\n", (memory[index + 2] << 8) | memory[index + 1]);
                pc += 3;
                index += 3;
                break;
            case 0x07: // CALL
                printf("CALL $%04X\n", (memory[index + 2] << 8) | memory[index + 1]);
                pc += 3;
                index += 3;
                break;
            case 0x08: // SETSP
                printf("SETSP $%02X\n", memory[index + 1]);
                pc += 2;
                index += 2;
                break;
            case 0x09: // TBA
                printf("TBA\n");
                pc++;
                index++;
                break;
            case 0x0A: // TAB
                printf("TAB\n");
                pc++;
                index++;
                break;
            case 0x0B: // LDA
                printf("LDA #$%02X\n", memory[index + 1]);
                pc += 2;
                index += 2;
                break;
            case 0x0C: // LDB
                printf("LDB #$%02X\n", memory[index + 1]);
                pc += 2;
                index += 2;
                break;
            case 0x0D: // LDA (mem)
                printf("LDA $%04X\n", (memory[index + 2] << 8) | memory[index + 1]);
                pc += 3;
                index += 3;
                break;
            case 0x0E: // LDB (mem)
                printf("LDB $%04X\n", (memory[index + 2] << 8) | memory[index + 1]);
                pc += 3;
                index += 3;
                break;
            case 0x0F: // HLT
                printf("HLT\n");
                pc++;
                index++;
                break;
            case 0x10: // STA
                printf("STA $%04X\n", (memory[index + 2] << 8) | memory[index + 1]);
                pc += 3;
                index += 3;
                break;
            case 0x11: // STB
                printf("STB $%04X\n", (memory[index + 2] << 8) | memory[index + 1]);
                pc += 3;
                index += 3;
                break;
            case 0x12: // PHA
                printf("PHA\n");
                pc++;
                index++;
                break;
            case 0x13: // PLA
                printf("PLA\n");
                pc++;
                index++;
                break;
            case 0x14: // RTS
                printf("RTS\n");
                pc++;
                index++;
                break;
            case 0x16: // CMP
                printf("CMP #$%02X\n", memory[index + 1]);
                pc += 2;
                index += 2;
                break;
            default:
                printf("UNKNOWN OPCODE: 0x%02X\n", opcode);
                pc++;
                index++;
                break;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Error: No input file specified.\n");
        return 1;
    }

    disassemble(argv[1]);
    return 0;
}

