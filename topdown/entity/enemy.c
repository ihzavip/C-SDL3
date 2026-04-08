#include "enemy.h"
#include "entity.h"
#include "../camera.h"

#define MAX_ENEMIES     6
#define DETECTION_RADIUS 70.0f  /* world pixels — how close before chasing */

/*
 * EnemyState — what the enemy is currently doing.
 *
 * PATROL: walking back and forth between two waypoints, unaware of player.
 * CHASE:  locked onto the player, moving straight toward them.
 * DEAD:   no longer updated, drawn as a dark splat on the floor.
 */
typedef enum {
  ENEMY_PATROL,
  ENEMY_CHASE,
  ENEMY_DEAD,
} EnemyState;

/*
 * Enemy — extends the base Entity with AI-specific data.
 *
 * In C there is no inheritance, so we embed Entity as the first field
 * (`base`). This lets us pass &enemy.base wherever an Entity* is needed,
 * and keeps position/size/speed/facing in one place.
 */
typedef struct {
  Entity     base;
  EnemyState state;

  /* Patrol waypoints in world space. The enemy walks A→B, then B→A, repeat. */
  float patrol_ax, patrol_ay;
  float patrol_bx, patrol_by;
  bool  going_to_b; /* true → heading toward B, false → heading toward A */
} Enemy;

/* The enemy array. `static` keeps it private to this file. */
static Enemy enemies[MAX_ENEMIES];
static int   enemy_count = 0;

/* -------------------------------------------------------------------------
 * Internal helpers
 * ------------------------------------------------------------------------- */

/*
 * add_enemy — convenience function to initialise one enemy slot.
 *
 * The enemy starts at (ax, ay) and patrols to (bx, by).
 */
static void add_enemy(float ax, float ay, float bx, float by) {
  if (enemy_count >= MAX_ENEMIES) return;

  Enemy *e       = &enemies[enemy_count++];
  e->base.x      = ax;
  e->base.y      = ay;
  e->base.w      = 8;
  e->base.h      = 8;
  e->base.speed  = 35;         /* world pixels per second */
  e->base.facing = DIR_RIGHT;
  e->state       = ENEMY_PATROL;
  e->patrol_ax   = ax;
  e->patrol_ay   = ay;
  e->patrol_bx   = bx;
  e->patrol_by   = by;
  e->going_to_b  = true;
}

/*
 * move_toward — move enemy `e` one step toward world point (tx, ty).
 *
 * We compute a direction vector from the enemy to the target, normalise it
 * to length 1 (so diagonal movement is the same speed as cardinal movement),
 * then scale by speed * delta.
 *
 * Normalisation: divide each component by the total distance.
 *   dir = (dx, dy) / sqrt(dx² + dy²)
 */
static void move_toward(Enemy *e, float tx, float ty, float delta) {
  float dx   = tx - e->base.x;
  float dy   = ty - e->base.y;
  float dist = SDL_sqrtf(dx * dx + dy * dy);

  if (dist < 1.0f) return; /* already at target, nothing to do */

  float step  = e->base.speed * delta;
  e->base.x  += (dx / dist) * step;
  e->base.y  += (dy / dist) * step;

  /* Update facing toward the dominant movement axis */
  if (SDL_fabsf(dx) > SDL_fabsf(dy))
    e->base.facing = (dx > 0) ? DIR_RIGHT : DIR_LEFT;
  else
    e->base.facing = (dy > 0) ? DIR_DOWN : DIR_UP;
}

/*
 * rects_overlap — axis-aligned bounding box (AABB) collision test.
 *
 * Two rectangles overlap when they are NOT separated on either axis.
 * They are separated on the X axis when one is fully to the left of the other:
 *   a.x + a.w <= b.x  OR  b.x + b.w <= a.x
 * Invert that condition to get "they DO overlap". Same for Y.
 */
static bool rects_overlap(SDL_FRect a, SDL_FRect b) {
  return a.x         < b.x + b.w &&
         a.x + a.w   > b.x       &&
         a.y         < b.y + b.h &&
         a.y + a.h   > b.y;
}

/*
 * centre_dist — distance between the centre points of two rects.
 *
 * We compare centres (not corners) so detection feels symmetric regardless
 * of which direction the player approaches from.
 */
static float centre_dist(SDL_FRect a, SDL_FRect b) {
  float dx = (a.x + a.w * 0.5f) - (b.x + b.w * 0.5f);
  float dy = (a.y + a.h * 0.5f) - (b.y + b.h * 0.5f);
  return SDL_sqrtf(dx * dx + dy * dy);
}

/* -------------------------------------------------------------------------
 * Public functions
 * ------------------------------------------------------------------------- */

void enemies_init(void) {
  enemy_count = 0;

  /*
   * Place enemies around the world with interesting patrol paths.
   * Arguments: start-x, start-y, end-x, end-y.
   * Feel free to add more or change positions.
   */
  add_enemy( 80,  60, 200,  60);  /* horizontal patrol, top-left area  */
  add_enemy(300,  40, 300, 140);  /* vertical patrol, top-centre        */
  add_enemy(450, 180, 570, 180);  /* horizontal patrol, mid-right       */
  add_enemy(140, 240, 140, 320);  /* vertical patrol, mid-left          */
  add_enemy(380,  90, 500, 260);  /* diagonal-ish patrol                */
  add_enemy(540, 290, 610, 290);  /* short horizontal patrol, bottom    */
}

