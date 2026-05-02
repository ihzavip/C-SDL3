#pragma once
#include "../camera.h"
#include <SDL3/SDL.h>

/*
 * enemy.h — public interface for the enemy system.
 *
 * All enemies are managed internally as a fixed-size array (no dynamic
 * allocation needed for a small game). External code only calls these three
 * functions; the Enemy struct and array stay private inside enemy.c.
 */

/* Load all enemy spritesheets. Call once at startup before enemies_init(). */
void enemies_load_textures(SDL_Renderer *renderer);

/* Free all enemy spritesheets. Call in SDL_AppQuit before destroying renderer. */
void enemies_destroy_textures(void);

/* Spawn all enemies at their starting positions. Call once at startup. */
void enemies_init(void);

/* Reset enemies to their starting state (call on restart). */
void enemies_reset(void);

/*
 * enemies_update — advance all enemy AI by one frame.
 *
 * Parameters the AI needs from outside:
 *   delta          — seconds since last frame (for frame-rate independent move)
 *   player_rect    — where the player is in world space (for chase + detection)
 *   attack_rect    — the player's attack hitbox in world space
 *   player_attacking — whether the attack is active this frame
 */
/* Returns true if any enemy touched the player this frame (so the caller can
   apply damage without enemy.c needing to know about player internals). */
bool enemies_update(float delta, SDL_FRect player_rect, SDL_FRect attack_rect,
                    bool player_attacking);

/* Draw all enemies, projected through the camera onto the screen. */
void enemies_render(SDL_Renderer *renderer, Camera camera);
