#include "chip8.hpp"

    const unsigned int start_address = 0x200;
    const unsigned int fontset_start_address = 0x50;
    const unsigned int fontset_size = 80;
    uint8_t fontset[fontset_size] =
    {
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

Chip8::Chip8()
    //Seeding random value
    :r(std::chrono::system_clock::now().time_since_epoch().count()) {

    //Initializing program counter to first unreserved byte 0x200
    program_counter = start_address;

    //Initializing RNG seed
    rand_byte = std::uniform_int_distribution<uint8_t>(0, 255U);

    //Loading fontset into memory
    for (unsigned int i = 0; i < fontset_size; i++) {
        memory[fontset_start_address + i] = fontset[i];
    }
    
    //Filling master function pointer table
    table[0x0] = &Chip8::Table0;
    table[0x1] = &Chip8::op_1nnn;
    table[0x2] = &Chip8::op_2nnn;
    table[0x3] = &Chip8::op_3xkk;
    table[0x4] = &Chip8::op_4xkk;
    table[0x5] = &Chip8::op_5xy0;
    table[0x6] = &Chip8::op_6xkk;
    table[0x7] = &Chip8::op_7xkk;
    table[0x8] = &Chip8::Table8;
    table[0x9] = &Chip8::op_9xy0;
    table[0xA] = &Chip8::op_annn;
    table[0xB] = &Chip8::op_bnnn;
    table[0xC] = &Chip8::op_cxkk;
    table[0xD] = &Chip8::op_dxyn;
    table[0xE] = &Chip8::TableE;
    table[0xF] = &Chip8::TableF;

    //Filling size 15 pointer tables (0, 8, E) w/ op_null pointers for invalid operands
    for (size_t i = 0; i < 0xE; i++) {
        table0[i] = &Chip8::op_null;
        table8[i] = &Chip8::op_null;
        tableE[i] = &Chip8::op_null;
    }

    //Filling function pointer tables 0, 8, E
    table0[0x0] = &Chip8::op_00e0;
    table0[0xE] = &Chip8::op_00ee;

    table8[0x0] = &Chip8::op_8xy0;
    table8[0x1] = &Chip8::op_8xy1;
    table8[0x2] = &Chip8::op_8xy2;
    table8[0x3] = &Chip8::op_8xy3;
    table8[0x4] = &Chip8::op_8xy4;
    table8[0x5] = &Chip8::op_8xy5;
    table8[0x6] = &Chip8::op_8xy6;
    table8[0x7] = &Chip8::op_8xy7;
    table8[0xE] = &Chip8::op_8xye;

    tableE[0x1] = &Chip8::op_exa1;
    tableE[0xE] = &Chip8::op_ex9e;

    //Filling F pointer table w/ op_null pointers for invalid operands
    for (size_t i = 0; i < 0x65; i++) {
        tableF[i] = &Chip8::op_null;
    }

    //Filling pointer table F
    tableF[0x07] = &Chip8::op_fx07;
    tableF[0x0A] = &Chip8::op_fx0a;
    tableF[0x15] = &Chip8::op_fx15;
    tableF[0x18] = &Chip8::op_fx18;
    tableF[0x1E] = &Chip8::op_fx1e;
    tableF[0x29] = &Chip8::op_fx29;
    tableF[0x33] = &Chip8::op_fx33;
    tableF[0x55] = &Chip8::op_fx55;
    tableF[0x65] = &Chip8::op_fx65;
}

//Function pointer loaders
//Runs operation from given pointer table based on opcode
//Ex: 81A0 -> table0[81A0 & 000Fu] -> table8[0000] -> op_8xy0() (LD V1, VA)
//Ex: 00E3 -> table0[00E3 & 000Fu] -> table0[0003] -> op_null() (Do nothing)
void Chip8::Table0() {
    ( (*this).*(table0[opcode & 0x000Fu]) )();
}

void Chip8::Table8() {
    ( (*this).*(table8[opcode & 0x000Fu]) )();
}

void Chip8::TableE() {
    ( (*this).*(tableE[opcode & 0x000Fu]) )();
}

void Chip8:: TableF() {
    ( (*this).*(tableF[opcode & 0x000Fu]) )();
}


//ROM Loader
//Reads ROM as binary and loads into memory using buffer array
void Chip8::loadRom(const char* filename) {
    std:: ifstream file(filename, std::ios::binary | std::ios::ate);

    if (file.is_open()) {
        std::streampos filesize = file.tellg();
        char* buffer = new char[filesize];

        file.seekg(0, std::ios::beg);
        file.read(buffer, filesize);
        file.close();

        for (long i = 0; i < filesize; i++) {
            memory[start_address + i] = buffer[i];
        }
            delete[] buffer;
    }
}

//Fetch-Decode-Execute cycle
void Chip8::cycle() {

    //Fetch opcode from memory (get first half, left shift, get second half)
    //opcode is 16-bit, so it is stored across 2 8-bit memory values
    opcode = (memory[program_counter] << 8u) | memory[program_counter + 1];

    //Increment program counter before execution
    program_counter += 2;

    //Decode and Execute: use first bit of opcode as index into function pointer table
    //Ex. 00E0 -> table[0x1nnn & 0xF000] -> table[0x1000 >> 12u] -> table[0x1] -> op_1nnn
    ( (*this).*(table[(opcode & 0xF000u) >> 12u]) )();

    //Decrement delay and sound timers if set
    if (delay_timer > 0) delay_timer--;
    if (sound_timer > 0) sound_timer--;
}

