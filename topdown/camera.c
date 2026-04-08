#include "camera.h"
#include "world.h"

void camera_follow(Camera *cam, float tx, float ty, float tw, float th) {
  /*
   * Goal: keep the target centred on screen.
   *
   * The centre of the target in world space is:
   *   (tx + tw/2, ty + th/2)
   *
   * For that point to appear at the centre of the screen (LOGICAL_W/2,
   * LOGICAL_H/2), the camera's top-left must be at:
   *   camera.x = target_centre_x - LOGICAL_W / 2
   *   camera.y = target_centre_y - LOGICAL_H / 2
   */
  cam->x = (tx + tw * 0.5f) - LOGICAL_W * 0.5f;
  cam->y = (ty + th * 0.5f) - LOGICAL_H * 0.5f;

  /*
   * Clamp so the camera never scrolls outside the world.
   *
   * Left/top edge: camera cannot go negative (would show void on the left/top).
   * Right/bottom edge: camera.x + LOGICAL_W must not exceed WORLD_W, so
   *   camera.x <= WORLD_W - LOGICAL_W.
   */
  if (cam->x < 0)                    cam->x = 0;
  if (cam->y < 0)                    cam->y = 0;
  if (cam->x > WORLD_W - LOGICAL_W)  cam->x = WORLD_W - LOGICAL_W;
  if (cam->y > WORLD_H - LOGICAL_H)  cam->y = WORLD_H - LOGICAL_H;
}

SDL_FRect camera_project(Camera cam, SDL_FRect world_rect) {
  /*
   * Subtract the camera offset from the world position.
   * Size (w, h) is unchanged — the camera does not scale, only translate.
   */
  return (SDL_FRect){
    world_rect.x - cam.x,
    world_rect.y - cam.y,
    world_rect.w,
    world_rect.h,
  };
}
