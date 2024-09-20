#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define MAX_LINE_LENGTH 128
#define MAX_LABEL_LENGTH 32
#define MAX_OUTPUT_SIZE (0xFFFF - 0x8000 + 1)

// Address range for the emulator
#define START_ADDRESS 0x8000

typedef struct {
    char label[MAX_LABEL_LENGTH];
    int address;
} Label;

Label labels[256]; // Support up to 256 labels
int label_count = 0;

// Function to add labels
void add_label(const char* label, int address) {
    strncpy(labels[label_count].label, label, MAX_LABEL_LENGTH);
    labels[label_count].address = address;
    label_count++;
}

// Function to resolve labels
int resolve_label(const char* label) {
    for (int i = 0; i < label_count; i++) {
        if (strcmp(labels[i].label, label) == 0) {
            return labels[i].address;
        }
    }
    return -1; // Label not found
}

// Function to handle the assembly process
void assemble(const char* input_file, const char* output_file) {
    char line[MAX_LINE_LENGTH];
    unsigned char output[MAX_OUTPUT_SIZE] = {0}; // Output buffer

    int asm_fd = open(input_file, O_RDONLY);
    if (asm_fd < 0) {
        perror("Error opening assembly file");
        exit(1);
    }

    // Read the file line by line
    int current_address = START_ADDRESS;
    while (read(asm_fd, line, sizeof(line) - 1) > 0) {
        line[strcspn(line, "\n")] = 0; // Strip newline

        // Remove comments
        char* comment_ptr = strchr(line, ';');
        if (comment_ptr) {
            *comment_ptr = '\0'; // Null-terminate to ignore the comment
        }

        // Check for labels
        char* label_ptr = strchr(line, ':');
        if (label_ptr) {
            *label_ptr = '\0'; // Null-terminate the label
            add_label(line, current_address); // Add label with current address
            continue; // Skip to the next line
        }

        // Handle .org directive
        if (strstr(line, ".org") == line) {
            sscanf(line + 5, "%x", &current_address);
            continue;
        }

        // Process instruction
        char instruction[16], operand[16];
        if (sscanf(line, "%s %s", instruction, operand) >= 1) {
            // Handle instructions
	if (strcmp(instruction, "NOP") == 0) {
                output[current_address - START_ADDRESS] = 0x00; // NOP
                current_address++;
                printf("Assembled NOP at %04X\n", current_address - 1);
            } else if (strcmp(instruction, "ADD") == 0) {
                output[current_address - START_ADDRESS] = 0x01; // ADD
                unsigned char operand_value;
                if (sscanf(operand, "%hhx", &operand_value) == 1) {
                    output[current_address - START_ADDRESS + 1] = operand_value;
                    current_address += 2; // Move to next instruction
                    printf("Assembled ADD #%02X at %04X\n", operand_value, current_address - 2);
                }
            } else if (strcmp(instruction, "OUTA") == 0) {
                output[current_address - START_ADDRESS] = 0x02; // OUTA
                current_address++;
                printf("Assembled OUTA at %04X\n", current_address - 1);
            } else if (strcmp(instruction, "JMP") == 0) {
                output[current_address - START_ADDRESS] = 0x03; // JMP
                int jump_address = resolve_label(operand);
                if (jump_address != -1) {
                    output[current_address - START_ADDRESS + 1] = jump_address & 0xFF; // Low byte
                    output[current_address - START_ADDRESS + 2] = (jump_address >> 8) & 0xFF; // High byte
                    current_address += 3;
                    printf("Assembled JMP to label %s at %04X\n", operand, current_address - 3);
                }
            } else if (strcmp(instruction, "JZ") == 0) {
                output[current_address - START_ADDRESS] = 0x04; // JZ
                int jump_address = resolve_label(operand);
                if (jump_address != -1) {
                    output[current_address - START_ADDRESS + 1] = jump_address & 0xFF;
                    output[current_address - START_ADDRESS + 2] = (jump_address >> 8) & 0xFF;
                    current_address += 3;
                }
            } else if (strcmp(instruction, "SUB") == 0) {
                output[current_address - START_ADDRESS] = 0x05; // SUB
                sscanf(operand, "%hhx", &output[current_address - START_ADDRESS + 1]);
                current_address += 2;
            } else if (strcmp(instruction, "JR") == 0) {
                output[current_address - START_ADDRESS] = 0x06; // JR
                int16_t offset;
                sscanf(operand, "%hd", &offset);
                output[current_address - START_ADDRESS + 1] = offset & 0xFF; // Low byte
                output[current_address - START_ADDRESS + 2] = (offset >> 8) & 0xFF; // High byte
                current_address += 3;
            } else if (strcmp(instruction, "CALL") == 0) {
                output[current_address - START_ADDRESS] = 0x07; // CALL
                int call_address = resolve_label(operand);
                if (call_address != -1) {
                    output[current_address - START_ADDRESS + 1] = call_address & 0xFF;
                    output[current_address - START_ADDRESS + 2] = (call_address >> 8) & 0xFF;
                    current_address += 3;
                }
            } else if (strcmp(instruction, "SETSP") == 0) {
                output[current_address - START_ADDRESS] = 0x08; // SETSP
                sscanf(operand, "%hhx", &output[current_address - START_ADDRESS + 1]);
                current_address += 2;
            } else if (strcmp(instruction, "TBA") == 0) {
                output[current_address - START_ADDRESS] = 0x09; // TBA
                current_address++;
            } else if (strcmp(instruction, "TAB") == 0) {
                output[current_address - START_ADDRESS] = 0x0A; // TAB
                current_address++;
            } else if (strcmp(instruction, "LDA") == 0) {
                output[current_address - START_ADDRESS] = 0x0B; // LDA
                int load_address = resolve_label(operand);
                if (load_address != -1) {
                    output[current_address - START_ADDRESS + 1] = load_address & 0xFF;
                    output[current_address - START_ADDRESS + 2] = (load_address >> 8) & 0xFF;
                    current_address += 3;
                } else {
                    sscanf(operand, "%hhx", &output[current_address - START_ADDRESS + 1]); // Immediate
                    current_address += 2;
                }
            } else if (strcmp(instruction, "LDB") == 0) {
                output[current_address - START_ADDRESS] = 0x0C; // LDB
                int load_address = resolve_label(operand);
                if (load_address != -1) {
                    output[current_address - START_ADDRESS + 1] = load_address & 0xFF;
                    output[current_address - START_ADDRESS + 2] = (load_address >> 8) & 0xFF;
                    current_address += 3;
                } else {
                    sscanf(operand, "%hhx", &output[current_address - START_ADDRESS + 1]); // Immediate
                    current_address += 2;
                }
            } else if (strcmp(instruction, "STA") == 0) {
                output[current_address - START_ADDRESS] = 0x10; // STA
                int store_address = resolve_label(operand);
                if (store_address != -1) {
                    output[current_address - START_ADDRESS + 1] = store_address & 0xFF;
                    output[current_address - START_ADDRESS + 2] = (store_address >> 8) & 0xFF;
                    current_address += 3;
                }
            } else if (strcmp(instruction, "STB") == 0) {
                output[current_address - START_ADDRESS] = 0x11; // STB
                int store_address = resolve_label(operand);
                if (store_address != -1) {
                    output[current_address - START_ADDRESS + 1] = store_address & 0xFF;
                    output[current_address - START_ADDRESS + 2] = (store_address >> 8) & 0xFF;
                    current_address += 3;
                }
            } else if (strcmp(instruction, "PHA") == 0) {
                output[current_address - START_ADDRESS] = 0x12; // PHA
                current_address++;
            } else if (strcmp(instruction, "PLA") == 0) {
                output[current_address - START_ADDRESS] = 0x13; // PLA
                current_address++;
            } else if (strcmp(instruction, "RTS") == 0) {
                output[current_address - START_ADDRESS] = 0x14; // RTS
                current_address++;
            } else if (strcmp(instruction, "CMP") == 0) {
                output[current_address - START_ADDRESS] = 0x16; // CMP
                sscanf(operand, "%hhx", &output[current_address - START_ADDRESS + 1]);
                current_address += 2;
            } else if (strcmp(instruction, "HLT") == 0) {
                output[current_address - START_ADDRESS] = 0x0F; // HLT
                current_address++;
            }
            // Handle more instructions as needed
        }
    }

    close(asm_fd);

    // Write output to file
    int bin_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (bin_fd < 0) {
        perror("Error opening binary file");
        exit(1);
    }

    write(bin_fd, output, current_address - START_ADDRESS);
    close(bin_fd);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Error: No input file specified.\n");
        return 1;
    }

    const char* output_file = "a.out"; // Default output
    if (argc == 4 && strcmp(argv[2], "-o") == 0) {
        output_file = argv[3];
    }

    assemble(argv[1], output_file);
    return 0;
}

