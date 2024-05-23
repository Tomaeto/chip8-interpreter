#pragma once

#include <cstdint>
#include <SDL2/SDL.h>

class Platform {
    public:
        Platform(const char* title, int window_width, int window_height, int texture_width, int texture_height);
        ~Platform();
        void update(const void* pixels, int pitch);
        bool processInput(uint8_t* keys);

    private:
        SDL_Window* window{};
        SDL_Renderer* renderer{};
        SDL_Texture* texture{};
};