CC=gcc
LD=gcc

OBJS=main.o cpu.o

%.o: %.c
	$(CC) $< -o $@ -c

i8e: $(OBJS)
	$(LD) $^ -o $@
