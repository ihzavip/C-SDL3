#pragma once
#include "../camera.h"
#include "entity.h"
#include <SDL3/SDL.h>

/*
 * player_init now needs the renderer because loading a texture requires one.
 * SDL_image decodes the PNG into pixels, then SDL uploads them to the GPU as
 * a texture — both steps need the renderer.
 */
void player_init(SDL_Renderer *renderer);
void player_update(float delta);
void player_render(SDL_Renderer *renderer, Camera camera);
void player_destroy(void); /* free all loaded textures */

SDL_FRect player_get_rect(void);
SDL_FRect player_get_attack_rect(void);
bool      player_is_attacking(void);
Direction player_get_facing(void);
