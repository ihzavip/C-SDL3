#include <SDL3/SDL_render.h>
#include "player.h"
#include "entity.h"


static Entity player; // private, not exposed

void player_init(void) {
    player.x = 100;
    player.y = 100;
    player.w = 50;
    player.h = 50;
    player.speed = 200;
}

void player_update(float delta) {
    // simple movement (example)
    // player.x += player.speed * delta;
}

void player_render(SDL_Renderer *renderer) {
    SDL_FRect rect = {
        player.x,
        player.y,
        player.w,
        player.h
    };

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &rect);
}
