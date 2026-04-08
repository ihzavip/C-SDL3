#pragma once
#include "../camera.h"
#include <SDL3/SDL.h>

void player_init(void);
void player_update(float delta);
void player_render(SDL_Renderer *renderer, Camera camera);

/*
 * Getters — let other systems (enemy AI, camera) read player state without
 * accessing the internal `player` variable directly.
 *
 * player_get_rect()        → current bounding box in world space
 * player_get_attack_rect() → hitbox in front of the player when attacking
 * player_is_attacking()    → true during the brief attack window
 */
SDL_FRect player_get_rect(void);
SDL_FRect player_get_attack_rect(void);
bool player_is_attacking(void);
