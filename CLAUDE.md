# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

The build directory is `build/`. SDL3 is vendored under `vendored/SDL` and built from source.

```bash
# Configure (first time or after CMakeLists changes)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build --target topdown -j$(nproc)

# Run
./build/Debug/topdown
```

The binary lands at `build/Debug/topdown` (or `build/Release/topdown` for release builds).

## Architecture

There are two game codebases in this repo:

### `topdown/` — active SDL3 game (the one being developed)

Uses SDL3's **callback architecture** (`SDL_MAIN_USE_CALLBACKS`): instead of a `main()` loop, SDL calls four functions that are each split into their own file:

| File | SDL callback | Purpose |
|---|---|---|
| `init.c` | `SDL_AppInit` | Create window, renderer, call `player_init()` |
| `events.c` | `SDL_AppEvent` | Handle quit / key-down events |
| `update.c` | `SDL_AppIterate` (first half) | Advance delta time, call `player_update()` |
| `iterate.c` | `SDL_AppIterate` (full) | Call update → render → frame cap |
| `render.c` | — | Clear screen, call `player_render()`, present |
| `quit.c` | `SDL_AppQuit` | Destroy renderer and window |

**Shared state** lives in `AppState` (defined in `common.h`) and flows through SDL's `void *appstate` pointer — there are no globals in the main loop code.

**Entity system**: `topdown/entity/entity.h` defines the `Entity` struct (`x, y, w, h, speed, facing`) and the `Direction` enum. Each entity type gets its own `_init / _update / _render` set of functions. Currently only `player.c` exists. The `player` variable inside `player.c` is `static` (private to that file) — other modules go through the `player_*` functions.

**Logical resolution**: the renderer is set to 320×180 (letterboxed) in `init.c`. All positions and sizes are in these logical pixels. `player.c` has `LOGICAL_W` / `LOGICAL_H` defines that must stay in sync with `init.c`.

### `game/` — terminal roguelike (separate, not SDL)

A separate turn-based game rendered with `printf` and raw terminal mode (`termios`). Has its own `Entity` struct (with `hp`, `attack`, `defense` — not the same as `topdown/entity/entity.h`), a 20×20 char map in `constants/map.c`, and a dice-based combat system in `game/combat.c`. Built separately from the SDL game and not wired into CMakeLists.

### `src/sdl_hello.c`

An earlier SDL3 prototype with global static state. Superseded by `topdown/`. Ignore it.
