#include "display.h"
#include <SDL3/SDL_main.h>
#include "chip.h"

int main(int argc, char **argv) {

    Chip8 chip8;
    Display display;

    initCHIP8(&chip8);
    loadROM(&chip8, argv[1]);
    displayInit(&display);

    bool done = false;
    while (!done) {

        while (SDL_PollEvent(&display.event)) {
            if (display.event.type == SDL_EVENT_QUIT) {
                done = true;
            }
            display_input(&chip8, &display.event);
        }

        for (int i = 0; i < 10; i++) {
            emulateCycle(&chip8);
        }

        if (chip8.delayTimer > 0) {
            // printf("%d\n", chip8.delayTimer);
            chip8.delayTimer--;
        }
        if (chip8.soundTimer > 0) {

            chip8.soundTimer--;
            // if (chip8.soundTimer == 0) printf("BEEP\n");
        }

        displayDraw(&display, &chip8);

        SDL_Delay(16); // caps the SDL at 60 fps
    }

   displayDestroy(&display);
   return 0;
}
