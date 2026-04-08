# C-SDL3

A top-down game built with C and SDL3, used as a learning project for C and game development concepts.

## Build & Run

SDL3 is vendored — no system install needed.

```bash
# Configure (first time, or after changing CMakeLists.txt)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build --target topdown -j$(nproc)

# Run
./build/Debug/topdown
```

## Controls

| Key | Action |
|-----|--------|
| `WASD` / Arrow keys | Move |
| `Space` | Attack (in facing direction) |
| `Q` | Quit |

## Project Structure

```
topdown/       # SDL3 top-down game (active)
  init.c       # SDL_AppInit  — window, renderer, game setup
  events.c     # SDL_AppEvent — quit / key handling
  update.c     # SDL_AppIterate — delta time, camera, AI
  iterate.c    # SDL_AppIterate — ties update + render together
  render.c     # clear, draw world grid, entities, present
  quit.c       # SDL_AppQuit  — cleanup
  camera.c/h   # camera follow + world→screen projection
  world.h      # world and logical screen dimensions
  entity/
    entity.h   # shared Entity struct and Direction enum
    player.c/h # player movement, attack, rendering
    enemy.c/h  # patrol/chase AI, hit detection, rendering

game/          # terminal roguelike (separate, not SDL)
constants/     # shared map data for the terminal game
```
