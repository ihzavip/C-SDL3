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

  /* On the death screen only listen for R to restart — skip all game logic. */
  if (state->game_state == GAME_DEAD) {
    const bool *keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_R]) {
      player_reset();
      enemies_reset();
      state->game_state = GAME_PLAYING;
    }
    return;
  }

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
  bool hit = enemies_update(state->delta_time,
                            player_get_rect(),
                            player_get_attack_rect(),
                            player_is_attacking());
  if (hit) player_take_damage(1);

  /* 4. Check for player death */
  if (player_get_hp() == 0)
    state->game_state = GAME_DEAD;
}
