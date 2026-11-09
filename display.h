#ifndef DISPLAY_H
#define DISPLAY_H


#include "chip.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdbool.h>


#define SDL_FLAG (SDL_INIT_VIDEO | SDL_INIT_AUDIO)


typedef struct Display {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Event event;
} Display;


bool displayInit(Display *display);
bool displayDraw(Display *display, Chip8 *chip8);
void display_input (Chip8 *chip8, SDL_Event *event );
bool displayDestroy(Display *display);

#endif
