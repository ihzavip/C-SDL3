#include "map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <termios.h>
#include <unistd.h>

/*
  INFO:
  1. Map
  2. Player
  3. Enemy
  4. Interaction?
*/

struct termios orig_termios;

void disable_raw_mode() { tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios); }

void enable_raw_mode() {
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disable_raw_mode);

  struct termios raw = orig_termios;
  raw.c_lflag &= ~(ICANON | ECHO); // disable canonical mode and echo

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

typedef struct {
  int x;  // max 20
  int y;  // max 20
  int hp; // max 100
} Entity;

Entity player = {5, 10, 10};
Entity enemy = {5, 11, 10};
char message[50] = "";

void render() {
  system("clear");

  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {

      if (x == player.x && y == player.y) {
        printf("%c", '@');
      } else if (x == enemy.x && y == enemy.y) {
        printf("%c", '*');
      } else {
        printf("%c", map[y][x]);
      }
    }
    printf("\n");
  }
  printf("%s\n", message);
}

void app_init() {
  enable_raw_mode();
  render();
}

void app_update() {

  int input = getchar();

  if (input == EOF) {
    exit(0);
  }

  if (input == 'q') {
    // TODO: Add prompt for exitting later
    printf("Quitting game.\n");
    exit(0);
  }

  int newX = player.x;
  int newY = player.y;

  if (input == 'k')
    newY--;
  if (input == 'j')
    newY++;
  if (input == 'h')
    newX--;
  if (input == 'l')
    newX++;

  if (newX < 0 || newX >= WIDTH || newY < 0 || newY >= HEIGHT) {
    strcpy(message, "Out of bounds!");
    return;
  }

  if (map[newY][newX] == '#') {
    strcpy(message, "You hit a wall");
    // is it the same as message = "You hit a wall"
    return;
  }

  // TODO: Combat interaction
  if (newX == enemy.x && newY == enemy.y) {
    enemy.hp -= 1;

    // TODO: calculate combat

    snprintf(message, sizeof(message), "You hit an enemy! Enemy hp: %d",
             enemy.hp);

    if (enemy.hp <= 0) {
      enemy.x = -1;
      enemy.y = -1;
      strcpy(message, "Enemy has died");
    }

    return;
  }

  player.x = newX;
  player.y = newY;
  message[0] = '\0'; // clear message
}
