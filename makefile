CC = gcc
CFLAGS = -Wextra -Wall -Isrc/include -Lsrc/lib -lmingw32 -lSDL2main -lSDL2 -mwindows
EXEC = build/foxglove
OBJS = src/main.c src/chip8.c

all:
	$(CC) $(OBJS) $(CFLAGS) -o $(EXEC)
