#include "player.h"
#include "entity.h"
#include "../world.h"
#include "../camera.h"
#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_render.h>
#include <stdio.h>

/* -------------------------------------------------------------------------
 * Animation system
 *
 * The character has three animations (idle, run, punch), each with frames
 * stored in a horizontal spritesheet — all frames side by side in one PNG.
 *
 * To draw frame N, we take a source rectangle starting at x = N * frame_width.
 * SDL_RenderTexture lets us specify exactly which part of the texture to draw.
 * ------------------------------------------------------------------------- */

typedef enum {
  ANIM_IDLE   = 0,
  ANIM_RUN    = 1,
  ANIM_PUNCH  = 2,
  ANIM_PICKUP = 3,
  ANIM_COUNT  = 4,
} AnimType;

/* Number of frames in each animation's spritesheet */
static const int frame_counts[ANIM_COUNT] = { 6, 6, 4, 3 };

/* How long (seconds) each frame is shown before advancing to the next */
static const float frame_durations[ANIM_COUNT] = {
  0.18f,  /* idle:  slow, relaxed */
  0.10f,  /* run:   one step every ~100ms */
  0.04f,  /* punch: very snappy */
  0.08f,  /* pickup: 3 frames x 80ms = 240ms total */
};

/*
 * Frame dimensions per (animation, direction) — each sheet can have a
 * different frame size. Populated at load time from the actual texture
 * dimensions so we never hardcode the wrong value.
 * Indexed as frame_w[AnimType][Direction].
 */
static float frame_w[ANIM_COUNT][DIR_COUNT];
static float frame_h[ANIM_COUNT][DIR_COUNT];

/* How far the body's left edge sits from the frame's left edge, per (anim, dir).
   Non-zero when the frame is wider than the body (e.g. punch-left fist extends left). */
static float anchor_x[ANIM_COUNT][DIR_COUNT];

/* No-hands body sprites — drawn as base layer when bat is equipped */
static SDL_Texture *nohands_textures[ANIM_COUNT][DIR_COUNT];
static float nohands_frame_w[ANIM_COUNT][DIR_COUNT];
static float nohands_frame_h[ANIM_COUNT][DIR_COUNT];
static float nohands_anchor_x[ANIM_COUNT][DIR_COUNT];

/* Hands overlay sprites — drawn on top of no-hands body when unarmed */
static SDL_Texture *hands_textures[ANIM_COUNT][DIR_COUNT];
static float hands_frame_w[ANIM_COUNT][DIR_COUNT];
static float hands_frame_h[ANIM_COUNT][DIR_COUNT];

/* Bat weapon overlay sprites — drawn on top of no-hands body when equipped */
static SDL_Texture *bat_textures[ANIM_COUNT][DIR_COUNT];
static float bat_frame_w[ANIM_COUNT][DIR_COUNT];
static float bat_frame_h[ANIM_COUNT][DIR_COUNT];
static float bat_anchor_x[ANIM_COUNT][DIR_COUNT];
/* y-offset from player.y: places bat arm at arm level, not head level */
static float bat_y_offset[ANIM_COUNT][DIR_COUNT];

static bool bat_equipped  = false;
static bool is_picking_up = false;

/*
 * Texture table: one texture per (animation, direction) pair.
 * Indexed as textures[AnimType][Direction].
 * Direction order: DIR_DOWN=0, DIR_UP=1, DIR_LEFT=2, DIR_RIGHT=3 (from entity.h)
 */
static SDL_Texture *textures[ANIM_COUNT][DIR_COUNT];

/* Animation playback state */
static AnimType anim_state = ANIM_IDLE;
static int      anim_frame = 0;
static float    anim_timer = 0.0f;

/* -------------------------------------------------------------------------
 * Player entity and attack state
 * ------------------------------------------------------------------------- */
static Entity player;

static bool  is_attacking = false;
static float attack_timer = 0.0f;

#define ATTACK_DURATION      0.20f
#define HIT_COOLDOWN_DURATION 1.0f  /* seconds of invincibility after being hit */
#define SPRITE_HEIGHT_PADDING 2     /* extra px read from sheet to avoid foot clipping */

static int   hp           = 5;
static int   max_hp       = 5;
static float hit_cooldown = 0.0f;

/* -------------------------------------------------------------------------
 * Texture loading helper
 *
 * IMG_LoadTexture (from SDL3_image) does everything in one call:
 *   1. Opens the file
 *   2. Decodes the PNG (including the alpha/transparency channel)
 *   3. Uploads the pixels to the GPU as an SDL_Texture
 *   4. Returns the texture (or NULL on error)
 *
 * We also call SDL_SetTextureBlendMode(SDL_BLENDMODE_BLEND) so the
 * transparent pixels in the PNG are actually transparent when drawn,
 * rather than being rendered as solid black.
 * ------------------------------------------------------------------------- */
