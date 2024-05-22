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

//00E0: CLS
//Clear display
void Chip8::op_00e0() {
    memset(video, 0, sizeof(video));
}


//00EE: RET
//Return from subroutine
//Sets program counter to previous address on stack
void Chip8::op_00ee() {
    stack_pointer--;
    program_counter = stack[stack_pointer];
}

//1nnn: JP nnn
//Set program counter to address nnn
//Gets address from opcode by zeroing first value, leaving nnn
void Chip8::op_1nnn() {
    uint16_t address = opcode & 0x0FFFu;

    program_counter = address;
}

//2nnn: CALL nnn
//Calls subroutine at nnn
//Puts currently stored program counter onto stack, increments stack pointer, and updates program counter
void Chip8::op_2nnn() {
    uint16_t address = opcode & 0x0FFFu;

    stack[stack_pointer] = program_counter;
    stack_pointer++;
    program_counter = address;
}


//3xkk: SE Vx, kk
//Skips next instruction if Vx = kk
void Chip8::op_3xkk() {
    uint8_t vx = (opcode & 0x0F00u) >> 8u;
    uint8_t kk = opcode & 0x00FFu;
    if (registers[vx] == kk) program_counter += 2;
}

//4xkk: SNE Vx, kk
//Skips next instruction if Vx != kk
void Chip8::op_4xkk() {
    uint8_t vx = (opcode & 0x0F00u) >> 8u;
    uint8_t kk = opcode & 0x00FFu;
    if (registers[vx] != kk) program_counter += 2;
}


//5xy0: SE Vx, Vy
//Skips next instruction if Vx == Vy
void Chip8::op_5xy0() {
    uint8_t vx = (opcode & 0x0F00u) >> 8u;
    uint8_t vy = (opcode & 0x00F0u) >> 4u;
    if (registers[vx] == registers[vy]) program_counter += 2;
}

//6xkk: LD Vx, kk
//Sets Vx = kk
void Chip8::op_6xkk() {
    uint8_t vx = (opcode & 0x0F00u) >> 8u;
    uint8_t kk = opcode & 0x00FFu;
    registers[vx] = kk;
}

//7xkk: ADD Vx, kk
//Adds kk to Vx
void Chip8::op_7xkk() {
    uint8_t vx = (opcode & 0x0F00u) >> 8u;
    uint8_t kk = opcode & 0x00FFu;
    registers[vx] += kk;
}

//8xy0: LD Vx, Vy
//Sets Vx = Vy
void Chip8::op_8xy0() {
    uint8_t vx = (opcode & 0x0F00u) >> 8u;
    uint8_t vy = (opcode & 0x00F0u) >> 4u;
    registers[vx] = registers[vy];
}

//8xy1: OR Vx, Vy
//Sets Vx = Vx OR Vy
void Chip8::op_8xy1() {
    uint8_t vx = (opcode & 0x0F00u) >> 8u;
    uint8_t vy = (opcode & 0x00F0u) >> 4u;
    registers[vx] |= registers[vy];
}

//8xy2: AND Vx, Vy
//Sets Vx = Vx AND Vy
void Chip8::op_8xy2() {
    uint8_t vx = (opcode & 0x0F00u) >> 8u;
    uint8_t vy = (opcode & 0x00F0u) >> 4u;
    registers[vx] &= registers[vy];
}

//8xy3: XOR Vx, Vy
//Sets Vx = Vx XOR Vy
void Chip8::op_8xy3() {
    uint8_t vx = (opcode & 0x0F00u) >> 8u;
    uint8_t vy = (opcode & 0x00F0u) >> 4u;
    registers[vx] ^= registers[vy];
}

//8xy4: ADD Vx, Vy
//Sets Vx = Vx + Vy and VF = carry
//If result of Vx + Vy is over 8 bits (>255) set VF = 1, else VF = 0
void Chip8::op_8xy4() {
    uint8_t vx = (opcode & 0x0F00u) >> 8u;
    uint8_t vy = (opcode & 0x00F0u) >> 4u;

    uint16_t sum = registers[vx] + registers[vy];
    if (sum > 255U) registers[0xF] = 1;
    else registers[0xF] = 0;

    registers[vx] = sum & 0xFFu;
}

