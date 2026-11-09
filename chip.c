#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "chip.h"

const uint8_t fontset[FONT_SIZE] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void initCHIP8(Chip8 *chip8) {

    chip8->pc = START_ADDRESS; // STARTED THE PC FROM THE 0x200.
    chip8->opcode = 0;
    chip8->index = 0;
    chip8->sp = 0;

    // clear stack
    for (int i = 0; i < STACK_SIZE; i++) {
        chip8->stack[i] = 0;
    }

    // clear registers
    for (int i = 0; i < REGISTERS_NUMS; i++) {
        chip8->registers[i] = 0;
    }

    // clear memory
    for (int i = 0; i < MEMORY_SIZE; i++) {
        chip8->memory[i] = 0;
    }

    // clear Display
    for (int i = 0; i < SCREEN_HEIGHT * SCREEN_WIDTH; i++) {
        chip8->video[i] = 0;
    }

    // clear keypad
    for (int i = 0; i < NUM_KEYS; i++) {
        chip8->keypad[i] = 0;
    }

    // loading fontset
    for (int i = 0; i < 80; i++) {
        chip8->memory[FONT_ADDRESS + i] = fontset[i];
    }

    // reset timers
    chip8->delayTimer = 0;
    chip8->soundTimer = 0;
}

void loadROM(Chip8 *chip8, char *filename) {
    FILE *fp;
    fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Error couldn't open ROM %s\n", filename);
        return;
    }

    int c;
    int i = 0;
    while ((c = fgetc(fp)) != EOF ) {
        if (START_ADDRESS + i >= MEMORY_SIZE) {
            fprintf(stderr, "ROM size is too large\n");
            fclose(fp);
            return;
        }
        chip8->memory[START_ADDRESS + i] = c;
        i++;
    }

    fclose(fp);
}

