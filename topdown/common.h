// to share state
#pragma once
#include <SDL3/SDL.h>
#include "camera.h"

typedef enum {
  GAME_PLAYING,
  GAME_DEAD,
} GameState;

typedef struct AppState {
  SDL_Window *window;
  SDL_Renderer *renderer;

  /*
   * SDL_GetTicks() returns milliseconds since SDL was initialised as a Uint64
   * (an unsigned 64-bit integer). We store two snapshots — the tick at the
   * start of the previous frame and the current frame — so we can compute how
   * much time passed between them.
   *
   * Why NOT float? A float has ~7 significant decimal digits. After ~49 days
   * of runtime SDL_GetTicks() grows large enough that a float can no longer
   * represent individual milliseconds accurately. Uint64 has no such problem.
   */
  Uint64 last_tick;
  Uint64 current_tick;

  /*
   * delta_time is the time between the last two frames, expressed in *seconds*
   * (not milliseconds). Multiplying a speed value (units/second) by delta_time
   * gives the distance to move this frame, which keeps movement frame-rate
   * independent — the player travels at the same real-world speed whether the
   * game runs at 30 fps or 144 fps.
   */
  float delta_time;

  /*
   * The camera tracks which part of the world is currently visible.
   * It is updated every frame in update.c to follow the player, then
   * passed to render functions so they can project world→screen coords.
   */
  Camera    camera;
  GameState game_state;
} AppState;
