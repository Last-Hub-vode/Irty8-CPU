#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include "cpu.h"

unsigned char mem[0x10000], reg[3];
extern uint16_t pc, sp;

int main(int argc, char **argv) {
    if (argc < 2) {
        dprintf(2, "No file specified.");
        return -1;
    }
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    read(fd, &mem[0x8000], 0x8000);
    pc = (mem[0xFFFD] << 8) | mem[0xFFFC]; // Initialize PC from reset vector

    for (;;) {
        RunCPU(); // Execute the current instruction
        pc++; // Increment PC after executing an instruction
    }

    close(fd);
}

