#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keycode.h>
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

static SDL_Texture *texture = NULL;
static SDL_Surface *gHelloWorld = NULL;
const char *imagePath = "media/hello-sdl3.bmp";

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {

  /* Create the window */
  if (!SDL_CreateWindowAndRenderer("Mantap", SCREEN_WIDTH, SCREEN_HEIGHT, 0,
                                   &window, &renderer)) {
    SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // Load Image
  gHelloWorld = SDL_LoadBMP(imagePath);
  if (!gHelloWorld) {
    SDL_Log("Unable to load image %s! SDL Error: %s", imagePath,
            SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // Load Texture
  texture = SDL_CreateTextureFromSurface(renderer, gHelloWorld);
  if (!texture) {
    SDL_Log("Unable to create texture! SDL Error: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  SDL_DestroySurface(gHelloWorld);
  gHelloWorld = NULL;
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {

  if (event->type == SDL_EVENT_QUIT) {
    return SDL_APP_SUCCESS; /* end the program, reporting success to the OS.
                             */
  }

  if (event->type == SDL_EVENT_KEY_DOWN) {
    if (event->key.key == SDLK_1) {
      printf("Pressed number 1\n");
      return SDL_APP_SUCCESS;
    }
  }

  return SDL_APP_CONTINUE;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate) {

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  SDL_RenderTexture(renderer, texture, NULL, NULL);
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
  if (texture) {
    SDL_DestroyTexture(texture);
  }

  if (renderer) {
    SDL_DestroyRenderer(renderer);
  }

  if (window) {
    SDL_DestroyWindow(window);
  }
}