//8xy5: SUB Vx, Vy
//Sets Vx = Vx - Vy and VF = NOT borrow
//If Vx > Vy, set VF = 1, else VF = 0
void Chip8::op_8xy5() {
    uint8_t vx = (opcode & 0x0F00u) >> 8u;
    uint8_t vy = (opcode & 0x00F0u) >> 4u;
    
    if (registers[vx] > registers[vy]) registers[0xF] = 1;
    else registers[0xF] = 0;

    registers[vx] -= registers[vy];
}

//8xy6: SHR Vx
//Sets Vx = Vx SHR 1
//Right shifts Vx (divide by 2) and saves least significant bit in VF
void Chip8::op_8xy6() {
    uint8_t vx = (opcode & 0x0F00u) >> 8u;
    registers[0xF] = (registers[vx] & 0x1u);
    registers[vx] >>= 1;
}

//8xy7: SUBN Vx, Vy
//Sets Vx = Vy - Vx and VF = NOT borrow
//if Vy > Vx, set VF = 1, else VF = 0
void Chip8::op_8xy7() {
    uint8_t vx = (opcode & 0x0F00u) >> 8u;
    uint8_t vy = (opcode & 0x00F0u) >> 4u;

    if (registers[vy] > registers[vx]) registers[0xF] = 1;
    else registers[0xF] = 0;

    registers[vx] = registers[vy] - registers[vx];
}

//8xyE: SHL Vx {, Vy}
//Sets Vx = Vx SHL 1
//Left shifts Vx (multiply by 2) and saves most significant bit in VF
void Chip8::op_8xye() {
    uint8_t vx = (opcode & 0x0F00u) >> 8u;
    registers[0xF] = (registers[vx] & 0x80u) >> 7u;
    registers[vx] <<= 1;
}

//9xy0: SNE Vx, Vy
//Skips next instruction if Vx != Vy
void Chip8::op_9xy0() {
    uint8_t vx = (opcode & 0x0F00u) >> 8u;
    uint8_t vy = (opcode & 0x00F0u) >> 4u;

    if (registers[vx] != registers[vy]) program_counter += 2;
}

//Annn: LD I, nnn
//Sets memory_index = nnn
void Chip8::op_annn() {
    uint16_t address = opcode & 0x0FFFu;
    memory_index = address;
}

//Bnnn: JP V0, nnn
//Jumps to location V0 + nnn
void Chip8::op_bnnn() {
    uint16_t address = opcode & 0x0FFFu;

    program_counter = registers[0] + address;
}

//Cxkk: RND Vx, kk
//Sets Vx = random byte AND kk
void Chip8::op_cxkk() {
    uint8_t vx = (opcode & 0x0F00) >> 8u;
    uint8_t kk = opcode & 0x00FFu;
    registers[vx] = rand_byte(r) & kk;
}

//Dxyn: DRW Vx, Vy, n
//Display n-byte sprite starting at memory index at (Vx, Vy) and set VF = collision
//Sprite is guaranteed 8 pixels wide
//Iterate through sprite row by row and column by column
//If collision, set VF = 1 and XOR pixel to flip
void Chip8::op_dxyn() {
    uint8_t vx = (opcode & 0x0F00u) >> 8u;
    uint8_t vy = (opcode & 0x00F0u) >> 4u;
    uint8_t height = opcode & 0x000Fu;

    //Wrap if beyond screen boundaries
    uint8_t xpos = registers[vx] % VIDEO_WIDTH;
    uint8_t ypos = registers[vy] % VIDEO_HEIGHT;
    registers[0xF] = 0;

    for (unsigned int row = 0; row < height; row++) {
        uint8_t sprite_byte = memory[memory_index + row];

        for (unsigned int col = 0; col < 8; col++) {
            uint8_t sprite_pixel = sprite_byte & (0x80u >> col);
            uint32_t* screen_pixel = &video[(ypos + row) * VIDEO_WIDTH + (xpos + col)];

            if (sprite_pixel) {
                if (*screen_pixel == 0xFFFFFFFF) registers[0xF] = 1;

                *screen_pixel ^= 0xFFFFFFFF;
            }
        }
    }
}

