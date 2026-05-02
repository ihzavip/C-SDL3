#include "render.h"
#include "common.h"
#include "world.h"
#include "tilemap.h"
#include "weapon.h"
#include "entity/player.h"
#include "entity/enemy.h"

void app_render(void *appstate) {
  AppState *state = (AppState *)appstate;

  /* --- Clear ------------------------------------------------------------ */
  /*
   * Fill the entire frame with a dark background colour before drawing
   * anything. Without this, the previous frame bleeds through.
   */
  SDL_SetRenderDrawColor(state->renderer, 60, 80, 60, 255);
  SDL_RenderClear(state->renderer);

  /* --- Floor tiles -------------------------------------------------------- */
  tilemap_render(state->renderer, state->camera);

  /* --- World grid (debug) ------------------------------------------------ */
  /*
  SDL_SetRenderDrawColor(state->renderer, 35, 42, 35, 255);
  float grid = 32.0f;
  for (float wx = 0; wx <= WORLD_W; wx += grid) {
    float sx = wx - state->camera.x;
    SDL_RenderLine(state->renderer, sx, 0, sx, (float)LOGICAL_H);
  }
  for (float wy = 0; wy <= WORLD_H; wy += grid) {
    float sy = wy - state->camera.y;
    SDL_RenderLine(state->renderer, 0, sy, (float)LOGICAL_W, sy);
  }
  */

  /* --- Entities (back to front — painter's algorithm) ------------------- */
  /*
   * Painter's algorithm: draw things in order from back to front. Whatever
   * is drawn last appears on top. Enemies are drawn before the player so the
   * player always appears "above" enemies visually.
   */
  weapon_render(state->renderer, state->camera);
  enemies_render(state->renderer, state->camera);
  player_render(state->renderer, state->camera);
  // player_render_debug(state->renderer, state->camera); /* DEBUG: comment out to hide coords */

  /* --- HUD: health bar -------------------------------------------------- */
  /*
   * The health bar is drawn in screen space (fixed logical position).
   * No camera projection needed — it always sits in the top-left corner.
   *
   * Layout: dark red background, bright red fill scaled by hp / max_hp,
   * thin white border drawn on top.
   */
  {
    int   hp     = player_get_hp();
    int   max_hp = player_get_max_hp();
    float bx = 4, by = 4, bw = 50, bh = 5;

    SDL_FRect bg   = { bx, by, bw, bh };
    SDL_FRect fill = { bx, by, bw * ((float)hp / max_hp), bh };

    SDL_SetRenderDrawColor(state->renderer, 80, 20, 20, 255);
    SDL_RenderFillRect(state->renderer, &bg);

    SDL_SetRenderDrawColor(state->renderer, 210, 50, 50, 255);
    SDL_RenderFillRect(state->renderer, &fill);

    SDL_SetRenderDrawColor(state->renderer, 200, 200, 200, 255);
    SDL_RenderRect(state->renderer, &bg);
  }

  /* --- Death screen ----------------------------------------------------- */
  /*
   * SDL_RenderDebugText uses a built-in 8×8 px bitmap font.
   * Centering formula: x = (LOGICAL_W - char_count * 8) / 2
   *
   * The overlay is drawn last so it sits on top of the world and HUD.
   */
  if (state->game_state == GAME_DEAD) {
    /* Semi-transparent dark veil over the whole screen */
    SDL_SetRenderDrawBlendMode(state->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 160);
    SDL_FRect overlay = { 0, 0, LOGICAL_W, LOGICAL_H };
    SDL_RenderFillRect(state->renderer, &overlay);
    SDL_SetRenderDrawBlendMode(state->renderer, SDL_BLENDMODE_NONE);

    /* "YOU DIED" — 8 chars × 8px = 64px wide */
    SDL_SetRenderDrawColor(state->renderer, 210, 50, 50, 255);
    SDL_RenderDebugText(state->renderer,
                        (LOGICAL_W - 8 * 8) / 2.0f, LOGICAL_H / 2.0f - 12,
                        "YOU DIED");

    /* "Press R to restart" — 18 chars × 8px = 144px wide */
    SDL_SetRenderDrawColor(state->renderer, 200, 200, 200, 255);
    SDL_RenderDebugText(state->renderer,
                        (LOGICAL_W - 18 * 8) / 2.0f, LOGICAL_H / 2.0f + 4,
                        "Press R to restart");
  }

  /* --- Present ---------------------------------------------------------- */
  /*
   * SDL_RenderPresent swaps the back buffer to the screen. Everything above
   * was drawn into a hidden buffer; this call makes it visible all at once,
   * preventing the player from seeing a partially-drawn frame.
   */
  SDL_RenderPresent(state->renderer);
}
