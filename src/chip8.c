#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "chip8.h"
#include "fontset.h"

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define SCALE 10

Chip8 *init_chip8(int use_original_shift_behavior) {
    Chip8 *chip8 = malloc(sizeof(Chip8));
    if (!chip8) {
        fprintf(stderr, "Failed to allocate Chip8\n");
        exit(1);
    }

    chip8->pc = 0x200;
    chip8->I = 0;
    chip8->sp = 0;
    chip8->delay_timer = 0;
    chip8->sound_timer = 0;

    chip8->use_original_shift_behavior = use_original_shift_behavior;

    memset(chip8->V, 0, sizeof(chip8->V));
    memset(chip8->memory, 0, sizeof(chip8->memory));
    memset(chip8->display, 0, sizeof(chip8->display));
    memset(chip8->stack, 0, sizeof(chip8->stack));
    memset(chip8->keys, 0, sizeof(chip8->keys));

    return chip8;
}

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
    for (uint16_t i = 0; i < size; i++) {
        chip8->memory[chip8->pc + i] = program[i];
    }
}

void run(Chip8 *chip8, uint8_t program[], size_t size) {
    initialise(chip8);
    load(chip8, program, size);

    while (1) {
        uint8_t instr1 = chip8->memory[chip8->pc];
        chip8->pc++;

        uint8_t instr2 = chip8->memory[chip8->pc];
        chip8->pc++;

        uint8_t higher_nibble = instr1 >> 4;
        uint16_t both_instr = (instr1 << 8) | instr2;

        switch (higher_nibble) {
            case 0x0: {
                if (instr2 == 0xe0) {
                    for (int i = 0; i < 64 * 32; i++) {
                     chip8->display[i] = 0;
                    }
                }
                else if (instr2 == 0xee) {
                    chip8->pc = chip8->stack[--chip8->sp];
                }
                break;
            }

            case 0x1: {
                uint16_t addr = both_instr & 0xfff;
                chip8->pc = addr;
                break;
            }

            case 0x2: {
                uint16_t addr = both_instr & 0xfff;
                chip8->stack[chip8->sp++] = addr;
                chip8->pc = addr;
                break;
            }
            
            case 0x3: {
                uint8_t reg = instr1 & 0xf;

                if (chip8->V[reg] == instr2) {
                    chip8->pc += 2;
                }
                break;
            }

            case 0x4: {
                uint8_t reg = instr1 & 0xf;

                if (chip8->V[reg] != instr2) {
                    chip8->pc += 2;
                }
                break;
            }
            
            case 0x5: {
                uint8_t x = instr1 & 0xf;
                uint8_t y = (instr2 >> 4) & 0xf;

                if (chip8->V[x] == chip8->V[y]) {
                    chip8->pc += 2;
                }
                break;
            }

            case 0x6: {
                uint8_t set_reg = instr1 & 0xf;
                uint8_t set_val = instr2;
                chip8->V[set_reg] = set_val;
                break;
            }

            case 0x7: {
                uint8_t add_reg = instr1 & 0xf;
                uint8_t add_val = instr2;
                chip8->V[add_reg] += add_val;
                break;
            }

            case 0x8: {
                switch (instr2 & 0xf) {
                    case 0x0: {
                        uint8_t x = instr1 & 0xf;
                        uint8_t y = (instr2 >> 4) & 0xf;

                        chip8->V[x] = chip8->V[y]; 
                        break;
                    }

                    case 0x01: {
                        uint8_t x = instr1 & 0xf;
                        uint8_t y = (instr2 >> 4) & 0xf;

                        chip8->V[x] |= chip8->V[y]; 
                        break;
                    }

                    case 0x02: {
                        uint8_t x = instr1 & 0xf;
                        uint8_t y = (instr2 >> 4) & 0xf;

                        chip8->V[x] &= chip8->V[y]; 
                        break;
                    }

                    case 0x03: {
                        uint8_t x = instr1 & 0xf;
                        uint8_t y = (instr2 >> 4) & 0xf;

                        chip8->V[x] ^= chip8->V[y]; 
                        break;
                    }

                    case 0x04: {
                        uint8_t x = instr1 & 0xf;
                        uint8_t y = (instr2 >> 4) & 0xf;

                        uint16_t sum = chip8->V[x] + chip8->V[y];
                        chip8->V[0xf] = (sum > 0xff) ? 1 : 0;
                        chip8->V[x] = (uint8_t)sum;
                        break;
                    }

                    case 0x05: {
                        uint8_t x = instr1 & 0xf;
                        uint8_t y = (instr2 >> 4) & 0xf;

                        chip8->V[0xf] = (chip8->V[x] >= chip8->V[y]) ? 1 : 0;
                        chip8->V[x] -= chip8->V[y]; 
                        break;
                    }

                    case 0x06: {
                        uint8_t x = instr1 & 0xf;
                        uint8_t y = (instr2 >> 4) & 0xf;

                        if (chip8->use_original_shift_behavior) {
                            chip8->V[x] = chip8->V[y];
                            chip8->V[0xf] = chip8->V[x] & 0x1;
                            chip8->V[x] >>= 1;
                        } else {
                            chip8->V[0xf] = chip8->V[x] & 0x1;
                            chip8->V[x] >>= 1;
                        }
                        break;
                    }

                    case 0x07: {
                        uint8_t x = instr1 & 0xf;
                        uint8_t y = (instr2 >> 4) & 0xf;

                        chip8->V[0xf] = (chip8->V[y] >= chip8->V[x]) ? 1 : 0;
                        chip8->V[x] = chip8->V[y] - chip8->V[x];; 
                        break;
                    }

                    case 0x0e: {
                        uint8_t x = instr1 & 0xf;
                        uint8_t y = (instr2 >> 4) & 0xf;

                        if (chip8->use_original_shift_behavior) {
                            chip8->V[x] = chip8->V[y];
                            chip8->V[0xf] = (chip8->V[x] & 0x80) >> 7;
                            chip8->V[x] <<= 1;
                        } else {
                            chip8->V[0xf] = (chip8->V[x] & 0x80) >> 7;
                            chip8->V[x] <<= 1;
                        }
                        break;
                    }
                }
                break;
            }
            case 0x9: {
                uint8_t x = instr1 & 0xf;
                uint8_t y = (instr2 >> 4) & 0xf;

                if (chip8->V[x] != chip8->V[y]) {
                    chip8->pc += 2;
                }
                break;
            }

            case 0xa: {
                uint16_t set_index_val = both_instr & 0xfff;
                chip8->I = set_index_val;
                break;
            }

            case 0xb: {
                uint16_t offset = both_instr & 0xfff;
                chip8->pc = offset + chip8->V[0];
                break;
            }

            case 0xc: {
                uint8_t x = instr1 & 0xf;
                uint8_t nn = instr2;

                uint8_t random_byte = rand() % 256;
                chip8->V[x] = random_byte & nn;

                break;
            }
                
            case 0xd: {
                uint8_t x = instr1 & 0xf;
                uint8_t y = (instr2 >> 4) & 0xf;
                uint8_t n = instr2 & 0xf;

                display(chip8, x, y, n);
                break;
            }

            case 0xe: {
                uint8_t key = instr1 & 0xf;
                
                if (instr2 == 0x9e) {
                    if (chip8->keys[key]) {
                        chip8->pc += 2;
                    }
                }
                else if (instr2 == 0xa1) {
                    if (!chip8->keys[key]) {
                        chip8->pc += 2;
                    }
                }
                break;
            }

            case 0xf: {
                uint8_t x = instr1 & 0xf;
                
                switch (instr2) {
                    case 0x07: {
                        chip8->V[x] = chip8->delay_timer;
                        break;
                    }

                    case 0x15: {
                        chip8->delay_timer = chip8->V[x];
                        break;
                    }

                    case 0x18: {
                        chip8->sound_timer = chip8->V[x];
                        break;
                    }

                    case 0x1e: {
                        chip8->I += chip8->V[x];
                        break;
                    }

                    case 0x0a: {
                        // get key
                        break;
                    }

                    case 0x29: {
                        uint8_t character = chip8->V[x] & 0xF;
                        chip8->I = character * 5;
                        break;
                    }

                    case 0x33: {
                        uint8_t val = chip8->V[x];
                        chip8->memory[chip8->I]     = val / 100;
                        chip8->memory[chip8->I + 1] = (val / 10) % 10;
                        chip8->memory[chip8->I + 2] = val % 10;
                        break;
                    }

                    case 0x55: {
                        for (int i = 0; i <= x; i++) {
                            chip8->memory[chip8->I + i] = chip8->V[i]; 
                        }
                        break;
                    }

                    case 0x65: {
                        for (int i = 0; i <= x; i++) {
                            chip8->V[i] = chip8->memory[chip8->I + i];
                        }
                        break;
                    }
                }
                break;
            }

            default:
                printf("unknown opcode: '%c'", higher_nibble);
                return;
        }
        
        usleep(50000);
        
        render(chip8);
    }
}