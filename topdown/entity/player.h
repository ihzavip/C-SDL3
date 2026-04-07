#ifndef PLAYER_H
#define PLAYER_H

#include <SDL3/SDL.h>

void player_init(void);
void player_update(float delta);
void player_render(SDL_Renderer *renderer);

#endif