void emulateCycle (Chip8 *chip8){

        // here << is bitwise left, used to occupy upper half 8 bits then | merges the two bytes together
        chip8->opcode = (chip8->memory[chip8->pc] << 8) | chip8->memory[chip8->pc + 1];
        uint16_t opcode = chip8->opcode;
        chip8->pc += 2;

        // imp variables to be used in instructions.
        uint16_t nnn = opcode & 0x0FFF; // last 12 bits, usually an address
        uint8_t n = opcode & 0x000F; // forth nibble, 4 bits
        uint8_t x = (opcode & 0x0F00) >> 8; // second nibble, usually a register index
        uint8_t y = (opcode & 0x00F0) >> 4; // third nibble, usually a register index
        uint8_t nn = opcode & 0x00FF; // last two nibbles, lower byte

        switch (opcode & 0xF000) // gives us the first nibble
        {
            case 0x0000:
                switch (opcode & 0x00FF) {
                    case 0x00E0: // 0x00E0 clears the screen, we got the last nibble from it.
                        memset(chip8->video, 0, sizeof(chip8->video));
                        break;
                    case 0x00EE: // 0x00EE return from subroutine
                        chip8->pc = chip8->stack[--chip8->sp];
                        break;
                    default:
                        fprintf(stderr, "Unknown opcode [0x0000]: 0x0%4X pc = 0x%4X\n", opcode, chip8->pc);
                        break;
                }
                break;

            case 0x1000: // 0x1nnn jmp to address nnn
                chip8->pc = nnn;
                break;

            case 0x2000: // 0x2nnn Call address to nnn
                chip8->stack[chip8->sp] = chip8->pc;
                chip8->sp++;
                chip8->pc = nnn;
                break;

            case 0x3000: // 0x3xnn if condition is true skip next instruction: SE = skip if equal
                if (chip8->registers[x] == nn) chip8->pc += 2;
                break;

            case 0x4000: // 0x4xnn skip next instruction: SNE = skip if not equal
                if (chip8->registers[x] != nn) chip8->pc += 2;
                break;

            case 0x5000: // 0x5xyo SE vx, vy
                if (chip8->registers[x] == chip8->registers[y]) chip8->pc += 2;
                break;

            case 0x6000: // 0x6xnn set register VX
                chip8->registers[x] = nn;
                break;

            case 0x7000: // 0x7xnn add some bytes in vx
                chip8->registers[x] += nn;
                break;

            case 0x8000:
                switch (opcode & 0x000F) {
                    case 0x0000:
                        chip8->registers[x] = chip8->registers[y];
                        break;

                    case 0x0001:
                        chip8->registers[x] |= chip8->registers[y];
                        break;

                    case 0x0002:
                        chip8->registers[x] &= chip8->registers[y];
                        break;

                    case 0x0003:
                        chip8->registers[x] ^= chip8->registers[y];
                        break;

                    case 0x0004:
                        uint16_t sum = chip8->registers[x] + chip8->registers[y];
                        chip8->registers[0xF] = (sum > 0xFF) ? 1 : 0;
                        chip8->registers[x] = sum & 0xFF; // if overflow keep only the lower 8 bits in vx
                        break;

                    case 0x0005:
                        chip8->registers[0xF] = (chip8->registers[x] > chip8->registers[y]) ? 1: 0;
                        chip8->registers[x] -= chip8->registers[y];
                        break;

                    case 0x0006: // SHR vx = Shift right
                        chip8->registers[0xF] = (chip8->registers[x] & 0x1);
                        chip8->registers[x] >>= 1;
                        break;

                    case 0x0007: // SUBN = subtract not(reverse subtract): vx = vy - vx
                        chip8->registers[0xF] = (chip8->registers[y] > chip8->registers[x])? 1: 0; // 1 if not borrow, 0 otherwise
                        chip8->registers[x] = chip8->registers[y] - chip8->registers[x];
                        break;

                    case 0x000E:
                        chip8->registers[0xF] = (chip8->registers[x] & 0x80) >> 7;
                        chip8->registers[x] <<= 1;
                        break;

                    default:
                        fprintf(stderr, "Unknown opcode [0x8000]: 0x0%4X pc = 0x%4X\n", opcode, chip8->pc);
                        break;
                }
                break;

            case 0x9000: // 0x9xy0 SNE vx vy
                if (chip8->registers[x] != chip8->registers[y]) chip8->pc += 2;
                break;

            case 0xA000: // 0xAnnn, set index register I to nnn(address)
                chip8->index = nnn;
                break;

            case 0xB000: // 0xBnnn, jmp to v0 + address(nnn)
                chip8->pc = chip8->registers[0x0] + nnn;
                break;

            case 0xC000: // 0xCxnn RND= random byte AND'd with kk stored in Vx
                chip8->registers[x] = (rand() & 0xFF) & nn;
                break;

            case 0xD000: // 0xDXYN display/Draw
                {  // Declaration after case is invalid c that's why we used curly brackets

                uint8_t x_loc = chip8->registers[x]; // get the value of Vx for the coordinates
                uint8_t y_loc = chip8->registers[y];
                uint8_t spriteByte;
                chip8->registers[0xF] = 0;
                for (int rows = 0; rows < n; rows++) {
                    spriteByte = chip8->memory[chip8->index + rows];
                   for (int col = 0; col < 8; col++) {
                       uint8_t spritePixel = spriteByte & (0x80 >> col);
                       int sx = (x_loc + col) % SCREEN_WIDTH;
                       int sy = (y_loc + rows) % SCREEN_HEIGHT;
                       uint32_t *screenPixel = &chip8->video[(sx + (sy * SCREEN_WIDTH))];

                       if (spritePixel) {
                           if (*screenPixel == 0xFFFFFFFF) {
                               chip8->registers[0xF] = 1; // collision
                           }

                           *screenPixel ^= 0xFFFFFFFF;
                       }
                   }
                }
                // chip8->drawFlag = true;
                // chip8->pc += 2;
                break;
                }

            case 0xE000: // 0xEx9e
                switch (opcode & 0x00FF) {
                    case 0x009E:
                        if (chip8->keypad[chip8->registers[x]]) {
                            chip8->pc += 2;
                        }
                        break;

                    case 0x00A1:
                        if (!chip8->keypad[chip8->registers[x]]) {
                            chip8->pc += 2;
                        }
                        break;

                    default:
                        fprintf(stderr, "Unknown opcode [0xE000]: 0x0%4X pc = 0x%4X\n", opcode, chip8->pc);
                        break;
                }
                break;

            case 0xF000: // 0xFx__ Timers instructions
                switch (opcode & 0x00FF) {
                    case 0x0007:
                        // printf("[DT READ] PC: %03X | DT: %d\n",
                        //            chip8->pc - 2,
                        //            chip8->delayTimer);
                        chip8->registers[x] = chip8->delayTimer;
                        break;

                    case 0x001E:
                        chip8->index += chip8->registers[x];
                        chip8->registers[0xF] = (chip8->index > 0x0FFF)? 1: 0;
                        break;

                    case 0x000A: {
                        int keypressed = 0;
                        for (int i = 0; i < NUM_KEYS; i++) {
                            if(chip8->keypad[i]) {
                                chip8->registers[x] = i;
                                keypressed = 1;
                                break;
                            }
                        }
                        if (!keypressed) chip8->pc -= 2;
                        break;
                    }

                    case 0x0015:
                        // printf("\n[DT RELOAD] PC: %03X | VX: %d\n",
                        //            chip8->pc,

                        //            chip8->registers[x]);

                        chip8->delayTimer = chip8->registers[x];
                        break;

                    case 0x0018:
                        chip8->soundTimer = chip8->registers[x];
                        break;

                    case 0x0029:
                        chip8->index = FONT_ADDRESS + (5 * chip8->registers[x]);
                        break;

                    case 0x0033:

                        uint8_t value = chip8->registers[x];
                        // ones place
                        chip8->memory[chip8->index + 2] = value % 10;
                        value /= 10;

                        // tens place
                        chip8->memory[chip8->index + 1] = value % 10;
                        value /= 10;

                        // hundreds place
                        chip8->memory[chip8->index] = value % 10;
                        break;

                    case 0x0055:

                        for (uint8_t i = 0; i <= x; i++) {
                            chip8->memory[chip8->index + i] = chip8->registers[i];
                        }
                        break;

                    case 0x0065:
                        for (uint8_t i = 0; i <= x; i++) {
                            chip8->registers[i] = chip8->memory[chip8->index + i];
                        }
                        break;

                    default:
                        fprintf(stderr, "Unknown opcode [0xF000]: 0x0%4X pc = 0x%4X\n", opcode, chip8->pc);
                        break;

                }
                break;

            default:
                fprintf(stderr, "Unknown opcode: 0x0%4X pc = 0x%4X\n", opcode, chip8->pc);
                break;
        }

}
