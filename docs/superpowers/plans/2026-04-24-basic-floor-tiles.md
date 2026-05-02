# Basic Floor Tiles Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the debug grid with a tiled floor using `Background_Green_TileSet.png`, rendering only tiles visible within the camera view each frame.

**Architecture:** A new `tilemap.c` / `tilemap.h` module follows the existing `_init / _render / _destroy` pattern. It loads one tileset texture, picks the first 16×16 tile as the floor, and loops over the camera-visible world area each frame to stamp it. The debug grid in `render.c` is commented out and replaced with a `tilemap_render` call.

**Tech Stack:** C, SDL3, SDL3_image

---

### Task 1: Add `tilemap.h`

**Files:**
- Create: `topdown/tilemap.h`

- [ ] **Step 1: Create the header**

```c
#pragma once
#include <SDL3/SDL.h>
#include "camera.h"

void tilemap_init(SDL_Renderer *renderer);
void tilemap_render(SDL_Renderer *renderer, Camera camera);
void tilemap_destroy(void);
```

- [ ] **Step 2: Build to verify it compiles clean**

```bash
cmake --build build --target topdown -j$(nproc)
```

Expected: no errors (header not included anywhere yet).

---

### Task 2: Implement `tilemap.c`

**Files:**
- Create: `topdown/tilemap.c`

- [ ] **Step 1: Create the implementation**

```c
#include "tilemap.h"
#include "world.h"
#include <SDL3_image/SDL_image.h>

#define TILE_SIZE 16

static SDL_Texture *tileset = NULL;

void tilemap_init(SDL_Renderer *renderer) {
  tileset = IMG_LoadTexture(renderer, "media/Tiles/Background_Green_TileSet.png");
  if (!tileset) {
    SDL_Log("Failed to load tileset: %s", SDL_GetError());
    return;
  }
  SDL_SetTextureBlendMode(tileset, SDL_BLENDMODE_NONE);
}

void tilemap_render(SDL_Renderer *renderer, Camera camera) {
  if (!tileset) return;

  /* First tile in the sheet — top-left 16×16 cell */
  SDL_FRect src = { 0, 0, TILE_SIZE, TILE_SIZE };

  /*
   * Only draw tiles that overlap the camera view.
   * Convert camera edges to tile indices, clamp to world bounds.
   */
  int col_start = (int)(camera.x / TILE_SIZE);
  int col_end   = (int)((camera.x + LOGICAL_W) / TILE_SIZE) + 1;
  int row_start = (int)(camera.y / TILE_SIZE);
  int row_end   = (int)((camera.y + LOGICAL_H) / TILE_SIZE) + 1;

  if (col_start < 0)               col_start = 0;
  if (row_start < 0)               row_start = 0;
  if (col_end   > WORLD_W / TILE_SIZE) col_end = WORLD_W / TILE_SIZE;
  if (row_end   > WORLD_H / TILE_SIZE) row_end = WORLD_H / TILE_SIZE;

  for (int row = row_start; row < row_end; row++) {
    for (int col = col_start; col < col_end; col++) {
      SDL_FRect world_tile = {
        (float)(col * TILE_SIZE),
        (float)(row * TILE_SIZE),
        TILE_SIZE,
        TILE_SIZE
      };
      SDL_FRect dst = {
        world_tile.x - camera.x,
        world_tile.y - camera.y,
        TILE_SIZE,
        TILE_SIZE
      };
      SDL_RenderTexture(renderer, tileset, &src, &dst);
    }
  }
}

void tilemap_destroy(void) {
  if (tileset) {
    SDL_DestroyTexture(tileset);
    tileset = NULL;
  }
}
```

- [ ] **Step 2: Add `tilemap.c` to `CMakeLists.txt`**

In `CMakeLists.txt`, find the source list and add `topdown/tilemap.c`:

```cmake
  topdown/init.c
  topdown/events.c
  topdown/update.c
  topdown/iterate.c
  topdown/render.c
  topdown/quit.c
  topdown/camera.c
  topdown/tilemap.c
  topdown/entity/player.c
  topdown/entity/enemy.c
```

- [ ] **Step 3: Build to verify it compiles**

```bash
cmake --build build --target topdown -j$(nproc)
```

Expected: no errors (tilemap not wired up yet, so no linker issues).

---

### Task 3: Wire `tilemap_init` and `tilemap_destroy`

**Files:**
- Modify: `topdown/init.c`
- Modify: `topdown/quit.c`

- [ ] **Step 1: Add `#include "tilemap.h"` and call `tilemap_init` in `init.c`**

Add the include at the top of `topdown/init.c`:

```c
#include "tilemap.h"
```

Add the call after `enemies_init()`:

```c
  player_init(state->renderer);
  enemies_load_textures(state->renderer);
  enemies_init();
  tilemap_init(state->renderer);
```

- [ ] **Step 2: Add `#include "tilemap.h"` and call `tilemap_destroy` in `quit.c`**

Add the include at the top of `topdown/quit.c`:

```c
#include "tilemap.h"
```

Add the call before `SDL_DestroyRenderer`:

```c
  player_destroy();
  enemies_destroy_textures();
  tilemap_destroy();

  SDL_DestroyRenderer(state->renderer);
```

- [ ] **Step 3: Build**

```bash
cmake --build build --target topdown -j$(nproc)
```

Expected: no errors.

---

### Task 4: Hook `tilemap_render` into `render.c` and comment out the debug grid

**Files:**
- Modify: `topdown/render.c`

- [ ] **Step 1: Add `#include "tilemap.h"` to `render.c`**

```c
#include "render.h"
#include "common.h"
#include "world.h"
#include "tilemap.h"
#include "entity/player.h"
#include "entity/enemy.h"
```

- [ ] **Step 2: Comment out the debug grid and replace with `tilemap_render`**

Find the `/* --- World grid --- */` block and replace it:

```c
  /* --- Floor tiles -------------------------------------------------------- */
  tilemap_render(state->renderer, state->camera);

  /* --- World grid (debug) ------------------------------------------------- */
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
```

- [ ] **Step 3: Build and run**

```bash
cmake --build build --target topdown -j$(nproc) && ./build/Debug/topdown
```

Expected: the world floor is covered with green tiles. The debug grid is gone. Entities render on top of the tiles. Camera panning still works.

- [ ] **Step 4: Commit**

```bash
git add topdown/tilemap.h topdown/tilemap.c topdown/init.c topdown/quit.c topdown/render.c CMakeLists.txt
git commit -m "feat: add basic floor tile rendering"
```
