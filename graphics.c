#include <SDL2/SDL.h>
#include "graphics.h"
#include "chip.h"


int display_init(display_t* display ,unsigned short width, unsigned short height){
    SDL_Init(SDL_INIT_VIDEO);

    display->window = SDL_CreateWindow("chip-8", 100, 100, width * PIXEL_SIZE, height * PIXEL_SIZE, SDL_WINDOW_SHOWN);
    if(!display->window){
        return -1;
    }

    display->renderer = SDL_CreateRenderer(display->window, -1, SDL_RENDERER_ACCELERATED);
    if(!display->renderer){
        return -1;
    }

    display->texture = SDL_CreateTexture(display->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, width, height);
    if(!display->texture){
        return -1;
    }

    return 0;
}


void display_draw(display_t* display,uint32_t video[SCREEN_WIDTH * SCREEN_HEIGHT]){
  SDL_UpdateTexture(display->texture, NULL, video, SCREEN_WIDTH * sizeof(uint32_t));
  SDL_RenderCopy(display->renderer, display->texture, NULL, NULL);
  SDL_RenderPresent(display->renderer);
}


void free_display(display_t* display) {
  SDL_DestroyTexture(display->texture);
  SDL_DestroyRenderer(display->renderer);
  SDL_DestroyWindow(display->window);
  SDL_Quit();
}
