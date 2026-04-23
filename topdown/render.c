#include "render.h"
#include "common.h"
#include "world.h"
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

  /* --- World grid ------------------------------------------------------- */
  /*
   * Drawing a tile grid makes the camera pan clearly visible — you can see
   * the world scrolling as you move. Without it, a solid-colour background
   * makes it hard to tell whether the camera is actually moving.
   *
   * The grid lines exist in WORLD space. We convert each endpoint through the
   * camera to get its SCREEN position before drawing.
   *
   * SDL_RenderLine(renderer, x1, y1, x2, y2) draws one straight line.
   */
  SDL_SetRenderDrawColor(state->renderer, 35, 42, 35, 255); /* slightly lighter */

  float grid = 32.0f; /* world pixels between grid lines */

  /* Vertical lines — iterate X from 0 to WORLD_W in steps of `grid` */
  for (float wx = 0; wx <= WORLD_W; wx += grid) {
    float sx = wx - state->camera.x; /* project X: subtract camera offset */
    SDL_RenderLine(state->renderer, sx, 0, sx, (float)LOGICAL_H);
  }

  /* Horizontal lines — iterate Y from 0 to WORLD_H in steps of `grid` */
  for (float wy = 0; wy <= WORLD_H; wy += grid) {
    float sy = wy - state->camera.y; /* project Y: subtract camera offset */
    SDL_RenderLine(state->renderer, 0, sy, (float)LOGICAL_W, sy);
  }

  /* --- Entities (back to front — painter's algorithm) ------------------- */
  /*
   * Painter's algorithm: draw things in order from back to front. Whatever
   * is drawn last appears on top. Enemies are drawn before the player so the
   * player always appears "above" enemies visually.
   */
  enemies_render(state->renderer, state->camera);
  player_render(state->renderer, state->camera);
  // player_render_debug(state->renderer, state->camera); /* DEBUG: comment out to hide coords */

  /* --- Present ---------------------------------------------------------- */
  /*
   * SDL_RenderPresent swaps the back buffer to the screen. Everything above
   * was drawn into a hidden buffer; this call makes it visible all at once,
   * preventing the player from seeing a partially-drawn frame.
   */
  SDL_RenderPresent(state->renderer);
}
