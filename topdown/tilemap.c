#include "tilemap.h"
#include "world.h"
#include <SDL3_image/SDL_image.h>

#define TILE_SIZE 16

static SDL_Texture *tileset = NULL;

void tilemap_init(SDL_Renderer *renderer) {
  tileset = IMG_LoadTexture(renderer, "media/Tiles/Background_Green_TileSet.png");
  if (!tileset) {
    SDL_Log("Failed to load tileset: %s", SDL_GetError());
    return;
  }
  SDL_SetTextureBlendMode(tileset, SDL_BLENDMODE_NONE);
}

/* Four floor tile variants from the first row of the sheet.
 * Adjust src_x values to swap in different tiles from the tileset. */
static const float floor_variants[4] = { 0, 16, 32, 48 };

void tilemap_render(SDL_Renderer *renderer, Camera camera) {
  if (!tileset) return;

  /*
   * Only draw tiles that overlap the camera view.
   * Convert camera edges to tile indices, clamp to world bounds.
   */
  int col_start = (int)(camera.x / TILE_SIZE);
  int col_end   = (int)((camera.x + LOGICAL_W) / TILE_SIZE) + 1;
  int row_start = (int)(camera.y / TILE_SIZE);
  int row_end   = (int)((camera.y + LOGICAL_H) / TILE_SIZE) + 1;

  if (col_start < 0)                    col_start = 0;
  if (row_start < 0)                    row_start = 0;
  if (col_end   > WORLD_W / TILE_SIZE)  col_end   = WORLD_W / TILE_SIZE;
  if (row_end   > WORLD_H / TILE_SIZE)  row_end   = WORLD_H / TILE_SIZE;

  for (int row = row_start; row < row_end; row++) {
    for (int col = col_start; col < col_end; col++) {
      /* Deterministic hash — same tile every frame, no flickering */
      int variant = (col * 3 + row * 7) % 4;
      SDL_FRect src = { floor_variants[variant], 0, TILE_SIZE, TILE_SIZE };
      SDL_FRect dst = {
        (float)(col * TILE_SIZE) - camera.x,
        (float)(row * TILE_SIZE) - camera.y,
        TILE_SIZE,
        TILE_SIZE
      };
      SDL_RenderTexture(renderer, tileset, &src, &dst);
    }
  }
}

void tilemap_destroy(void) {
  if (tileset) {
    SDL_DestroyTexture(tileset);
    tileset = NULL;
  }
}
