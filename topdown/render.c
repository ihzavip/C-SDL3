#include "render.h"
#include "common.h"
#include "entity/player.h"

void app_render(void *appstate) {
  AppState* state = (AppState*) appstate;

  SDL_SetRenderDrawColor(state->renderer, 255, 0, 0, 255);
  SDL_RenderClear(state->renderer);

  player_render(state->renderer);
 
  SDL_RenderPresent(state->renderer);
}