void enemies_update(float delta, SDL_FRect player_rect,
                    SDL_FRect attack_rect, bool player_attacking) {
  for (int i = 0; i < enemy_count; i++) {
    Enemy *e = &enemies[i];

    if (e->state == ENEMY_DEAD) continue; /* skip dead enemies */

    SDL_FRect erect = { e->base.x, e->base.y, e->base.w, e->base.h };

    /* --- Attack hit detection ------------------------------------------- */
    /*
     * If the player is swinging this frame AND the attack rect overlaps this
     * enemy's rect, the enemy dies. One-hit kill, Hotline Miami style.
     */
    if (player_attacking && rects_overlap(attack_rect, erect)) {
      e->state = ENEMY_DEAD;
      continue;
    }

    /* --- State transition: patrol → chase -------------------------------- */
    /*
     * Once an enemy spots the player (comes within DETECTION_RADIUS) they
     * switch to CHASE and never go back to PATROL.
     * This is intentional — enemies stay alert once alerted.
     */
    if (e->state == ENEMY_PATROL && centre_dist(erect, player_rect) < DETECTION_RADIUS) {
      e->state = ENEMY_CHASE;
    }

    /* --- Movement -------------------------------------------------------- */
    if (e->state == ENEMY_CHASE) {
      /*
       * Move toward the player's centre, offsetting by half the enemy size so
       * the enemy centres on the player rather than its own top-left corner.
       */
      float tx = player_rect.x + player_rect.w * 0.5f - e->base.w * 0.5f;
      float ty = player_rect.y + player_rect.h * 0.5f - e->base.h * 0.5f;
      move_toward(e, tx, ty, delta);

    } else {
      /* PATROL: walk toward the current waypoint, flip when close enough */
      float tx = e->going_to_b ? e->patrol_bx : e->patrol_ax;
      float ty = e->going_to_b ? e->patrol_by : e->patrol_ay;

      float dx   = tx - e->base.x;
      float dy   = ty - e->base.y;
      float dist = SDL_sqrtf(dx * dx + dy * dy);

      if (dist < 2.0f)
        e->going_to_b = !e->going_to_b; /* reached waypoint, turn around */
      else
        move_toward(e, tx, ty, delta);
    }
  }
}

void enemies_render(SDL_Renderer *renderer, Camera camera) {
  for (int i = 0; i < enemy_count; i++) {
    Enemy    *e         = &enemies[i];
    SDL_FRect world_rect  = { e->base.x, e->base.y, e->base.w, e->base.h };

    /*
     * camera_project converts world coordinates to screen coordinates.
     * Every entity must do this before rendering — never pass world coords
     * directly to SDL draw calls.
     */
    SDL_FRect sr = camera_project(camera, world_rect); /* sr = screen rect */

    if (e->state == ENEMY_DEAD) {
      /* Dead: draw a small dark blood splat, slightly wider than the body */
      SDL_SetRenderDrawColor(renderer, 60, 10, 10, 255);
      SDL_FRect splat = { sr.x - 1, sr.y + 2, sr.w + 2, sr.h - 2 };
      SDL_RenderFillRect(renderer, &splat);
      continue;
    }

    /* Outline — draw 1 px larger rect first, body covers the inside */
    SDL_SetRenderDrawColor(renderer, 60, 20, 20, 255);
    SDL_FRect outline = { sr.x - 1, sr.y - 1, sr.w + 2, sr.h + 2 };
    SDL_RenderFillRect(renderer, &outline);

    /*
     * Body colour signals intent:
     *   orange-brown → patrolling, unaware
     *   bright red   → chasing the player (dangerous!)
     */
    if (e->state == ENEMY_CHASE)
      SDL_SetRenderDrawColor(renderer, 220, 40, 40, 255);
    else
      SDL_SetRenderDrawColor(renderer, 180, 110, 40, 255);
    SDL_RenderFillRect(renderer, &sr);

    /* Facing pip — same technique as the player */
    SDL_SetRenderDrawColor(renderer, 255, 190, 190, 255);
    float cx = sr.x + sr.w * 0.5f - 1;
    float cy = sr.y + sr.h * 0.5f - 1;
    SDL_FRect pip;
    switch (e->base.facing) {
      case DIR_UP:    pip = (SDL_FRect){ cx,           sr.y + 1,          2, 2 }; break;
      case DIR_DOWN:  pip = (SDL_FRect){ cx,           sr.y + sr.h - 3,   2, 2 }; break;
      case DIR_LEFT:  pip = (SDL_FRect){ sr.x + 1,     cy,                2, 2 }; break;
      case DIR_RIGHT: pip = (SDL_FRect){ sr.x + sr.w - 3, cy,            2, 2 }; break;
      default:        pip = (SDL_FRect){ cx,           cy,                2, 2 }; break;
    }
    SDL_RenderFillRect(renderer, &pip);
  }
}
