
#include "helper.h"
#include <stdio.h>

int add(int a, int b) { return a + b; }

void echo_char() {
  // getchar here just get the integer of what the user type (1 character)
  int c = getchar();

  // EOF == -1
  while (c != EOF) {
    putchar(c);
    printf("%d\n", c);
    c = getchar();
  }
}

void echo_char2() {
  int c;
  while ((c = getchar()) != EOF) {
    putchar(c);
    printf("%d\n", c);
  }
}

void count_characters() {
  long nc = 0;
  int c;
  while ((c = getchar()) != EOF) {
    if (c != '\n') {
      ++nc;
      printf("%ld\n", nc);
    }
  }
}
