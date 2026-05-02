#include "enemy.h"
#include "entity.h"
#include "../camera.h"
#include <SDL3_image/SDL_image.h>

#define MAX_ENEMIES       6
#define DETECTION_RADIUS  70.0f  /* world pixels — how close before chasing */

#define WALK_FRAMES       6
#define WALK_DURATION     0.12f
#define ATTACK_FRAMES     4
#define ATTACK_DURATION   0.10f
#define DEATH_FRAMES      6
#define DEATH_DURATION    0.14f
#define SPRITE_HEIGHT_PADDING 2  /* extra px read from sheet to avoid foot clipping */

/* Spritesheets, one per direction (DIR_DOWN=0, DIR_UP=1, DIR_LEFT=2, DIR_RIGHT=3). */
static SDL_Texture *textures_move[DIR_COUNT];
static float        frame_w[DIR_COUNT],     frame_h[DIR_COUNT];

static SDL_Texture *textures_attack[DIR_COUNT];
static float        frame_w_atk[DIR_COUNT], frame_h_atk[DIR_COUNT];

/* Death sheets: only Side and Side-left exist; Down and Up fall back to Side. */
static SDL_Texture *textures_death[DIR_COUNT];
static float        frame_w_dth[DIR_COUNT], frame_h_dth[DIR_COUNT];

typedef enum { ENEMY_ANIM_WALK, ENEMY_ANIM_ATTACK, ENEMY_ANIM_DEATH } EnemyAnimType;

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

  int           hp;
  float         hit_cooldown;    /* red-blink window after taking damage from player */
  float         attack_cooldown; /* locks animation for one full attack cycle */
  bool          attack_can_hit;  /* true until damage is checked at impact frame */
  int           anim_frame;
  float         anim_timer;
  EnemyAnimType anim_type;
} Enemy;

#define ATTACK_IMPACT_FRAME 2  /* frame index at which the hit is checked — player can dodge before this */

#define ENEMY_MAX_HP 3

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
  e->hp              = ENEMY_MAX_HP;
  e->hit_cooldown    = 0.0f;
  e->attack_cooldown = 0.0f;
  e->attack_can_hit  = false;
  e->anim_frame  = 0;
  e->anim_timer  = 0.0f;
  e->anim_type   = ENEMY_ANIM_WALK;
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
/* Returns the rect one body-length ahead of the enemy in its facing direction.
   Used for the impact-frame hit check so push-out doesn't nullify the attack. */
static SDL_FRect enemy_attack_rect(const Enemy *e) {
  float ax = e->base.x, ay = e->base.y;
  switch (e->base.facing) {
    case DIR_UP:    ay -= e->base.h; break;
    case DIR_DOWN:  ay += e->base.h; break;
    case DIR_LEFT:  ax -= e->base.w; break;
    case DIR_RIGHT: ax += e->base.w; break;
    default: break;
  }
  return (SDL_FRect){ ax, ay, e->base.w, e->base.h };
}

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

void enemies_load_textures(SDL_Renderer *renderer) {
  /* Down walk uses lowercase 'walk'; all others use 'Walk'. */
  const char *walk_paths[4] = {
    "media/Enemies/Zombie_Small/Zombie_Small_Down_walk-Sheet6.png",
    "media/Enemies/Zombie_Small/Zombie_Small_Up_Walk-Sheet6.png",
    "media/Enemies/Zombie_Small/Zombie_Small_Side-left_Walk-Sheet6.png",
    "media/Enemies/Zombie_Small/Zombie_Small_Side_Walk-Sheet6.png",
  };
  const char *attack_paths[4] = {
    "media/Enemies/Zombie_Small/Zombie_Small_Down_First-Attack-Sheet4.png",
    "media/Enemies/Zombie_Small/Zombie_Small_Up_First-Attack-Sheet4.png",
    "media/Enemies/Zombie_Small/Zombie_Small_Side-left_First-Attack-Sheet4.png",
    "media/Enemies/Zombie_Small/Zombie_Small_Side_First-Attack-Sheet4.png",
  };
  /* No Down/Up death sheets exist — fall back to Side for those directions. */
  const char *death_paths[4] = {
    "media/Enemies/Zombie_Small/Zombie_Small_Side_First-Death-Sheet6.png",
    "media/Enemies/Zombie_Small/Zombie_Small_Side_First-Death-Sheet6.png",
    "media/Enemies/Zombie_Small/Zombie_Small_Side-left_First-Death-Sheet6.png",
    "media/Enemies/Zombie_Small/Zombie_Small_Side_First-Death-Sheet6.png",
  };

  for (int d = 0; d < DIR_COUNT; d++) {
    textures_move[d] = IMG_LoadTexture(renderer, walk_paths[d]);
    if (textures_move[d]) {
      SDL_SetTextureBlendMode(textures_move[d], SDL_BLENDMODE_BLEND);
      float tw, th;
      SDL_GetTextureSize(textures_move[d], &tw, &th);
      frame_w[d] = tw / WALK_FRAMES;
      frame_h[d] = th;
    } else {
      SDL_Log("Failed to load enemy walk texture '%s': %s", walk_paths[d], SDL_GetError());
    }

    textures_attack[d] = IMG_LoadTexture(renderer, attack_paths[d]);
    if (textures_attack[d]) {
      SDL_SetTextureBlendMode(textures_attack[d], SDL_BLENDMODE_BLEND);
      float tw, th;
      SDL_GetTextureSize(textures_attack[d], &tw, &th);
      frame_w_atk[d] = tw / ATTACK_FRAMES;
      frame_h_atk[d] = th;
    } else {
      SDL_Log("Failed to load enemy attack texture '%s': %s", attack_paths[d], SDL_GetError());
    }

    textures_death[d] = IMG_LoadTexture(renderer, death_paths[d]);
    if (textures_death[d]) {
      SDL_SetTextureBlendMode(textures_death[d], SDL_BLENDMODE_BLEND);
      float tw, th;
      SDL_GetTextureSize(textures_death[d], &tw, &th);
      frame_w_dth[d] = tw / DEATH_FRAMES;
      frame_h_dth[d] = th;
    } else {
      SDL_Log("Failed to load enemy death texture '%s': %s", death_paths[d], SDL_GetError());
    }
  }
}

