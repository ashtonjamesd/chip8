#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "chip8.h"

int main() {
    FILE *fptr = fopen("example/test_opcode.ch8", "rb");
    if (!fptr) {
        perror("Error opening file.");
        return 1;
    }

    fseek(fptr, 0, SEEK_END);
    long sz = ftell(fptr);
    rewind(fptr);

    uint8_t *buff = malloc(sz + 1);
    if (!buff) {
        perror("Memory allocation failed.");
        fclose(fptr);
        return 1;
    }

    fread(buff, 1, sz, fptr);

    Chip8 *chip8 = init_chip8(1);
    run(chip8, buff, sz);
    // run(chip8, program, 6);

    // for (int i = 0; i < 16; i++) {
    //     printf("V[%d]: %d\n", i, chip8->V[i]);
    // }

    free(chip8);

    return 0;
}