//Ex9E: SKP Vx
//Skips next instruction if key with value of Vx is pressed
void Chip8::op_ex9e() {
    uint8_t vx = (opcode & 0x0F00u) >> 8u;

    if (keypad[registers[vx]]) program_counter += 2;
}

//ExA1: SKNP Vx
//Skips next instruction if key with value of Vx is not pressed
void Chip8::op_exa1() {
    uint8_t vx = (opcode & 0x0F00u) >> 8u;

    if (!keypad[registers[vx]]) program_counter += 2;
}

//Fx07: LD Vx, DT
//Sets Vx = delay timer value
void Chip8::op_fx07() {
    uint8_t vx = (opcode & 0x0F00u) >> 8u;
    registers[vx] = delay_timer;
}

//Fx0A: LD Vx, k
//Wait for a key press and store value of key in Vx
//Waits for key press by decrementing program counter, effectively repeating the instruction
void Chip8::op_fx0a() {
    uint8_t vx = (opcode & 0x0F00u) >> 8u;

    if (keypad[0]) registers[vx] = 0;
    else if (keypad[1]) registers[vx] = 1;
    else if (keypad[2]) registers[vx] = 2;
    else if (keypad[3]) registers[vx] = 3;
    else if (keypad[4]) registers[vx] = 4;
    else if (keypad[5]) registers[vx] = 5;
    else if (keypad[6]) registers[vx] = 6;
    else if (keypad[7]) registers[vx] = 7;
    else if (keypad[8]) registers[vx] = 8;
    else if (keypad[9]) registers[vx] = 9;
    else if (keypad[10]) registers[vx] = 10;
    else if (keypad[11]) registers[vx] = 11;
    else if (keypad[12]) registers[vx] = 12;
    else if (keypad[13]) registers[vx] = 13;
    else if (keypad[14]) registers[vx] = 14;
    else if (keypad[15]) registers[vx] = 15;
    else program_counter -= 2;
}

//Fx15: LD DT, Vx
//Sets delay timer = Vx
void Chip8::op_fx15() {
    uint8_t vx = (opcode & 0x0F00u) >> 8u;
    delay_timer = registers[vx];
}

//Fx18: LD St, Vx
//Sets sound timer = Vx
void Chip8::op_fx18() {
    uint8_t vx = (opcode & 0x0F00u) >> 8u;
    sound_timer = registers[vx];
}

//Fx1E: ADD I, Vx
//Sets memory index = memory index + Vx
void Chip8::op_fx1e() {
    uint8_t  vx = (opcode & 0x0F00u) >> 8u;
    memory_index += registers[vx];
}

//Fx29: LD F, Vx
//Sets index = location of sprite for digit vx
//Fonts are located in 0x50 and are 5 bytes each, so gets address by taking an offset from fontset start address
void Chip8::op_fx29() {
    uint8_t vx = (opcode & 0x0F00u) >> 8u;
    memory_index = fontset_start_address + (5 * registers[vx]);
}

//Fx33: LD B, Vx
//Stores BCD representation of Vx in memory locations index, index+1, and index+2
//Places hundreds digit in index
//Places tens digit in index+1
//Places ones digit in index+2
//Uses modulus to get rightmost digit and divides by 10 to remove rightmost digit
void Chip8::op_fx33() {
    uint8_t vx = (opcode & 0x0F00u) >> 8u;
    uint8_t val = registers[vx];

    memory[memory_index+2] = val % 10;
    val /= 10;

    memory[memory_index+1] = val % 10;
    val /= 10;

    memory[memory_index] = val % 10;
}

//Fx55: LD [I], Vx
//Stores registers V0 through Vx in memory starting at memory index
void Chip8::op_fx55() {
    uint8_t vx = (opcode & 0x0F00u) >> 8u;
    for (uint8_t i = 0; i <= vx; i++) {
        memory[memory_index + i] = registers[i];
    }
}

//Fx65: LD Vx, [I]
//Reads registers V0 through Vx from memory starting at memory index
void Chip8::op_fx65() {
    uint8_t vx = (opcode & 0x0F00u) >> 8u;
    for (uint8_t i = 0; i <= vx; i++) {
        registers[i] = memory[memory_index + i];
    }
}