# CHIP8 Interpreter
CHIP-8 Emulator written in C++

Assets folder contains test ROMS for testing program, more can be found [here](https://github.com/dmatlack/chip8/tree/master/roms/games).
# To Compile/Run

Requires [MinGW](https://www.mingw-w64.org/) with [SDL2](https://www.libsdl.org/).

Run `c++ ./src/main.cpp ./src/chip8.cpp ./src/platform.cpp -lmingw32 -lSDL2main -lSDL2 -o main.exe` to compile a main executable.

Running executable requires arguments in the format `main.exe <scale> <delay> <ROM>`. 

After compilation, run `./main.exe 10 2 ./assets/test_opcode.ch8` to run test ROM that validates registers.

# Key Mapping
The keypad mapping is as follows:


    +-+-+-+-+    +-+-+-+-+
    
    |1|2|3|C|    |1|2|3|4|
    
    +-+-+-+-+    +-+-+-+-+
    
    |4|5|6|D|    |Q|W|E|R|
    
    +-+-+-+-+    +-+-+-+-+
    
    |7|8|9|E|    |A|S|D|F|
    
    +-+-+-+-+    +-+-+-+-+
    
    |A|0|B|F|    |Z|X|C|V|
    
    +-+-+-+-+    +-+-+-+-+
