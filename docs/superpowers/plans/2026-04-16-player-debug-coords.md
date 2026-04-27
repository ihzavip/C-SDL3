# Player Debug Coordinates Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Display the player's world-space x/y coordinates below the sprite each frame, in a form that is trivial to comment out.

**Architecture:** Add `player_render_debug` to `player.c` — a standalone function that draws a text label below the player sprite using `SDL_RenderDebugText`. It is called from `render.c` on one line, making it easy to comment out without touching render logic.

**Tech Stack:** C, SDL3 (`SDL_RenderDebugText` — no font loading required)

---

### Task 1: Declare `player_render_debug` in `player.h`

**Files:**
- Modify: `topdown/entity/player.h`

- [ ] **Step 1: Add the function declaration**

In `topdown/entity/player.h`, add after `player_render`:

```c
void player_render_debug(SDL_Renderer *renderer, Camera camera);
```

Result:

```c
void player_render(SDL_Renderer *renderer, Camera camera);
void player_render_debug(SDL_Renderer *renderer, Camera camera); /* DEBUG: comment out to hide coords */
void player_destroy(void);
```

- [ ] **Step 2: Verify it compiles**

```bash
cmake --build build --target topdown -j$(nproc)
```

Expected: no errors. The function is declared but not yet defined — the linker won't complain until it's called.

---

### Task 2: Implement `player_render_debug` in `player.c`

**Files:**
- Modify: `topdown/entity/player.c`

- [ ] **Step 1: Add the function at the bottom of `player.c`, before `player_destroy`**

```c
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
```

- [ ] **Step 2: Build**

```bash
cmake --build build --target topdown -j$(nproc)
```

Expected: no errors or warnings.

---

### Task 3: Call `player_render_debug` from `render.c`

**Files:**
- Modify: `topdown/render.c`

- [ ] **Step 1: Add the call after `player_render`**

In `topdown/render.c`, find:

```c
  enemies_render(state->renderer, state->camera);
  player_render(state->renderer, state->camera);
```

Change to:

```c
  enemies_render(state->renderer, state->camera);
  player_render(state->renderer, state->camera);
  player_render_debug(state->renderer, state->camera); /* DEBUG: comment out to hide coords */
```

- [ ] **Step 2: Build and run**

```bash
cmake --build build --target topdown -j$(nproc) && ./build/Debug/topdown
```

Expected: small white text `x:314 y:172` (or similar) appears just below the player sprite and updates as the player moves.

- [ ] **Step 3: Commit**

```bash
git add topdown/entity/player.h topdown/entity/player.c topdown/render.c
git commit -m "feat: add player world-coord debug overlay"
```
