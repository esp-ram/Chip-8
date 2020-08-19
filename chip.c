#include "chip.h"
#include <time.h>
#include <stdio.h>
#include <string.h>


void errorOpcode(int code){
    fprintf(stderr,"ERROR: invalid opcode: %X\n", code);
}


int chip_init(chip8* chip, const char* filename){
    FILE* gameFile = fopen(filename, "rb");
    if (!gameFile) {
        fprintf(stderr, "Unable to open file '%s'!\n", filename);
        return -1;
    }
    fseek(gameFile , 0 , SEEK_END);
	uint32_t gameSize = ftell(gameFile);
	rewind(gameFile);

    fread(&chip->memory[0x200], 1, gameSize, gameFile);
    fclose(gameFile);

    memcpy(chip->memory, font, sizeof(font));
    memset(chip->video, OFF_COLOR, sizeof(chip->video));
    memset(chip->stack, 0, sizeof(chip->stack));
    memset(chip->v, 0, sizeof(chip->v));
    memset(chip->keys, 0, sizeof(chip->keys));

    chip->drawFlag = false;
    chip->i = 0;
    chip->opcode = 0;
    chip->sp = 0;
    chip->soundTimer = 0;
    chip->delayTimer = 0;
    chip->pc = 0x200;

    srand(time(NULL));

    return 0;
}


void chip_update_timers(chip8* chip){
    if(chip->soundTimer > 0) chip->soundTimer--;
    if(chip->delayTimer > 0) chip->delayTimer--;
}


void emulateCycle(chip8* chip){
    chip->opcode = chip->memory[chip->pc] << 8 | chip->memory[chip->pc + 1];
    uint16_t operation = (chip->opcode & 0xF000) >> 12;
    uint16_t address = chip->opcode & 0x0FFF;
    uint16_t x = (chip->opcode & 0x0F00) >> 8;
    uint16_t y = (chip->opcode & 0x00F0) >> 4;
    uint16_t nn = chip->opcode & 0x00FF;
    uint16_t n = (chip->opcode & 0x000F);
    chip->pc += 2;
    switch(operation){
        case 0x0:
            switch (n) {
                case 0x0:
                    memset(chip->video, ON_COLOR, sizeof(chip->video));
                    chip->drawFlag = true;
                    break;
                case 0xE:
                    chip->sp -= 1;
                    chip->pc = chip->stack[chip->sp];
                    break;
                default:
                    errorOpcode(chip->opcode);
            }
            break;
        case 0x1:
            chip->pc = address;
            break;
        case 0x2:
            chip->stack[chip->sp] = chip->pc;
            chip->sp += 1;
            chip->pc = address;
            break;
        case 0x3:
            if(chip->v[x] == nn){
                chip->pc += 2;
            }
            break;
        case 0x4:
            if(chip->v[x] != nn){
                chip->pc += 2;
            }
            break;
        case 0x5:
            if(chip->v[x] == chip->v[y]){
                chip->pc += 2;
            }
            break;
        case 0x6:
            chip->v[x] = nn;
            break;
        case 0x7:
            chip->v[x] += nn;
            break;
        case 0x8:
            switch(n){
                case 0x0:
                    chip->v[x] = chip->v[y];
                    break;
                case 0x1:
                    chip->v[x] = chip->v[x] | chip->v[y];
                    break;
                case 0x2:
                    chip->v[x] = chip->v[x] & chip->v[y];
                    break;
                case 0x3:
                    chip->v[x] = chip->v[x] ^ chip->v[y];
                    break;
                case 0x4:
                    if((chip->v[x] + chip->v[y]) > 255){
                        chip->v[0xF] = 1;
                    }
                    chip->v[x] = chip->v[x] + chip->v[y];
                    break;
                case 0x5:
                    if(chip->v[x] < chip->v[y]){
                        chip->v[0xF] = 0;
                    }
                    chip->v[x] = chip->v[x] - chip->v[y];
                    break;
                case 0x6:
                    chip->v[0xF] = chip->v[x] & 0x1;
                    chip->v[x] >>= 1;
                    break;
                case 0x7:
                    if(chip->v[y] < chip->v[x]){
                        chip->v[0xF] = 0;
                    }
                    chip->v[x] = chip->v[y] - chip->v[x];
                    break;
                case 0xE:
                    chip->v[0xF] = (bool)chip->v[x] & 0x80;
                    chip->v[x] <<= 1;
                    break;
                default:
                    errorOpcode(chip->opcode);
            }
            break;
        case 0x9:
            if(chip->v[x] != chip->v[y]){
                chip->pc += 2;
            }
            break;
        case 0xA:
            chip->i = address;
            break;
        case 0xB:
            chip->pc = address + chip->v[0];
            break;
        case 0xC:
            chip->v[x] = nn & (rand() % 256);
            break;
        case 0xD:
            chip->v[0xF] = 0;
            for(int y_line = 0; y_line<n; y_line++){
                uint8_t pixel = chip->memory[chip->i + y_line];
                for(int x_line = 0; x_line < 8; x_line++){
                    if (pixel & (0x80 >> x_line)){
                        int index = ((chip->v[x] + x_line) % SCREEN_WIDTH) + (((chip->v[y] + y_line) % SCREEN_HEIGHT) * SCREEN_WIDTH);
                        if (chip->video[index] == ON_COLOR) {
                            chip->v[0xF] = 1;
                            chip->video[index] = OFF_COLOR;
                        }else {
                            chip->video[index] = ON_COLOR;
                        }
                    }
                }
            }
            chip->drawFlag = true;
            break;
        case 0xE:
            switch (n) {
                case 0xE:
                    if(SDL_GetKeyboardState(NULL)[key_map[chip->v[x]]]){
                        chip->pc += 2;
                    }
                    break;
                case 0x1:
                    if(!SDL_GetKeyboardState(NULL)[key_map[chip->v[x]]]){
                        chip->pc += 2;
                    }
                    break;
                default:
                    errorOpcode(chip->opcode);
            }
            break;
        case 0xF:
            switch (nn) {
                case 0x07:
                    chip->v[x] = chip->delayTimer;
                    break;
                case 0x0A:
                    chip->pc -= 2;
                    for(int i = 0; i < 16; i++){
                        if(SDL_GetKeyboardState(NULL)[key_map[i]]){
                            chip->v[x] = i;
                            chip->pc += 2;
                            break;
                        }
                    }
                    break;
                case 0x15:
                    chip->delayTimer = chip->v[x];
                    break;
                case 0x18:
                    chip->soundTimer = chip->v[x];
                    break;
                case 0x1E:
                    chip->i += chip->v[x];
                    break;
                case 0x29:
                    chip->i = chip->v[x] * 5;
                    break;
                case 0x33:
                    chip->memory[chip->i]     = chip->v[x] / 100;
                    chip->memory[chip->i + 1] = (chip->v[x] / 10) % 10;
                    chip->memory[chip->i + 2] = chip->v[x] % 10;
                    break;
                case 0x55:
                    memcpy(chip->memory + chip->i, chip->v, x+1);
                    break;
                case 0x65:
                    memcpy(chip->v, chip->memory + chip->i, x+1);
                    break;
                default:
                    errorOpcode(chip->opcode);
            }
            break;
        default:
            errorOpcode(chip->opcode);
    };
    chip_update_timers(chip);
}
