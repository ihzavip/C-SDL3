#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
#include <stdbool.h>
#define SDL_MAIN_USE_CALLBACKS 1 /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_FRect rect = {100, 100, 20, 20};

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {

  SDL_SetRenderDrawColor(renderer, 33, 33, 33, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);

  /* Create the window */
  if (!SDL_CreateWindowAndRenderer("Hello World", SCREEN_WIDTH, SCREEN_HEIGHT,
                                   0, &window, &renderer)) {
    SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {

  if (event->type == SDL_EVENT_QUIT) {
    return SDL_APP_SUCCESS; /* end the program, reporting success to the OS.
                             */
  }

  if (event->type == SDL_EVENT_KEY_DOWN) {

    float new_x = rect.x;
    float new_y = rect.y;

    if (event->key.key == SDLK_DOWN || event->key.key == SDLK_J) {
      new_y += rect.h;
    }

    if (event->key.key == SDLK_UP || event->key.key == SDLK_K) {
      new_y -= rect.h;
    }

    if (event->key.key == SDLK_RIGHT || event->key.key == SDLK_L) {
      new_x += rect.w;
    }

    if (event->key.key == SDLK_LEFT || event->key.key == SDLK_H) {
      new_x -= rect.w;
    }

    if (event->key.key == SDLK_Q) {
      printf("Pressed number Q\n");
      return SDL_APP_SUCCESS;
    }

    rect.x = new_x;
    rect.y = new_y;

    printf("x: %f, y: %f, w: %f, h: %f\n", rect.x, rect.y, rect.w, rect.h);
  }

  return SDL_APP_CONTINUE;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate) {

  SDL_SetRenderDrawColor(renderer, 33, 33, 33,
                         SDL_ALPHA_OPAQUE); /* dark gray, full alpha */
  SDL_RenderClear(renderer);                /* start with a blank canvas. */

  // TODO: Render a box

  SDL_SetRenderDrawColor(renderer, 255, 33, 33, SDL_ALPHA_OPAQUE);

  // static float velocity = 0.2f;
  // rect.x += velocity;
  //
  // if (rect.x < 0 || rect.x + rect.w > SCREEN_WIDTH) {
  //   velocity = -velocity;
  // }

  SDL_RenderFillRect(renderer, &rect);
  SDL_RenderPresent(renderer);

  // Default
  // const char *message = "why? should we have initial value. Nice";
  // int w = 0, h = 0;
  // float x, y;
  // const float scale = 4.0f;
  //
  // /* Center the message and scale it up */
  // SDL_GetRenderOutputSize(renderer, &w, &h);
  // SDL_SetRenderScale(renderer, scale, scale);
  // x = ((w / scale) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE *
  // SDL_strlen(message)) /
  //     2;
  // y = ((h / scale) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE) / 2;
  //
  // /* Draw the message */
  // SDL_SetRenderDrawColor(renderer, 30, 30, 120, 255);
  // SDL_RenderClear(renderer);
  // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  // SDL_RenderDebugText(renderer, x, y, message);
  // SDL_RenderPresent(renderer);

  return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result) {

  if (renderer) {
    SDL_DestroyRenderer(renderer);
  }

  if (window) {
    SDL_DestroyWindow(window);
  }
}
