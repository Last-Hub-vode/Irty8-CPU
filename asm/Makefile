CC = gcc
CFLAGS = -Wall

# Target for the assembler
all: i8a

# Rule to build the assembler
i8a: main.c
	$(CC) $(CFLAGS) main.c -o i8a

# Clean up generated files
clean:
	rm -f i8a

