#include "player.h"
#include "entity.h"
#include "../world.h"
#include <SDL3/SDL_render.h>

/* The player state is private to this file (static = not visible externally) */
static Entity player;

/*
 * Attack state.
 *
 * is_attacking  — true for a short window after pressing Space.
 * attack_timer  — counts down in seconds; attack ends when it hits 0.
 *
 * Keeping the attack as a timed window (rather than just "key is held") means
 * the player commits to a swing — it feels more intentional, like Hotline Miami.
 */
static bool  is_attacking  = false;
static float attack_timer  = 0.0f;

#define ATTACK_DURATION 0.15f  /* seconds the attack hitbox is active */

void player_init(void) {
  /*
   * Start roughly in the middle of the world so there is room to pan in all
   * directions. Position is the top-left corner of the sprite.
   */
  player.x      = WORLD_W * 0.5f - 4;
  player.y      = WORLD_H * 0.5f - 4;
  player.w      = 8;
  player.h      = 8;
  player.speed  = 90;           /* world pixels per second */
  player.facing = DIR_DOWN;
}

void player_update(float delta) {
  /*
   * SDL_GetKeyboardState returns a pointer to an internal array of SDL_bool,
   * indexed by SDL_Scancode values. Checking it every frame gives us smooth
   * held-key movement (as opposed to SDL_EVENT_KEY_DOWN which only fires once
   * per press).
   */
  const bool *keys = SDL_GetKeyboardState(NULL);

  float step = player.speed * delta;

  if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) {
    player.y     -= step;
    player.facing = DIR_UP;
  }
  if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) {
    player.y     += step;
    player.facing = DIR_DOWN;
  }
  if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT]) {
    player.x     -= step;
    player.facing = DIR_LEFT;
  }
  if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) {
    player.x     += step;
    player.facing = DIR_RIGHT;
  }

  /*
   * Clamp to world boundaries (not screen boundaries — the player should be
   * able to walk off the edge of the screen but not off the world).
   *
   * We subtract player.w / player.h so the RIGHT and BOTTOM edges of the
   * sprite are also kept inside, not just the top-left corner.
   */
  player.x = SDL_clamp(player.x, 0.0f, (float)WORLD_W - player.w);
  player.y = SDL_clamp(player.y, 0.0f, (float)WORLD_H - player.h);

  /* --- Attack input ------------------------------------------------------- */
  /*
   * Space triggers an attack. We only start a new attack if one is not already
   * active — this prevents holding Space from creating a permanent hitbox.
   */
  if (keys[SDL_SCANCODE_SPACE] && !is_attacking) {
    is_attacking = true;
    attack_timer = ATTACK_DURATION;
  }

  /* Count the attack timer down; clear is_attacking when it expires */
  if (is_attacking) {
    attack_timer -= delta;
    if (attack_timer <= 0.0f) {
      is_attacking = false;
      attack_timer = 0.0f;
    }
  }
}

/* -------------------------------------------------------------------------
 * Getters — read-only access for other modules
 * ------------------------------------------------------------------------- */

SDL_FRect player_get_rect(void) {
  return (SDL_FRect){ player.x, player.y, player.w, player.h };
}

bool player_is_attacking(void) {
  return is_attacking;
}

SDL_FRect player_get_attack_rect(void) {
  /*
   * The attack hitbox is a sprite-sized rectangle placed directly in front of
   * the player in the direction they are facing. This is what actually checks
   * whether an enemy gets hit.
   *
   * The 1-pixel gap between the player body and the hitbox is intentional —
   * it prevents the hitbox from overlapping the player's own rect, which keeps
   * collision logic clean if you add player-vs-enemy collision later.
   */
  float ax = player.x;
  float ay = player.y;

  switch (player.facing) {
    case DIR_UP:    ay -= player.h + 1; break;
    case DIR_DOWN:  ay += player.h + 1; break;
    case DIR_LEFT:  ax -= player.w + 1; break;
    case DIR_RIGHT: ax += player.w + 1; break;
  }

  return (SDL_FRect){ ax, ay, player.w, player.h };
}

/* -------------------------------------------------------------------------
 * Rendering
 * ------------------------------------------------------------------------- */

void player_render(SDL_Renderer *renderer, Camera camera) {
  /*
   * All positions are in world space. We pass each rect through camera_project
   * to get the screen-space position before handing it to SDL.
   */

  SDL_FRect world_body = { player.x, player.y, player.w, player.h };
  SDL_FRect sr         = camera_project(camera, world_body); /* screen rect */

  /* --- 1. Drop shadow ----------------------------------------------------- */
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 80);
  SDL_FRect shadow = { sr.x + 2, sr.y + 2, sr.w, sr.h };
  SDL_RenderFillRect(renderer, &shadow);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

  /* --- 2. Outline --------------------------------------------------------- */
  SDL_SetRenderDrawColor(renderer, 30, 60, 30, 255);
  SDL_FRect outline = { sr.x - 1, sr.y - 1, sr.w + 2, sr.h + 2 };
  SDL_RenderFillRect(renderer, &outline);

  /* --- 3. Body ------------------------------------------------------------ */
  /*
   * Flash brighter white-green while attacking — gives immediate visual
   * feedback that the swing is active.
   */
  if (is_attacking)
    SDL_SetRenderDrawColor(renderer, 180, 255, 180, 255);
  else
    SDL_SetRenderDrawColor(renderer, 80, 200, 100, 255);
  SDL_RenderFillRect(renderer, &sr);

  /* --- 4. Facing pip ------------------------------------------------------ */
  SDL_SetRenderDrawColor(renderer, 220, 255, 220, 255);
  float cx = sr.x + sr.w * 0.5f - 1;
  float cy = sr.y + sr.h * 0.5f - 1;
  SDL_FRect pip;
  switch (player.facing) {
    case DIR_UP:    pip = (SDL_FRect){ cx,            sr.y + 1,          2, 2 }; break;
    case DIR_DOWN:  pip = (SDL_FRect){ cx,            sr.y + sr.h - 3,   2, 2 }; break;
    case DIR_LEFT:  pip = (SDL_FRect){ sr.x + 1,      cy,                2, 2 }; break;
    case DIR_RIGHT: pip = (SDL_FRect){ sr.x + sr.w - 3, cy,             2, 2 }; break;
    default:        pip = (SDL_FRect){ cx,            cy,                2, 2 }; break;
  }
  SDL_RenderFillRect(renderer, &pip);

  /* --- 5. Attack hitbox (debug visualisation) ----------------------------- */
  /*
   * Drawing the hitbox lets you SEE exactly what you are hitting.
   * This is very useful while learning — you can verify the attack range feels
   * right and tweak ATTACK_DURATION or the rect size accordingly.
   */
  if (is_attacking) {
    SDL_FRect world_attack  = player_get_attack_rect();
    SDL_FRect screen_attack = camera_project(camera, world_attack);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 255, 255, 100, 100); /* yellow, semi-transparent */
    SDL_RenderFillRect(renderer, &screen_attack);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
  }
}
