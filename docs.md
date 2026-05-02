# Topdown Game — Documentation

## Overview

A 2D top-down game built with SDL3's callback architecture. The player moves through a scrolling world, fights enemies, and is animated with spritesheets.

---

## Architecture

SDL3 callback architecture (`SDL_MAIN_USE_CALLBACKS`) — no traditional `main()` loop. SDL drives four callbacks:

| Callback | File | Role |
|---|---|---|
| `SDL_AppInit` | `topdown/init.c` | Window, renderer, player & enemy setup |
| `SDL_AppEvent` | `topdown/events.c` | Quit on window close or `Q` key |
| `SDL_AppIterate` | `topdown/iterate.c` | update → render → frame cap, every frame |
| `SDL_AppQuit` | `topdown/quit.c` | Destroy all resources |

All shared state is carried in `AppState` (`topdown/common.h`) and passed through SDL's `void *appstate` pointer — no globals.

---

## Coordinate Systems

Defined in `topdown/world.h`:

| Constant | Value | Meaning |
|---|---|---|
| `WORLD_W / WORLD_H` | 640 × 360 | Total world size in logical pixels |
| `LOGICAL_W / LOGICAL_H` | 320 × 180 | Camera viewport (what the player sees) |

SDL scales 320×180 up to the actual window (1280×720 by default) via `SDL_SetRenderLogicalPresentation`. All game code works in logical pixels — never screen pixels.

---

## Frame Loop

Each frame in `topdown/iterate.c`:

1. **`app_update`** — advance game state
2. **`app_render`** — draw everything
3. **`app_wait_for_next_frame`** — cap at 144 FPS with `SDL_Delay`

---

## Update (`topdown/update.c`)

Each frame:

1. Compute `delta_time` — `(current_tick - last_tick) / 1000.0f` in seconds. All movement multiplies by this so speed is frame-rate independent.
2. Call `player_update(delta)`.
3. Call `camera_follow(...)` with the player's new position — done *after* player update to avoid a one-frame lag.
4. Call `enemies_update(...)` passing player position, attack rect, and attack state.

---

## Camera (`topdown/camera.c`)

The `Camera` struct holds the top-left corner of the visible world area (`x, y`).

**`camera_follow`** — centres the camera on a target rect, then clamps so it never shows outside the world:

```
camera.x = target_centre_x - LOGICAL_W / 2
camera.y = target_centre_y - LOGICAL_H / 2
// clamp to [0, WORLD_W - LOGICAL_W] and [0, WORLD_H - LOGICAL_H]
```

**`camera_project`** — converts a world-space `SDL_FRect` to screen-space by subtracting the camera offset. Every entity must call this before any `SDL_Render*` call.

---

## Entity System

### Base Entity (`topdown/entity/entity.h`)

```c
typedef struct {
  float x, y;       // world-space position (top-left)
  float w, h;       // size in logical pixels
  float speed;      // logical pixels per second
  Direction facing; // DIR_DOWN, DIR_UP, DIR_LEFT, DIR_RIGHT
} Entity;
```

### Player (`topdown/entity/player.c`)

The `player` variable is `static` — private to `player.c`. All external access goes through the public API in `player.h`.

**Initialisation** (`player_init`):
- Spawns at world centre.
- Loads 12 spritesheets: 3 animations × 4 directions from `media/Character/Main/`.
- Reads actual texture dimensions to derive `frame_w` / `frame_h` per sheet.

**Animation system**:

| Animation | Frames | Frame duration | Notes |
|---|---|---|---|
| `ANIM_IDLE` | 6 | 0.18s | Holds frame 0 to avoid sliding look |
| `ANIM_RUN` | 6 | 0.10s | Loops |
| `ANIM_PUNCH` | 4 | 0.04s | Plays once, holds last frame |

Priority: `ANIM_PUNCH` > `ANIM_RUN` > `ANIM_IDLE`. Switching animation resets to frame 0.

**Movement** (`player_update`):
- WASD or arrow keys move the player by `speed * delta` per frame.
- Position clamped to world bounds with `SDL_clamp`.
- Spacebar triggers attack (`is_attacking = true`) for 0.20s (`ATTACK_DURATION`).

**Attack hitbox** (`player_get_attack_rect`): a rect the same size as the player, placed one player-width ahead in the facing direction.

**Rendering** (`player_render`):
- Source rect from spritesheet: `{ frame_index * frame_w, 0, frame_w, frame_h }`.
- `anchor_x` corrects for wider punch frames (fist extends left in left-facing direction).
- Fallback: green rect if texture failed to load.

**Public API**:

| Function | Returns |
|---|---|
| `player_get_rect()` | Physics bounding box |
| `player_get_attack_rect()` | Active attack hitbox |
| `player_is_attacking()` | Whether attack is active this frame |
| `player_get_facing()` | Current `Direction` |
| `player_destroy()` | Frees all 12 textures |

---

### Enemy (`topdown/entity/enemy.c`)

Fixed array of up to 6 enemies (`MAX_ENEMIES`). The `Enemy` struct embeds `Entity base` as its first field.

**States**:

| State | Colour | Behaviour |
|---|---|---|
| `ENEMY_PATROL` | Orange-brown | Walks A → B → A between two waypoints |
| `ENEMY_CHASE` | Bright red | Moves straight toward player centre |
| `ENEMY_DEAD` | Dark splat | Skipped in update, drawn as blood splat |

**State transitions**:
- `PATROL → CHASE`: player centre enters `DETECTION_RADIUS` (70 px). One-way — enemies never de-aggro.
- `Any → DEAD`: player attack rect overlaps enemy rect while `player_attacking` is true. One-hit kill.

**Movement** (`move_toward`): normalises direction vector `(dx, dy) / dist` then applies `speed * delta`, so diagonal speed equals cardinal speed. Updates `facing` toward the dominant axis.

**Patrol**: flips `going_to_b` when within 2px of the current waypoint.

---

## Render (`topdown/render.c`)

Each frame:

1. Clear with dark green background (`60, 80, 60`).
2. Draw a 32px world-space grid — lines projected by subtracting `camera.x / camera.y`. Visualises camera movement.
3. `enemies_render` — drawn before player (painter's algorithm, player appears on top).
4. `player_render`.
5. `SDL_RenderPresent` — swap back buffer to screen.

---

## Events (`topdown/events.c`)

- `SDL_EVENT_QUIT` (window close) → exit
- `Q` key → exit

Movement and attack input is polled via `SDL_GetKeyboardState` inside `player_update`, not through the event queue.

---

## Cleanup (`topdown/quit.c`)

Order matters:

1. `player_destroy()` — frees all 12 textures **before** the renderer is destroyed.
2. `SDL_DestroyRenderer` / `SDL_DestroyWindow`.
3. `SDL_QuitSubSystem(SDL_INIT_VIDEO)`.

---

## Asset Layout

```
media/Character/Main/
  Idle/   Character_down_idle-Sheet6.png    (+ up, side-left, side)
  Run/    Character_down_run-Sheet6.png     (+ up, side-left, side)
  Punch/  Character_down_punch-Sheet4.png   (+ up, side-left, side)
```

The game must be run from the project root so these relative paths resolve correctly.

---

## Debug Toggles

| What | Where | How to disable |
|---|---|---|
| Sprite frame outline (green) + physics box (red) | `player.c:player_render` | Remove the two `SDL_RenderRect` calls |
| World-space coords label | `render.c` | Uncomment `player_render_debug` call |
| Enemy spawning | `init.c` | Uncomment `enemies_init()` call |
| Attack hitbox overlay | `player.c:player_render` | Uncomment the `is_attacking` block |
