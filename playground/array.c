
#include "map.h"
#include <stdio.h>

void print_map() {
  // for (int y = 0; y < HEIGHT; y++) {
  //   printf("%s\n", map[y]);
  // }
  printf("%c\n", map[2][4]);
}

void print_map2() {
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      printf("%c", map[y][x]);
    }
    printf("\n");
  }
}
