#pragma once
#include <SDL3/SDL.h>
#include "camera.h"

void tilemap_init(SDL_Renderer *renderer);
void tilemap_render(SDL_Renderer *renderer, Camera camera);
void tilemap_destroy(void);
