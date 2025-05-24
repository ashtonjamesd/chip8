#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "chip8.h"
#include "fontset.h"

Chip8 *init_chip8() {
    Chip8 *chip8 = malloc(sizeof(Chip8));
    chip8->pc = 0;
    chip8->I = 0;

    return chip8;
}

static inline void tick(Chip8 *chip8) {
    chip8->pc++;
}

static inline uint8_t instr(Chip8 *chip8) {
    return chip8->memory[chip8->pc];
}

static inline void clear(Chip8 *chip8) {
    for (int i = 0; i < 64 * 32; i++) {
        chip8->display[i] = 0;
    }
}

static inline void jump(Chip8 *chip8, uint16_t address) {
    chip8->pc = address;
}

static inline void set_register(Chip8 *chip8, uint8_t reg, uint8_t val) {
    chip8->V[reg] = val;
}

static inline void add_register(Chip8 *chip8, uint8_t reg, uint8_t val) {
    chip8->V[reg] += val;
}

static inline void set_index_register(Chip8 *chip8, uint16_t val) {
    chip8->I = val;
}

static inline void display(Chip8 *chip8, uint8_t x, uint8_t y, uint8_t n) {
    uint8_t x_val = chip8->V[x] % 64;
    uint8_t y_val = chip8->V[y] % 32;
    chip8->V[0xf] = 0;

    for (int row = 0; row < n; row++) {
        uint8_t sprite_byte = chip8->memory[chip8->I + row];
        
        for (int col = 0; col < 8; col++) {
            uint8_t sprite_pixel = (sprite_byte >> (7 - col)) & 1;

            uint16_t idx = (y_val + row) * 64 + (x_val + col);
            uint8_t *screen_pixel = &chip8->display[idx];

            if (sprite_pixel) {
                if (*screen_pixel == 1) {
                    chip8->V[0xf] = 1;
                }
                *screen_pixel ^= 1;
            }
        }
    }
}

static inline void initialise(Chip8 *chip8) {
    memset(chip8->memory, 0, sizeof(chip8->memory));
    memcpy(&chip8->memory[0], chip8_font, sizeof(chip8_font));
}

static inline void load(Chip8 *chip8, uint8_t program[], size_t size) {
    chip8->pc = 0x200;

    for (uint16_t i = 0; i < size; i++) {
        chip8->memory[chip8->pc + i] = program[i];
    }
}

void run(Chip8 *chip8, uint8_t program[], size_t size) {
    initialise(chip8);
    load(chip8, program, size);

    int i = 0;
    while (i++ < size / 2) {
        uint8_t instr1 = instr(chip8);
        tick(chip8);

        uint8_t instr2 = instr(chip8);
        tick(chip8);

        uint8_t higher_nibble = instr1 >> 4;
        uint16_t both_instr = (instr1 << 8) | instr2;

        switch (higher_nibble) {
            case 0x0:
                if (instr2 == 0xe0) {
                    clear(chip8);
                }
                break;

            case 0x1:
                uint16_t nnn = both_instr & 0xfff;
                jump(chip8, nnn);
                break;
            
            case 0x6:
                uint8_t set_reg = instr1 & 0xf;
                uint8_t set_val = instr2;
                set_register(chip8, set_reg, set_val);
                break;

            case 0x7:
                uint8_t add_reg = instr1 & 0xf;
                uint8_t add_val = instr2;
                add_register(chip8, add_reg, add_val);
                break;

            case 0xa:
                uint16_t set_index_val = both_instr & 0xfff;
                set_index_register(chip8, set_index_val);
                break;

            case 0xd:
                uint8_t x = instr1 & 0xf;
                uint8_t y = (instr2 >> 4) & 0xf;
                uint8_t n = instr2 & 0xf;

                display(chip8, x, y, n);
                break;

            default:
                printf("unknown opcode: '%c'", higher_nibble);
        }
    }
}