static SDL_Texture *load_sheet(SDL_Renderer *renderer, const char *path) {
  SDL_Texture *tex = IMG_LoadTexture(renderer, path);
  if (!tex) {
    SDL_Log("Failed to load texture '%s': %s", path, SDL_GetError());
    return NULL;
  }
  /*
   * Blend mode must be set to BLEND for alpha (transparency) to work.
   * Without this, transparent pixels in the PNG are drawn as black.
   */
  SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
  return tex;
}

void player_init(SDL_Renderer *renderer) {

  // Why not player width / 2? Float
  player.x      = WORLD_W * 0.5f - 6; 
  player.y      = WORLD_H * 0.5f - 8;
  player.w      = 13;  /* match idle frame width  */
  player.h      = 16;  /* match idle frame height */
  player.speed  = 60;
  player.facing = DIR_DOWN;

  /*
   * Build the texture path from parts and load each spritesheet.
   *
   * Path format:
   *   media/Character/Main/<AnimDir>/Character_<dirName>_<animName>-<sheet>.png
   *
   * The game must be run from the project root directory so these relative
   * paths resolve correctly. (e.g. ./build/Debug/topdown from project root)
   *
   * snprintf writes a formatted string into a fixed-size buffer — safer than
   * sprintf because it cannot overflow the buffer.
   */

  /* Animation folder names, lowercase anim names, sheet suffixes */
  const char *anim_dirs[ANIM_COUNT]  = { "Idle",  "Run",  "Punch",   "Pick-up" };
  const char *anim_names[ANIM_COUNT] = { "idle",  "run",  "punch",   "Pick-up" };
  const char *anim_sfx[ANIM_COUNT]   = { "Sheet6","Sheet6","Sheet4", "Sheet3"  };

  /*
   * Direction name as it appears in the filename.
   * Indexed by Direction enum: DIR_DOWN=0, DIR_UP=1, DIR_LEFT=2, DIR_RIGHT=3
   */
  const char *dir_names[DIR_COUNT] = { "down", "up", "side-left", "side" };

  char path[256];

  /* Base animation textures*/
  for (int a = 0; a < ANIM_COUNT; a++) {
    for (int d = 0; d < DIR_COUNT; d++) {
      snprintf(path, sizeof(path),
        "media/Character/Main/%s/Character_%s_%s-%s.png",
        anim_dirs[a], dir_names[d], anim_names[a], anim_sfx[a]);

      textures[a][d] = load_sheet(renderer, path);

      /*
       * Read the actual texture dimensions and compute the frame size
       * for this specific (animation, direction) pair.
       * Each direction's sheet can have a different width — e.g. the idle
       * "up" sheet is 66px wide (11px/frame) while "down" is 78px (13px/frame).
       */
      if (textures[a][d]) {
        float target_width, target_height;
        SDL_GetTextureSize(textures[a][d], &target_width, &target_height);
        frame_w[a][d] = target_width / frame_counts[a];
        frame_h[a][d] = target_height;
      }
    }
  }

  /* Compute anchor offsets now that all frame widths are known.
     For left-facing animations, extra width is on the left (fist/weapon side),
     so the body is shifted right by that amount. */
  for (int a = 0; a < ANIM_COUNT; a++) {
    float extra = frame_w[a][DIR_LEFT] - frame_w[ANIM_IDLE][DIR_LEFT];
    anchor_x[a][DIR_LEFT] = extra > 0 ? extra : 0;
  }

  /* Load bat character sprites — used after the bat is picked up.
     ANIM_IDLE and ANIM_RUN share the same idle-and-run sheet (loaded separately).
     ANIM_PICKUP is not used for the bat set — bat_textures[ANIM_PICKUP] stays NULL. */
  // const char *bat_sheet[ANIM_COUNT] = {
  //   "idle-and-run-Sheet6",  /* ANIM_IDLE  */
  //   "idle-and-run-Sheet6",  /* ANIM_RUN   */
  //   "attack-Sheet4",         /* ANIM_PUNCH */
  //   NULL,                    /* ANIM_PICKUP — not used for bat */
  // };

  // for (int a = 0; a < ANIM_COUNT; a++) {
  //   if (!bat_sheet[a]) continue;
  //   for (int d = 0; d < DIR_COUNT; d++) {
  //     snprintf(path, sizeof(path),
  //       "media/Character/Bat/Bat_%s_%s.png",
  //       dir_names[d], bat_sheet[a]);
  //     bat_textures[a][d] = load_sheet(renderer, path);
  //     if (bat_textures[a][d]) {
  //       float w, h;
  //       SDL_GetTextureSize(bat_textures[a][d], &w, &h);
  //       bat_frame_w[a][d] = w / frame_counts[a];
  //       bat_frame_h[a][d] = h;
  //     }
  //   }
  // }
  /*
   * bat_anchor_x for DIR_LEFT: the bat arm sits at the RIGHT side of the
   * left-facing bat frame.  We shift the render position left so the arm
   * portion lines up with the character body.
   *
   * Formula: bat_fw - nohands_fw_idle  (nohands idle = narrowest body frame,
   * gives the amount of "bat-head overhang" to the left).
   */
  // for (int a = 0; a < ANIM_COUNT; a++) {
  //   if (!bat_sheet[a]) continue;
  //   bat_anchor_x[a][DIR_LEFT] =
  //     bat_frame_w[a][DIR_LEFT] - nohands_frame_w[ANIM_IDLE][DIR_LEFT];
  // }

  /* Load no-hands body sprites — base layer drawn beneath the bat weapon overlay 
     sfx means suffix
  */
  const char *nohands_sfx[ANIM_COUNT] = {
    "idle_no-hands-Sheet6",   /* ANIM_IDLE  */
    "run_no-hands-Sheet6",    /* ANIM_RUN   */
    "punch_no-hands-Sheet4",  /* ANIM_PUNCH */
    NULL,                      /* ANIM_PICKUP — uses regular Pick-up sprite */
  };

  for (int a = 0; a < ANIM_COUNT; a++) {
    if (!nohands_sfx[a]) continue;
    for (int d = 0; d < DIR_COUNT; d++) {
      snprintf(path, sizeof(path),
        "media/Character/Main/%s/Character_%s_%s.png",
        anim_dirs[a], dir_names[d], nohands_sfx[a]);
      nohands_textures[a][d] = load_sheet(renderer, path);
      if (nohands_textures[a][d]) {
        float w, h;
        SDL_GetTextureSize(nohands_textures[a][d], &w, &h);
        nohands_frame_w[a][d] = w / frame_counts[a];
        nohands_frame_h[a][d] = h;
      }
    }
  }
  for (int a = 0; a < ANIM_COUNT; a++) {
    if (!nohands_sfx[a]) continue;
    float extra = nohands_frame_w[a][DIR_LEFT] - nohands_frame_w[ANIM_IDLE][DIR_LEFT];
    nohands_anchor_x[a][DIR_LEFT] = extra > 0 ? extra : 0;
  }

  /*
   * bat_y_offset: the bat arm sprites are shorter than the full body frame.
   * Rendering them at player.y puts the arm at head level.  Instead, sink
   * them to the bottom of the body frame so they sit at arm level.
   * y_offset = body_frame_h - bat_frame_h (differs per direction).
   */
  // for (int a = 0; a < ANIM_COUNT; a++) {
  //   if (!bat_sheet[a]) continue;
  //   for (int d = 0; d < DIR_COUNT; d++) {
  //     if (nohands_textures[a][d] && bat_textures[a][d])
  //       bat_y_offset[a][d] = nohands_frame_h[a][d] - bat_frame_h[a][d];
  //   }
  // }

  /* Hands overlay — unarmed idle (filenames use inconsistent capitalisation) */
  const char *hands_idle_paths[DIR_COUNT] = {
    "media/Character/Main/Idle/Hands_down_idle-Sheet6.png",      /* DIR_DOWN  */
    "media/Character/Main/Idle/Hands_Up_idle-Sheet6.png",        /* DIR_UP    */
    "media/Character/Main/Idle/Hands_Side-left_idle-Sheet6.png", /* DIR_LEFT  */
    "media/Character/Main/Idle/Hands_Side_idle-Sheet6.png",      /* DIR_RIGHT */
  };
  for (int d = 0; d < DIR_COUNT; d++) {
    hands_textures[ANIM_IDLE][d] = load_sheet(renderer, hands_idle_paths[d]);
    if (hands_textures[ANIM_IDLE][d]) {
      float w, h;
      SDL_GetTextureSize(hands_textures[ANIM_IDLE][d], &w, &h);
      hands_frame_w[ANIM_IDLE][d] = w / frame_counts[ANIM_IDLE];
      hands_frame_h[ANIM_IDLE][d] = h;
    }
  }
}

