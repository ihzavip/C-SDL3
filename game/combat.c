
#include "entity.h"
#include <stdio.h>
#include <stdlib.h>

void handle_combat(Entity *attacker, Entity *defender, char *message) {

    int roll = rand() % 20 + 1;

    if (roll + attacker->attack > defender->defense) {
        defender->hp -= attacker->attack;
        snprintf(message, 50, "Hit! Defender HP: %d", defender->hp);
    } else {
        snprintf(message, 50, "Miss!");
    }

}
