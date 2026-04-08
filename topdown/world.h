#pragma once

/*
 * world.h — single source of truth for world and screen dimensions.
 *
 * The "world" is the full space where entities live and move.
 * The "logical screen" is what the player sees at any one moment — the camera
 * is a 320×180 window that pans across the larger 640×360 world.
 *
 * Both sets of constants live here so that camera.c, player.c, render.c etc.
 * all agree on the same numbers without hardcoding them in multiple places.
 *
 * LOGICAL_W/H must match the arguments to SDL_SetRenderLogicalPresentation()
 * in init.c — if you change one, change the other.
 */

#define WORLD_W   640   /* total world width  in logical pixels */
#define WORLD_H   360   /* total world height in logical pixels */

#define LOGICAL_W 320   /* camera viewport width  */
#define LOGICAL_H 180   /* camera viewport height */
