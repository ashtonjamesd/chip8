CC = gcc
CFLAGS = -Wextra -Wall
EXEC = build/foxglove
OBJS = src/main.c src/chip8.c

all:
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC) -lSDL2