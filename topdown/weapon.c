#include "weapon.h"
#include "camera.h"
#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_render.h>

static SDL_Texture *tex        = NULL;
static float        spawn_x    = 0, spawn_y = 0;
static float        wx         = 0, wy      = 0;
static float        item_w     = 0, item_h  = 0;
static bool         picked_up  = false;
static bool         just_picked = false;

void weapon_init(SDL_Renderer *renderer, float x, float y) {
  tex = IMG_LoadTexture(renderer, "media/Objects/Pickable/Bat.png");
  if (!tex) {
    SDL_Log("Failed to load weapon texture: %s", SDL_GetError());
    return;
  }
  SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
  SDL_GetTextureSize(tex, &item_w, &item_h);
  spawn_x = wx = x;
  spawn_y = wy = y;
  picked_up   = false;
  just_picked = false;
}

void weapon_update(SDL_FRect player_rect) {
  just_picked = false;
  if (picked_up) return;

  const bool *keys = SDL_GetKeyboardState(NULL);
  if (!keys[SDL_SCANCODE_E]) return;

  SDL_FRect item_rect = { wx, wy, item_w, item_h };
  if (SDL_HasRectIntersectionFloat(&player_rect, &item_rect)) {
    picked_up   = true;
    just_picked = true;
  }
}

void weapon_render(SDL_Renderer *renderer, Camera camera) {
  if (picked_up || !tex) return;
  SDL_FRect dst = camera_project(camera, (SDL_FRect){ wx, wy, item_w, item_h });
  SDL_RenderTexture(renderer, tex, NULL, &dst);
}

void weapon_reset(void) {
  wx = spawn_x;
  wy = spawn_y;
  picked_up   = false;
  just_picked = false;
}

void weapon_destroy(void) {
  if (tex) { SDL_DestroyTexture(tex); tex = NULL; }
}

bool weapon_just_picked_up(void) { return just_picked; }
