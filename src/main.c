
#include "game.h"

#define MAXLINE 1000
#define SIZE 10

int main() {
  app_init();

  while (1) {
    app_update();
    render();
  }

  return 0;
}
