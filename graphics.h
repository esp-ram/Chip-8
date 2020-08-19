#include <SDL2/SDL.h>


typedef struct display {
  SDL_Window* window;
  SDL_Renderer* renderer;
  SDL_Texture* texture;
} display_t;


int display_init(display_t* display ,unsigned short width, unsigned short height);
void free_display(display_t* display);
void display_draw(display_t* display,uint32_t video[64*32]);
