#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIDTH 20
#define HEIGHT 20

  // INFO: +1 for null terminator \0
char map[HEIGHT][WIDTH + 1] = {
  "####################",
  "#..............#...#",
  "#..######......#...#",
  "#..............#...#",
  "#......####........#",
  "#..................#",
  "#..##########......#",
  "#..................#",
  "#......#...........#",
  "#......#...........#",
  "#......#...........#",
  "#......#...........#",
  "#......######......#",
  "#..................#",
  "#..........####....#",
  "#..................#",
  "#..####............#",
  "#..................#",
  "#..................#",
  "####################"
};

typedef struct {
  int x;
  int y;
} Entity;

Entity player;

char message[50] = "";

void render() {
  system("clear");

  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {

      if (x == player.x && y == player.y) {
        printf("%c", '@');
      } else {
        printf("%c", map[y][x]);
      }

    }
    printf("\n");
  }
    printf("\n%s\n", message);
}

void app_init() {
  player.x = 5;
  player.y = 10;
  // Entity player = {1, 1};
  render();
}

void app_update() {
  char input;

  if (scanf(" %c", &input) != 1) {
    printf("Input error or EOF. Exiting.\n");
    exit(0);
  }

  if (input == 'q') {
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

  if (map[newY][newX] != '#') {
    player.x = newX;
    player.y = newY;
    message[0] = '\0';  // clear message
  } else {
    strcpy(message, "You hit a wall");
  }
}
