# Entity System

- Never access `player` directly from outside `player.c` — always go through `player_*` functions
- New entity types go in `topdown/entity/<name>.c` + `<name>.h`
- Every entity type must implement `_init`, `_update`, and `_render` functions
- All positions and sizes are in logical pixels (320×180) — never use screen pixel coordinates
- Never hardcode screen dimensions — use `LOGICAL_W` / `LOGICAL_H`
- `LOGICAL_W` / `LOGICAL_H` in `player.c` must stay in sync with the renderer resolution in `init.c`