void player_update(float delta) {
  const bool *keys = SDL_GetKeyboardState(NULL);

  if (hit_cooldown > 0.0f) hit_cooldown -= delta;

  /* Pickup animation blocks all other input until it finishes */
  if (is_picking_up) {
    anim_timer += delta;
    if (anim_timer >= frame_durations[ANIM_PICKUP]) {
      anim_timer -= frame_durations[ANIM_PICKUP];
      if (anim_frame < frame_counts[ANIM_PICKUP] - 1) {
        anim_frame++;
      } else {
        bat_equipped  = true;
        is_picking_up = false;
        anim_state    = ANIM_IDLE;
        anim_frame    = 0;
        anim_timer    = 0.0f;
      }
    }
    return;
  }

  float step      = player.speed * delta;
  bool  is_moving = false;

  if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) {
    player.y     -= step;
    player.facing = DIR_UP;
    is_moving     = true;
  }
  if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) {
    player.y     += step;
    player.facing = DIR_DOWN;
    is_moving     = true;
  }
  if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT]) {
    player.x     -= step;
    player.facing = DIR_LEFT;
    is_moving     = true;
  }
  if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) {
    player.x     += step;
    player.facing = DIR_RIGHT;
    is_moving     = true;
  }

  player.x = SDL_clamp(player.x, 0.0f, (float)WORLD_W - player.w);
  player.y = SDL_clamp(player.y, 0.0f, (float)WORLD_H - player.h);

  /* Attack input */
  if (keys[SDL_SCANCODE_SPACE] && !is_attacking) {
    is_attacking = true;
    attack_timer = ATTACK_DURATION;
  }
  if (is_attacking) {
    attack_timer -= delta;
    if (attack_timer <= 0.0f) {
      is_attacking = false;
      attack_timer = 0.0f;
    }
  }

  /* --- Determine which animation should play --- */
  AnimType new_anim;
  if (is_attacking)   new_anim = ANIM_PUNCH;
  else if (is_moving) new_anim = ANIM_RUN;
  else                new_anim = ANIM_IDLE;

  if (new_anim != anim_state) {
    anim_state = new_anim;
    anim_frame = 0;
    anim_timer = 0.0f;
  }

  /* --- Advance animation frame --- */
  anim_timer += delta;
  if (anim_timer >= frame_durations[anim_state]) {
    anim_timer -= frame_durations[anim_state];

    /* Punch plays once and holds the last frame; idle and run loop. */
    if (anim_state == ANIM_PUNCH) {
      if (anim_frame < frame_counts[ANIM_PUNCH] - 1)
        anim_frame++;
    } else {
      anim_frame = (anim_frame + 1) % frame_counts[anim_state];
    }
  }
}

