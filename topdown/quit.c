#include "common.h"
#include "entity/player.h"
#include "entity/enemy.h"
#include <stdio.h>

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  AppState* state = (AppState*) appstate;

  player_destroy();          /* free player textures before destroying renderer */
  enemies_destroy_textures(); /* free enemy textures before destroying renderer */

  SDL_DestroyRenderer(state->renderer);
  state->renderer = NULL;
  SDL_DestroyWindow(state->window);
  state->window = NULL;
  SDL_QuitSubSystem(SDL_INIT_VIDEO);

  printf("%s\n", "Cleanup");
}
