#include "update.h"
#include "common.h"
#include "entity/player.h"
#include "entity/enemy.h"

void app_update(void *appstate) {
  AppState *state = (AppState *)appstate;

  /*
   * Delta time: how many seconds elapsed since the previous frame.
   * SDL_GetTicks() returns milliseconds as a Uint64.
   * Dividing by 1000.0f converts ms → seconds.
   */
  state->last_tick    = state->current_tick;
  state->current_tick = SDL_GetTicks();
  state->delta_time   = (state->current_tick - state->last_tick) / 1000.0f;

  /* 1. Move the player (reads keyboard state internally) */
  player_update(state->delta_time);

  /*
   * 2. Update the camera to follow the player.
   *
   * We do this AFTER player_update so the camera tracks the player's new
   * position this frame, not where they were last frame (which would cause
   * a one-frame lag that's visible at high speeds).
   */
  SDL_FRect pr = player_get_rect();
  camera_follow(&state->camera, pr.x, pr.y, pr.w, pr.h);

  /*
   * 3. Update enemies — pass them everything they need to make decisions:
   *    - Where the player is (for detection radius and chase targeting)
   *    - The attack hitbox + whether it's active (for hit detection)
   */
  enemies_update(state->delta_time,
                 player_get_rect(),
                 player_get_attack_rect(),
                 player_is_attacking());
}