void player_render(SDL_Renderer *renderer, Camera camera) {
  int dir = (int)player.facing;
  bool blink = hit_cooldown > 0.0f && ((int)(hit_cooldown / 0.1f) % 2) == 0;

  if (false) { /* bat_equipped — disabled for debugging */
  } else {
    SDL_Texture *tex   = nohands_textures[anim_state][dir];
    float frame_width  = nohands_frame_w[anim_state][dir];
    float frame_height = nohands_frame_h[anim_state][dir];
    float ax           = nohands_anchor_x[anim_state][dir];
    SDL_FRect src      = { anim_frame * frame_width, 0, frame_width, frame_height + SPRITE_HEIGHT_PADDING };
    SDL_FRect world_rect = { player.x - ax, player.y,
                             frame_width, frame_height + SPRITE_HEIGHT_PADDING };
    SDL_FRect dst = camera_project(camera, world_rect);
    if (tex) {
      if (blink) SDL_SetTextureColorMod(tex, 255, 80, 80);
      SDL_RenderTexture(renderer, tex, &src, &dst);
      SDL_SetTextureColorMod(tex, 255, 255, 255);
    } else {
      SDL_SetRenderDrawColor(renderer, 80, 200, 100, 255);
      SDL_RenderFillRect(renderer, &dst);
    }

    /* Hands overlay on top of no-hands body */
    SDL_Texture *hands_tex = hands_textures[anim_state][dir];
    if (hands_tex) {
      float hfw = hands_frame_w[anim_state][dir];
      float hfh = hands_frame_h[anim_state][dir];
      SDL_FRect h_src   = { anim_frame * hfw, 0, hfw, hfh };
      SDL_FRect h_world = { player.x - ax, player.y, hfw, hfh };
      SDL_FRect h_dst   = camera_project(camera, h_world);
      if (blink) SDL_SetTextureColorMod(hands_tex, 255, 80, 80);
      SDL_RenderTexture(renderer, hands_tex, &h_src, &h_dst);
      SDL_SetTextureColorMod(hands_tex, 255, 255, 255);
    }
  }

  /* DEBUG boxes — comment out either line to hide it
   * Green  = sprite frame  (frame_width × frame_height)
   * Red    = physics box   (player.w × player.h) */
  // SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
  // SDL_RenderRect(renderer, &dst);
  // SDL_FRect physics_screen = camera_project(camera, (SDL_FRect){player.x, player.y, player.w, player.h});
  // SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  // SDL_RenderRect(renderer, &physics_screen);

  /* Attack hitbox — semi-transparent yellow overlay, useful for learning */
  if (is_attacking) {
    SDL_FRect world_attack  = player_get_attack_rect();
    SDL_FRect screen_attack = camera_project(camera, world_attack);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 255, 255, 80, 80);
    SDL_RenderFillRect(renderer, &screen_attack);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
  }
}

