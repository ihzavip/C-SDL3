#include "common.h"
#include "entity/enemy.h"
#include "entity/player.h"
#include "world.h"
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
  /*
   * SDL_calloc(count, size) allocates count*size bytes and zeroes them all.
   * This is safer than SDL_malloc (which leaves memory uninitialised) because
   * fields like last_tick and current_tick start at 0, giving a sane delta_time
   * on the very first frame instead of garbage.
   */
  AppState *state = SDL_calloc(1, sizeof(AppState));
  if (!state) {
    SDL_Log("Out of memory");
    return SDL_APP_FAILURE;
  }
  *appstate = state;

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Error initialising SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  state->window = SDL_CreateWindow("Topdown Ihza", LOGICAL_W, LOGICAL_H, SDL_WINDOW_RESIZABLE);
  if (!state->window) {
    SDL_Log("Error creating window: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  state->renderer = SDL_CreateRenderer(state->window, NULL);
  if (!state->renderer) {
    SDL_Log("Error creating renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  /*
   * SDL_SetRenderLogicalPresentation tells SDL to treat the renderer as if it

   * were exactly 320×180 pixels, regardless of the actual window size.
   * SDL scales and letterboxes automatically.
   *
   * This means all our game coordinates are in "logical pixels" (320×180),
   * not real pixels — the same code works at any window resolution.
   *
   * These numbers MUST match LOGICAL_W / LOGICAL_H in world.h.
   */
  SDL_SetRenderLogicalPresentation(state->renderer, LOGICAL_W, LOGICAL_H,
                                   SDL_LOGICAL_PRESENTATION_LETTERBOX);

  /* Initialise camera at world origin — camera_follow() will correct it
     on the first update frame. */
  state->camera.x = 0;
  state->camera.y = 0;

  player_init(state->renderer);
  enemies_load_textures(state->renderer);
  enemies_init();

  return SDL_APP_CONTINUE;
}
