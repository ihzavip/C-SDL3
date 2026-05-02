#pragma once
#include <SDL3/SDL.h>
#include "camera.h"

void weapon_init(SDL_Renderer *renderer, float x, float y);
void weapon_update(SDL_FRect player_rect);
void weapon_render(SDL_Renderer *renderer, Camera camera);
void weapon_reset(void);
void weapon_destroy(void);
bool weapon_just_picked_up(void);
