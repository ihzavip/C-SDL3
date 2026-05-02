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
void player_reset(void); /* restart: resets position, hp, animation — no texture reload */
void player_update(float delta);

/*
This function called in render.c, so keep in mind that it will call for every
frame
*/
void player_render(SDL_Renderer *renderer, Camera camera);
void player_render_debug(SDL_Renderer *renderer, Camera camera); /* DEBUG: comment out to hide coords */
void player_destroy(void); /* free all loaded textures */

SDL_FRect player_get_rect(void);
SDL_FRect player_get_attack_rect(void);
bool      player_is_attacking(void);
Direction player_get_facing(void);
int       player_get_hp(void);
int       player_get_max_hp(void);
void      player_take_damage(int amount);
void      player_equip_bat(void);
