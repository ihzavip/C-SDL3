#ifndef ENTITY_H
#define ENTITY_H

#include <SDL3/SDL.h>

/*
 * Direction enum — tracks which way the entity is facing.
 * We use it to draw a small "facing indicator" on the player so you can
 * tell at a glance which way they will move next.
 *
 * An enum is just a named set of integer constants. DIR_DOWN = 0, DIR_UP = 1,
 * etc. Using names instead of raw numbers makes the code self-documenting.
 */
typedef enum {
  DIR_DOWN  = 0,
  DIR_UP    = 1,
  DIR_LEFT  = 2,
  DIR_RIGHT = 3,
  DIR_COUNT
} Direction;

typedef struct {
  float x, y;       /* world-space position (top-left corner of the entity) */
  float w, h;       /* width and height in logical pixels                    */
  float speed;      /* movement speed in logical pixels per second           */
  Direction facing; /* which way the entity is currently facing              */
} Entity;

#endif
