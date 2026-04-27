# Code Style

- Always check return values from SDL functions (`SDL_Init`, `SDL_CreateWindow`, etc.) — SDL uses `SDL_FALSE` or NULL on failure
- Destroy/free SDL resources in `topdown/quit.c`, not inline in other files
- No globals — all shared state goes through `AppState` in `common.h`
