
#include "helper.h"
#include <stdio.h>

int add(int a, int b) { return a + b; }

void all_is_integer() {
  int i = 97;
  char c = 97;
  printf("%d, %c\n", i, c);
}

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

    if (c == '\n')
      printf(" <-- this is newline\n");

    printf("%d", c);
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

void count_occurances() {
  int c, i, nwhite, nother;
  int ndigit[10];

  nwhite = nother = 0;

  for (i = 0; i < 10; ++i)
    ndigit[i] = 0; // it gives all the item value of 0

  while ((c = getchar()) != EOF) {
    if (c >= '0' && c <= '9') {
      // Every digit is exactly its value + 48.
      printf("%d, %c\n", c, c);
      ++ndigit[c - '0'];
    } else if (c == ' ' || c == '\n' || c == '\t')
      ++nwhite;
    else
      ++nother;
  }
  printf("digits =");
  for (i = 0; i < 10; ++i)
    printf("%d", ndigit[i]);
  printf(", white space = %d, other = %d\n", nwhite, nother);
}
