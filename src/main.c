#include <stdio.h>

#include "chip8.h"

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define SCALE 10

void render(Chip8 *chip8) {
    printf("\033[2J\033[H");

    for (int y = 0; y < DISPLAY_HEIGHT; y++) {
        for (int x = 0; x < DISPLAY_WIDTH; x++) {
            int index = y * DISPLAY_WIDTH + x;

            if (chip8->display[index]) {
                printf("#");
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }

    printf("\nPress Enter to exit...");
    getchar();
}

int main() {
    FILE *fptr = fopen("example/ibm_logo.ch8", "rb");
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

    // for (long i = 0; i < sz; i++) {
    //     printf("%02X ", buff[i]);
    // }

    Chip8 *chip8 = init_chip8();
    run(chip8, buff, sz);

    render(chip8);

    free(chip8);

    return 0;
}