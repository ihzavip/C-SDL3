#include "render.h"
#include "common.h"

void app_render(void *appstate) {
  AppState* state = (AppState*) appstate;

  SDL_RenderClear(state->renderer);
  SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 255);
  
  SDL_RenderPresent(state->renderer);
}
