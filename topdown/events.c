#include <SDL3/SDL.h>

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {

  const bool *keyboard_state = SDL_GetKeyboardState(NULL);

  if (event->type == SDL_EVENT_QUIT) {
    return SDL_APP_SUCCESS; /* end the program, reporting success to the OS.
                             */
  }

  if (event->type == SDL_EVENT_KEY_DOWN) {
    if (event->key.key == SDLK_Q) {
      return SDL_APP_SUCCESS;
    }
  }

  return SDL_APP_CONTINUE;
}
