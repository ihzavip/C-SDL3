#include "update.h"
#include <stdio.h>
#include "common.h"
#include "entity/player.h"

void app_update(void *appstate) {
  AppState *state = (AppState *)appstate;

  state->last_tick = state->current_tick;
  state->current_tick = SDL_GetTicks();
  state->delta_time = (state->current_tick - state->last_tick) / 1000.0f;

  player_update(state->delta_time);

  printf("last_tick: %f\n", state->last_tick);
  printf("current_tick: %f\n", state->current_tick);
  printf("delta_time: %f\n", state->delta_time);
}
