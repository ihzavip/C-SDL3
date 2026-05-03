# Code Style

- Always check return values from SDL functions (`SDL_Init`, `SDL_CreateWindow`, etc.) — SDL uses `SDL_FALSE` or NULL on failure
- Destroy/free SDL resources in `topdown/quit.c`, not inline in other files
- No globals — all shared state goes through `AppState` in `common.h`
- Follow the existing code formatting — do not align `=` signs into columns; let the auto-formatter decide spacing
