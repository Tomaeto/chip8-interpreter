#pragma once

#include <cstdint>
#include <fstream>
#include <random>
#include <chrono>
#include <string.h>

const unsigned int REG_COUNT = 16;
const unsigned int MEM_SIZE = 4096;
const unsigned int STACK_SIZE = 16;
const unsigned int KEY_COUNT = 16;
const unsigned int VIDEO_WIDTH = 64;
const unsigned int VIDEO_HEIGHT = 32;
class Chip8 {
    public:
        Chip8();

        uint8_t keypad[KEY_COUNT]{};
        uint32_t video[VIDEO_HEIGHT * VIDEO_WIDTH]{};

        void loadRom(const char* filename);
        void cycle();

    private: 
        uint8_t registers[REG_COUNT]{};
        uint8_t memory[MEM_SIZE]{};
        uint16_t stack[STACK_SIZE]{};
        uint16_t memory_index{};
        uint16_t program_counter{};
        uint8_t stack_pointer{};
        uint8_t delay_timer{};
        uint8_t sound_timer{};
        uint16_t opcode;

        std::default_random_engine r;
        std::uniform_int_distribution<uint8_t> rand_byte;

	    typedef void (Chip8::*Chip8Func)();
        //Function pointer tables
	    Chip8Func table[0xF + 1]; 
        //Tables 0, 8, E have 16 possible permutations (last valid opcode ends in E)
        //Table F has 102 possible permutations (last valid opcode ends in 65)
	    Chip8Func table0[0xE + 1];
	    Chip8Func table8[0xE + 1];
	    Chip8Func tableE[0xE + 1]; 
	    Chip8Func tableF[0x65 + 1];

        void Table0();
        void Table8();
        void TableE();
        void TableF();

        //CHIP-8 instructions
        void op_00e0(); //CLS
        void op_00ee(); //RET
        void op_1nnn(); //JP nnn
        void op_2nnn(); //CALL nnn
        void op_3xkk(); //SE Vx, kk
        void op_4xkk(); //SNE Vx, kk
        void op_5xy0(); //SE Vx, Vy
        void op_6xkk(); //LD Vx, kk
        void op_7xkk(); //ADD Vx, kk
        void op_8xy0(); //LD Vx, Vy
        void op_8xy1(); //OR Vx, Vy
        void op_8xy2(); //AND Vx, Vy
        void op_8xy3(); //XOR Vx, Vy
        void op_8xy4(); //ADD Vx, Vy
        void op_8xy5(); //SUB Vx, Vy
        void op_8xy6(); //SHR Vx
        void op_8xy7(); //SUBN Vx, Vy
        void op_8xye(); //SHL Vx, {, Vy}
        void op_9xy0(); //SNE Vx, Vy
        void op_annn(); //LD i, nnn
        void op_bnnn(); //JP V0, addr
        void op_cxkk(); //RND Vx, kk
        void op_dxyn(); //DRW Vx, Vy, n
        void op_ex9e(); //SKP Vx
        void op_exa1(); //SKNP Vx
        void op_fx07(); //LD Vx, DT
        void op_fx0a(); //LD Vx, K
        void op_fx15(); //LD DT, Vx
        void op_fx18(); //LD ST, Vx
        void op_fx1e(); //ADD i, Vx
        void op_fx29(); //LD f, Vx
        void op_fx33(); //LD b, Vx
        void op_fx55(); //LD [i], Vx
        void op_fx65(); //LD Vx, [i]
        void op_null(); //Do nothing


};