void player_render_debug(SDL_Renderer *renderer, Camera camera) {
  char buf[32];
  SDL_snprintf(buf, sizeof(buf), "x:%.0f y:%.0f", player.x, player.y);

  /*
   * Position the label 2px below the bottom edge of the sprite in screen space.
   * SDL_RenderDebugText uses an 8px tall bitmap font.
   */
  SDL_FRect world_rect = {player.x, player.y, player.w, player.h};
  SDL_FRect dst = camera_project(camera, world_rect);

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderDebugText(renderer, dst.x, dst.y + dst.h + 2, buf);
}

void player_reset(void) {
  player.x      = WORLD_W * 0.5f - 6;
  player.y      = WORLD_H * 0.5f - 8;
  player.facing = DIR_DOWN;
  hp            = max_hp;
  hit_cooldown  = 0.0f;
  is_attacking  = false;
  attack_timer  = 0.0f;
  anim_state    = ANIM_IDLE;
  anim_frame    = 0;
  anim_timer    = 0.0f;
  bat_equipped  = false;
  is_picking_up = false;
}

void player_destroy(void) {
  /*
   * SDL_DestroyTexture frees the GPU memory used by the texture.
   * Always destroy textures you no longer need — SDL does not do it for you.
   */
  for (int a = 0; a < ANIM_COUNT; a++)
    for (int d = 0; d < DIR_COUNT; d++)
      if (textures[a][d]) SDL_DestroyTexture(textures[a][d]);
  for (int a = 0; a < ANIM_COUNT; a++)
    for (int d = 0; d < DIR_COUNT; d++)
      if (bat_textures[a][d]) SDL_DestroyTexture(bat_textures[a][d]);
  for (int a = 0; a < ANIM_COUNT; a++)
    for (int d = 0; d < DIR_COUNT; d++)
      if (nohands_textures[a][d]) SDL_DestroyTexture(nohands_textures[a][d]);
  for (int a = 0; a < ANIM_COUNT; a++)
    for (int d = 0; d < DIR_COUNT; d++)
      if (hands_textures[a][d]) SDL_DestroyTexture(hands_textures[a][d]);
}

/* -------------------------------------------------------------------------
 * Getters
 * ------------------------------------------------------------------------- */

SDL_FRect player_get_rect(void) {
  return (SDL_FRect){ player.x, player.y, player.w, player.h };
}

bool      player_is_attacking(void) { return is_attacking; }
Direction player_get_facing(void)   { return player.facing; }
int       player_get_hp(void)       { return hp; }
int       player_get_max_hp(void)   { return max_hp; }

void player_take_damage(int amount) {
  if (hit_cooldown > 0.0f) return; /* still invincible from last hit */
  hp -= amount;
  if (hp < 0) hp = 0;
  hit_cooldown = HIT_COOLDOWN_DURATION;
}

void player_equip_bat(void) {
  is_picking_up = true;
  anim_state    = ANIM_PICKUP;
  anim_frame    = 0;
  anim_timer    = 0.0f;
}

SDL_FRect player_get_attack_rect(void) {
  float ax = player.x, ay = player.y;
  if (bat_equipped) {
    float reach = 20.0f;
    switch (player.facing) {
      case DIR_UP:    ay -= reach; break;
      case DIR_DOWN:  ay += reach; break;
      case DIR_LEFT:  ax -= reach; break;
      case DIR_RIGHT: ax += reach; break;
      case DIR_COUNT: break;
    }
  } else {
    switch (player.facing) {
      case DIR_UP:    ay -= player.h; break;
      case DIR_DOWN:  ay += player.h; break;
      case DIR_LEFT:  ax -= player.w; break;
      case DIR_RIGHT: ax += player.w; break;
      case DIR_COUNT: break;
    }
  }
  return (SDL_FRect){ ax, ay, player.w, player.h };
}
