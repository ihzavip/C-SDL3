#pragma once
#include <SDL3/SDL.h>

/*
 * camera.h — a camera that pans across the world and projects world
 * coordinates onto the screen.
 *
 * The Camera tracks the top-left corner of the visible area in *world space*.
 * To draw an entity, you subtract (camera.x, camera.y) from its world
 * position — that gives you where it appears on the screen.
 *
 * Example:
 *   Entity is at world (350, 200). Camera is at (300, 150).
 *   Screen position = (350 - 300, 200 - 150) = (50, 50). ✓
 */
typedef struct {
  float x; /* world-space X of the left edge of the camera view  */
  float y; /* world-space Y of the top  edge of the camera view  */
} Camera;

/*
 * camera_follow — smoothly centres the camera on a target rectangle
 * (usually the player) and clamps it so it never shows outside the world.
 *
 * target_x/y/w/h describe the rectangle to follow in world space.
 */
void camera_follow(Camera *cam, float target_x, float target_y,
                   float target_w, float target_h);

/*
 * camera_project — converts a world-space SDL_FRect into a screen-space
 * SDL_FRect by subtracting the camera offset.
 *
 * Use this before every SDL_Render* call so things appear in the right place
 * on screen even as the camera moves.
 */
SDL_FRect camera_project(Camera cam, SDL_FRect world_rect);
