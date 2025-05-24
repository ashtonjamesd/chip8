CC = gcc
CFLAGS = -Wextra -Wall
EXEC = build/foxglove
OBJS = src/main.c

all:
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)