void enemies_destroy_textures(void) {
  for (int d = 0; d < DIR_COUNT; d++) {
    if (textures_move[d])   { SDL_DestroyTexture(textures_move[d]);   textures_move[d]   = NULL; }
    if (textures_attack[d]) { SDL_DestroyTexture(textures_attack[d]); textures_attack[d] = NULL; }
    if (textures_death[d])  { SDL_DestroyTexture(textures_death[d]);  textures_death[d]  = NULL; }
  }
}

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

void enemies_reset(void) { enemies_init(); }

bool enemies_update(float delta, SDL_FRect player_rect,
                    SDL_FRect attack_rect, bool player_attacking) {
  bool hit_player = false;

  for (int i = 0; i < enemy_count; i++) {
    Enemy *e = &enemies[i];

    /* Dead enemies only need their death animation advanced. */
    if (e->state == ENEMY_DEAD) {
      if (e->anim_frame < DEATH_FRAMES - 1) {
        e->anim_timer += delta;
        if (e->anim_timer >= DEATH_DURATION) {
          e->anim_timer -= DEATH_DURATION;
          e->anim_frame++;
        }
      }
      continue;
    }

    SDL_FRect erect = { e->base.x, e->base.y, e->base.w, e->base.h };

    /* --- Attack hit detection ------------------------------------------- */
    /*
     * If the player is swinging this frame AND the attack rect overlaps this
     * enemy's rect, the enemy dies. One-hit kill, Hotline Miami style.
     */
    if (e->hit_cooldown > 0.0f) e->hit_cooldown -= delta;

    if (player_attacking && rects_overlap(attack_rect, erect) && e->hit_cooldown <= 0.0f) {
      e->hp--;
      e->hit_cooldown = 0.4f;
      if (e->hp <= 0) {
        e->state      = ENEMY_DEAD;
        e->anim_type  = ENEMY_ANIM_DEATH;
        e->anim_frame = 0;
        e->anim_timer = 0.0f;
      }
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

    if (e->attack_cooldown > 0.0f) e->attack_cooldown -= delta;

    /* --- Movement: freeze while attacking -------------------------------- */
    if (e->attack_cooldown <= 0.0f) {
      if (e->state == ENEMY_CHASE) {
        float tx = player_rect.x + player_rect.w * 0.5f - e->base.w * 0.5f;
        float ty = player_rect.y + player_rect.h * 0.5f - e->base.h * 0.5f;
        move_toward(e, tx, ty, delta);
      } else {
        float tx = e->going_to_b ? e->patrol_bx : e->patrol_ax;
        float ty = e->going_to_b ? e->patrol_by : e->patrol_ay;
        float dx = tx - e->base.x;
        float dy = ty - e->base.y;
        float dist = SDL_sqrtf(dx * dx + dy * dy);
        if (dist < 2.0f)
          e->going_to_b = !e->going_to_b;
        else
          move_toward(e, tx, ty, delta);
      }
    }

    /* --- Contact: push-out + start attack -------------------------------- */
    SDL_FRect post     = { e->base.x, e->base.y, e->base.w, e->base.h };
    bool      touching = rects_overlap(post, player_rect);

    if (touching) {
      float ox = SDL_min(post.x + post.w, player_rect.x + player_rect.w)
               - SDL_max(post.x,          player_rect.x);
      float oy = SDL_min(post.y + post.h, player_rect.y + player_rect.h)
               - SDL_max(post.y,          player_rect.y);
      if (ox < oy)
        e->base.x += (post.x < player_rect.x) ? -ox : ox;
      else
        e->base.y += (post.y < player_rect.y) ? -oy : oy;

      /*
       * Fire damage + start attack animation only once per cooldown cycle.
       * attack_cooldown lasts the full animation duration so the next hit
       * can't land until the current swing completes.
       */
      if (e->state == ENEMY_CHASE && e->attack_cooldown <= 0.0f) {
        e->attack_cooldown = ATTACK_FRAMES * ATTACK_DURATION;
        e->attack_can_hit  = true;  /* damage fires at impact frame, not here */
        e->anim_type       = ENEMY_ANIM_ATTACK;
        e->anim_frame      = 0;
        e->anim_timer      = 0.0f;
      }
    }

    /* --- Animation -------------------------------------------------------- */
    if (e->attack_cooldown <= 0.0f && e->anim_type == ENEMY_ANIM_ATTACK) {
      e->anim_type      = ENEMY_ANIM_WALK;
      e->anim_frame     = 0;
      e->anim_timer     = 0.0f;
      e->attack_can_hit = false;
    }

    float dur    = (e->anim_type == ENEMY_ANIM_ATTACK) ? ATTACK_DURATION : WALK_DURATION;
    int   frames = (e->anim_type == ENEMY_ANIM_ATTACK) ? ATTACK_FRAMES   : WALK_FRAMES;

    e->anim_timer += delta;
    if (e->anim_timer >= dur) {
      e->anim_timer -= dur;
      int new_frame  = (e->anim_frame + 1) % frames;
      e->anim_frame  = new_frame;

      /* Impact frame: check if player is still in range. */
      if (e->anim_type == ENEMY_ANIM_ATTACK &&
          new_frame == ATTACK_IMPACT_FRAME   &&
          e->attack_can_hit) {
        if (rects_overlap(enemy_attack_rect(e), player_rect)) hit_player = true;
        e->attack_can_hit = false;
      }
    }
  }

  return hit_player;
}

void enemies_render(SDL_Renderer *renderer, Camera camera) {
  for (int i = 0; i < enemy_count; i++) {
    Enemy    *e  = &enemies[i];
    SDL_FRect phys = { e->base.x, e->base.y, e->base.w, e->base.h };
    SDL_FRect sr   = camera_project(camera, phys);

    if (e->state == ENEMY_DEAD) {
      int          dir  = (int)e->base.facing;
      SDL_Texture *dtex = textures_death[dir];
      float        dfw  = frame_w_dth[dir];
      float        dfh  = frame_h_dth[dir];
      if (dtex) {
        SDL_FRect src = { e->anim_frame * dfw, 0, dfw, dfh };
        SDL_FRect world_sprite = {
          e->base.x + e->base.w * 0.5f - dfw * 0.5f,
          e->base.y + e->base.h * 0.5f - dfh * 0.5f,
          dfw, dfh
        };
        SDL_FRect dst = camera_project(camera, world_sprite);
        SDL_RenderTexture(renderer, dtex, &src, &dst);
      }
      continue;
    }

    int          dir = (int)e->base.facing;
    bool         atk = (e->anim_type == ENEMY_ANIM_ATTACK);
    SDL_Texture *tex = atk ? textures_attack[dir] : textures_move[dir];
    float        fw  = atk ? frame_w_atk[dir]     : frame_w[dir];
    float        fh  = atk ? frame_h_atk[dir]     : frame_h[dir];

    SDL_FRect src = { e->anim_frame * fw, 0, fw, fh + SPRITE_HEIGHT_PADDING };

    /*
     * Centre the sprite on the physics box so the hitbox stays in the
     * middle of the visible sprite regardless of frame size.
     */
    SDL_FRect world_sprite = {
      e->base.x + e->base.w * 0.5f - fw * 0.5f,
      e->base.y + e->base.h * 0.5f - (fh + SPRITE_HEIGHT_PADDING) * 0.5f,
      fw, fh + SPRITE_HEIGHT_PADDING
    };
    SDL_FRect dst = camera_project(camera, world_sprite);

    if (tex) {
      if (e->hit_cooldown > 0.0f && ((int)(e->hit_cooldown / 0.1f) % 2) == 0)
        SDL_SetTextureColorMod(tex, 255, 80, 80);
      SDL_RenderTexture(renderer, tex, &src, &dst);
      SDL_SetTextureColorMod(tex, 255, 255, 255);
      /* DEBUG: green = sprite frame, red = physics box */
      // SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
      // SDL_RenderRect(renderer, &dst);
      // SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
      // SDL_RenderRect(renderer, &sr);
    } else {
      /* Fallback colored rect if texture failed to load */
      SDL_SetRenderDrawColor(renderer,
        e->state == ENEMY_CHASE ? 220 : 180,
        e->state == ENEMY_CHASE ?  40 : 110,
        40, 255);
      SDL_RenderFillRect(renderer, &sr);
    }
  }
}
