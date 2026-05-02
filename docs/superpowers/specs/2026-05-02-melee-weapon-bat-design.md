# Melee Weapon — Baseball Bat Design

**Goal:** Add a baseball bat pickup to the topdown game. The bat spawns near the player, is picked up by pressing E, triggers a pickup animation, then replaces the player's sprite set with bat-wielding variants and extends the attack hitbox.

**Architecture:** Two new files (`weapon.c` / `weapon.h`) handle the ground item. Player changes handle equipped state and sprite swap. `update.c` bridges them — when `weapon_just_picked_up()` fires, it calls `player_equip_bat()`.

**Tech Stack:** C, SDL3, SDL3_image. All sprites from `media/`.

---

## Files

| Action | File |
|---|---|
| Create | `topdown/weapon.c` |
| Create | `topdown/weapon.h` |
| Modify | `topdown/entity/player.c` |
| Modify | `topdown/entity/player.h` |
| Modify | `topdown/update.c` |
| Modify | `topdown/render.c` |
| Modify | `topdown/init.c` |
| Modify | `topdown/quit.c` |
| Modify | `CMakeLists.txt` |

---

## Sprites

### Ground item
- `media/Objects/Pickable/Bat.png` — sprite for the bat lying on the floor
- Size read from texture at load time via `SDL_GetTextureSize`

### Player — Main (unarmed, already loaded)
- `media/Character/Main/Idle/Character_<dir>_idle-Sheet6.png` — 6 frames
- `media/Character/Main/Run/Character_<dir>_run-Sheet6.png` — 6 frames
- `media/Character/Main/Punch/Character_<dir>_punch-Sheet4.png` — 4 frames
- `media/Character/Main/Pick-up/Character_<dir>_Pick-up-Sheet3.png` — 3 frames *(new)*

### Player — Bat (new, loaded at init)
- `media/Character/Bat/Bat_<dir>_idle-and-run-Sheet6.png` — 6 frames (idle holds frame 0, run cycles all 6)
- `media/Character/Bat/Bat_<dir>_attack-Sheet4.png` — 4 frames

Direction name mapping (same as existing): `down`, `up`, `side-left`, `side`

---

## Section 1: `weapon.h`

```c
#pragma once
#include <SDL3/SDL.h>
#include "camera.h"

void weapon_init(SDL_Renderer *renderer, float x, float y);
void weapon_update(SDL_FRect player_rect);
void weapon_render(SDL_Renderer *renderer, Camera camera);
void weapon_reset(void);
void weapon_destroy(void);
bool weapon_just_picked_up(void);
```

---

## Section 2: `weapon.c`

**Static state:**
```c
static SDL_Texture *tex;
static float spawn_x, spawn_y;
static float wx, wy;
static float item_w, item_h;
static bool  picked_up;
static bool  just_picked;
```

**`weapon_init(renderer, x, y)`:**
- Load `media/Objects/Pickable/Bat.png` via `IMG_LoadTexture`
- Set `SDL_BLENDMODE_BLEND`
- Read size via `SDL_GetTextureSize` into `item_w`, `item_h`
- Store `spawn_x = wx = x`, `spawn_y = wy = y`
- Clear `picked_up`, `just_picked`

**`weapon_update(player_rect)`:**
- Clear `just_picked = false` at top of function
- If `picked_up`: return early
- Read `SDL_GetKeyboardState`, check `SDL_SCANCODE_E`
- Build `SDL_FRect item_rect = { wx, wy, item_w, item_h }`
- If E pressed and player_rect overlaps item_rect: set `picked_up = true`, `just_picked = true`

**`weapon_render(renderer, camera)`:**
- If `picked_up`: return
- Project via `camera_project(camera, (SDL_FRect){wx, wy, item_w, item_h})`
- `SDL_RenderTexture(renderer, tex, NULL, &dst)`

**`weapon_reset()`:**
- `wx = spawn_x`, `wy = spawn_y`
- `picked_up = false`, `just_picked = false`

**`weapon_destroy()`:**
- `SDL_DestroyTexture(tex)`, `tex = NULL`

---

## Section 3: Player changes

### New AnimType value
```c
typedef enum {
  ANIM_IDLE   = 0,
  ANIM_RUN    = 1,
  ANIM_PUNCH  = 2,
  ANIM_PICKUP = 3,
  ANIM_COUNT  = 4,
} AnimType;
```

### Updated frame counts and durations
```c
static const int   frame_counts[ANIM_COUNT]    = { 6, 6, 4, 3 };
static const float frame_durations[ANIM_COUNT] = { 0.18f, 0.10f, 0.04f, 0.08f };
```

