#include "map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <termios.h>
#include <time.h>
#include <unistd.h>

/*
  INFO:
  1. Map
  2. Player
  3. Enemy
  4. Interaction?
  5.
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

/* ------------------------------- */

typedef struct {
  int x;  // max 20
  int y;  // max 20
  int hp; // max 100
} Entity;

Entity player = {5, 10, 10};
Entity enemy = {5, 11, 10};
char message[50] = "";

time_t currentTime;
struct tm *t;

char date_str[20];
char time_str[20];

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

  strftime(date_str, sizeof(date_str), "%d-%m-%Y", t);
  strftime(time_str, sizeof(time_str), "%H:%M:%S", t);

  printf("Date: %s\n", date_str);
  printf("Time: %s\n", time_str);
}

void app_init() {

  // Set time
  currentTime = time(NULL);
  t = localtime(&currentTime);
  srand(time(NULL));

  enable_raw_mode();
  render();
}

void game_tick() {
  currentTime += 1;
  t = localtime(&currentTime);
}

void app_update() {

  int input = getchar();

  // EOF equal to 4 in ASCII
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

  game_tick();

  if (newX == enemy.x && newY == enemy.y) {

    int random = rand() % 10;

    // TODO: Combat interaction
    if (random < 8) {
      enemy.hp -= 1;
      snprintf(message, sizeof(message), "You hit an enemy! Enemy hp: %d",
               enemy.hp);
    } else {
      player.hp -= 1;
      snprintf(message, sizeof(message), "Enemy hit you! Player hp: %d",
               player.hp);
    }

    if (enemy.hp <= 0) {
      enemy.x = -1;
      enemy.y = -1;
      strcpy(message, "Enemy has died");
    }

    if (player.hp <= 0) {
      strcpy(message, "You have died");
      // TODO: set state game_over
    }
    return;
  }

  player.x = newX;
  player.y = newY;
  message[0] = '\0'; // clear message
}
