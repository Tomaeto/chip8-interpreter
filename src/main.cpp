#include "chip8.hpp"
#include "platform.hpp"
#include <iostream>

int main(int argc, char** argv) {
    //Takes arguments for video scale, cycle delay, and ROM to load
    if (argc != 4) {
        std::cerr << "Invalid arguments. Correct usage is " << argv[0] << " <scale> <delay> <ROM>\n";
        std::exit(EXIT_FAILURE);
    }

    int video_scale = std::stoi(argv[1]);
    int cycle_delay = std::stoi(argv[2]);
    const char* rom_filename = argv[3];

    Platform platform("CHIP-8 Emulator", VIDEO_WIDTH * video_scale, VIDEO_HEIGHT * video_scale, VIDEO_WIDTH, VIDEO_HEIGHT);

    Chip8 chip8;
    chip8.loadRom(rom_filename);

    int video_pitch = sizeof(chip8.video[0]) * VIDEO_WIDTH;
    auto last_cycletime = std::chrono::high_resolution_clock::now();

    bool quit = false;
    while (!quit) {
        quit = platform.processInput(chip8.keypad);

        auto current_time = std::chrono::high_resolution_clock::now();
        
        //Getting difference between last cycle time and current time (length of time since last cycle)
		float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(current_time - last_cycletime).count();

        //If time since last cycle is greater than the cycle delay, run cycle and update video
        if (dt > cycle_delay) {
            last_cycletime = current_time;
            chip8.cycle();
            platform.update(chip8.video, video_pitch);
        }
    }
    return 0;
}