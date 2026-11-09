#include "display.h"
#include "chip.h"

#define SCALE 10
static const SDL_Scancode keymap[NUM_KEYS] = {
    SDL_SCANCODE_X, // 0x0
    SDL_SCANCODE_1, // 0x1
    SDL_SCANCODE_2, // 0x2
    SDL_SCANCODE_3, // 0x3
    SDL_SCANCODE_Q, // 0x4
    SDL_SCANCODE_W, // 0x5
    SDL_SCANCODE_E, // 0x6
    SDL_SCANCODE_A, // 0x7
    SDL_SCANCODE_S, // 0x8
    SDL_SCANCODE_D, // 0x9
    SDL_SCANCODE_Z, // 0xA
    SDL_SCANCODE_C, // 0xB
    SDL_SCANCODE_4, // 0xC
    SDL_SCANCODE_R, // 0xD
    SDL_SCANCODE_F, // 0xE
    SDL_SCANCODE_V, // 0xF
};


bool displayInit (Display *display) {
    if (!SDL_Init(SDL_FLAG)) {
        fprintf(stderr, "Couldn't initialize the SDL: %s\n", SDL_GetError());
        return false;
    }

    display->window = SDL_CreateWindow("CHIP 8", SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE, SDL_WINDOW_RESIZABLE);
    if (!display->window)
    {
        fprintf(stderr, "Couldn't initialize the Window or renderer: %s\n", SDL_GetError());
        return false;
    }

    display->renderer = SDL_CreateRenderer(display->window, NULL);
    if (!display->renderer)
    {
        fprintf(stderr, "Couldn't initialize the Renderer: %s\n", SDL_GetError());
        return false;
    }

    display->texture = SDL_CreateTexture(display->renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!display->texture)
    {
        fprintf(stderr, "Couldn't initiate the texture: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

bool displayDraw(Display *display, Chip8 *chip8) {
    int const size = SCREEN_WIDTH * SCREEN_HEIGHT;
    uint32_t pixels[size];
    int pitch = SCREEN_WIDTH * sizeof(uint32_t);

    for (int i = 0; i < size; i++) {
        pixels[i] = chip8->video[i] ? 0xFFFFFFFF : 0x000000FF;
    }
    SDL_SetTextureScaleMode(display->texture, SDL_SCALEMODE_NEAREST);  // removes the blur from the render

    SDL_UpdateTexture(display->texture, NULL, pixels, pitch);
    SDL_RenderClear(display->renderer);
    SDL_RenderTexture(display->renderer, display->texture, NULL, NULL);
    SDL_RenderPresent(display->renderer);
}

void display_input(Chip8 *chip8, SDL_Event *event) {
    if (event->type != SDL_EVENT_KEY_DOWN && event->type != SDL_EVENT_KEY_UP) return;

    for (int i = 0; i < NUM_KEYS; i++) {
        if (event->key.scancode == keymap[i]) {
            chip8->keypad[i] = (event->type == SDL_EVENT_KEY_DOWN) ? 1 : 0;
            break;
        }
    }
}


bool displayDestroy(Display *display) {
    SDL_DestroyTexture(display->texture);
    SDL_DestroyRenderer(display->renderer);
    SDL_DestroyWindow(display->window);
    SDL_Quit();
}
