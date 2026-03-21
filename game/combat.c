#include "entity.h"
#include <stdio.h>
#include <stdlib.h>

/*
  What are suitable baseline values
  x, y, hp, attack, defense
  Entity player = {5, 10, 10, 1, 1};
  Entity enemy = {5, 11, 10, 1, 2};
*/
int roll(int dice, int sides) {
  int total = 0;
  for (int i = 0; i < dice; i++) {
    total += (rand() % sides) + 1;
  }
  return total;
}

void handle_combat(Entity *attacker, Entity *defender, char *message) {

  int hit_roll = roll(attacker->attack, 20);
  printf("hit_roll %d \n", hit_roll);

  int damage = roll(1, attacker->attack);
  printf("damage %d \n", damage);

  if (hit_roll > defender->defense) {
    defender->hp -= damage;
    snprintf(message, 50, "Hit! Defender HP: %d", defender->hp);
  } else {
    snprintf(message, 50, "Miss!");
  }
}
