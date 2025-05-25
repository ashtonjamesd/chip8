#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#include "chip8.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        perror("Expected *.ch8 file path.\n");
        return 1;
    }

    FILE *fptr = fopen(argv[1], "rb");
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

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow(
        "Chip-8", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        DISPLAY_WIDTH * SCALE, DISPLAY_HEIGHT * SCALE,
        SDL_WINDOW_SHOWN
    );

    if (!win) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    Chip8 *chip8 = init_chip8(1);
    run(chip8, buff, sz, renderer);

    SDL_DestroyWindow(win);
    SDL_Quit();

    free(chip8);

    return 0;
}