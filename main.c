#include "chip.h"
#include <stdio.h>
#include "graphics.h"
#include <unistd.h>


int main(int argc, char const *argv[]) {
    chip8 newChip;
    display_t display;

    if(display_init(&display,SCREEN_WIDTH,SCREEN_HEIGHT) == -1){
        fprintf(stderr, "failed to initiate screen\n");
        return -1;
    }



    if (chip_init(&newChip,argv[1]) == -1){
        fprintf(stderr, "failed to initiate chip\n");
        free_display(&display);
        return -1;
    }


    SDL_Event event;
    uint32_t start_tick;
    uint32_t frame_speed;
    bool running = true;


    while (running) {
      start_tick = SDL_GetTicks();
      emulateCycle(&newChip);

      if (newChip.drawFlag) {
        display_draw(&display, newChip.video);
        newChip.drawFlag = false;
      }

      frame_speed = SDL_GetTicks() - start_tick;
      if (frame_speed < MILLISECONDS_PER_CYCLE) {
        SDL_Delay(MILLISECONDS_PER_CYCLE - frame_speed);
      }

      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
          running = false;
        }
      }
    }
    free_display(&display);
    return 0;
}