### New static state
```c
static SDL_Texture *bat_textures[ANIM_COUNT][DIR_COUNT];
static float bat_frame_w[ANIM_COUNT][DIR_COUNT];
static float bat_frame_h[ANIM_COUNT][DIR_COUNT];
static float bat_anchor_x[ANIM_COUNT][DIR_COUNT];

static bool bat_equipped  = false;
static bool is_picking_up = false;
```

### `player_init` additions
Load ANIM_PICKUP into `textures[ANIM_PICKUP][d]`:
- Path: `media/Character/Main/Pick-up/Character_<dir>_Pick-up-Sheet3.png`

Load bat textures into `bat_textures[a][d]`:
- ANIM_IDLE: `media/Character/Bat/Bat_<dir>_idle-and-run-Sheet6.png` — load independently per direction
- ANIM_RUN: `media/Character/Bat/Bat_<dir>_idle-and-run-Sheet6.png` — load independently (same file, separate `SDL_Texture*` pointer; consistent with how all other animations are loaded)
- ANIM_PUNCH: `media/Character/Bat/Bat_<dir>_attack-Sheet4.png`
- ANIM_PICKUP: not used for bat set — leave NULL; `bat_equipped` is only true after pickup finishes, so `ANIM_PICKUP` is never active while `bat_equipped` is true

Compute `bat_frame_w`, `bat_frame_h`, `bat_anchor_x` from loaded bat textures (same formula as main textures).

### `player_update` additions
- If `is_picking_up`: skip movement input and attack input; only advance `ANIM_PICKUP` animation
- When `ANIM_PICKUP` reaches its last frame (frame index == `frame_counts[ANIM_PICKUP] - 1`) and timer fires: set `bat_equipped = true`, `is_picking_up = false`, switch `anim_state = ANIM_IDLE`, reset `anim_frame = 0`, `anim_timer = 0`

### `player_render` changes
Select active texture set based on `bat_equipped`:
```c
SDL_Texture *tex  = bat_equipped ? bat_textures[anim_state][dir] : textures[anim_state][dir];
float fw = bat_equipped ? bat_frame_w[anim_state][dir] : frame_w[anim_state][dir];
float fh = bat_equipped ? bat_frame_h[anim_state][dir] : frame_h[anim_state][dir];
float ax = bat_equipped ? bat_anchor_x[anim_state][dir] : anchor_x[anim_state][dir];
```

### `player_get_attack_rect` changes

When `bat_equipped`, use `reach = 20.0f` as the offset in all four directions. When not equipped, preserve current behavior (`player.h` for UP/DOWN, `player.w` for LEFT/RIGHT):

```c
SDL_FRect player_get_attack_rect(void) {
  float ax = player.x, ay = player.y;
  if (bat_equipped) {
    float reach = 20.0f;
    switch (player.facing) {
      case DIR_UP:    ay -= reach;    break;
      case DIR_DOWN:  ay += reach;    break;
      case DIR_LEFT:  ax -= reach;    break;
      case DIR_RIGHT: ax += reach;    break;
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
```

### `player_reset` additions
```c
bat_equipped  = false;
is_picking_up = false;
```

### `player_destroy` additions
```c
for (int a = 0; a < ANIM_COUNT; a++)
  for (int d = 0; d < DIR_COUNT; d++)
    if (bat_textures[a][d]) SDL_DestroyTexture(bat_textures[a][d]);
```

### New API in `player.h`
```c
void player_equip_bat(void);
```

**`player_equip_bat()`:**
```c
void player_equip_bat(void) {
  is_picking_up = true;
  anim_state    = ANIM_PICKUP;
  anim_frame    = 0;
  anim_timer    = 0.0f;
}
```

---

## Section 4: Integration

### `update.c`
```c
#include "weapon.h"

// after player_update and camera_follow:
weapon_update(player_get_rect());
if (weapon_just_picked_up()) player_equip_bat();

// inside the GAME_DEAD restart branch:
weapon_reset();
```

### `render.c`
```c
#include "weapon.h"

// render order (back to front):
weapon_render(state->renderer, state->camera);
enemies_render(state->renderer, state->camera);
player_render(state->renderer, state->camera);
```

### `init.c`
```c
#include "weapon.h"

player_init(state->renderer);
SDL_FRect pr = player_get_rect();
weapon_init(state->renderer, pr.x + 20, pr.y);
```

### `quit.c`
```c
#include "weapon.h"

weapon_destroy();
```

### `CMakeLists.txt`
Add `topdown/weapon.c` to the source list alongside the other `topdown/*.c` files.
