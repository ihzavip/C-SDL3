
#include "entity.h"
#include <stdio.h>
#include <stdlib.h>

int roll(int dice, int sides) {
    int total = 0;
    for (int i = 0; i < dice; i++) {
        // 0 <= r < b
        total += (rand() % sides) + 1;
    }
    return total;
}

void handle_combat(Entity *attacker, Entity *defender, char *message) {

    int hit_roll = roll(attacker->attack, 6);
    printf("hit_roll %d \n", hit_roll);

    if (hit_roll + attacker->attack > defender->defense) {
        defender->hp -= attacker->attack;
        snprintf(message, 50, "Hit! Defender HP: %d", defender->hp);
    } else {
        snprintf(message, 50, "Miss!");
    }

}
