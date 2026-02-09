
#include "helper.h"
#include <stdio.h>

int add(int a, int b) { return a + b; }

void check_os() {
#if defined(_WIN32)
  printf("Windows\n");
#elif defined(__linux__)
  printf("Linux\n");
#elif defined(__APPLE__)
  printf("macOS\n");
#else
  printf("Unknown OS\n");
#endif
}

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
    // printf("%c\n", c);
    c = getchar();
  }
}

void echo_char2() {
  int c;
  while ((c = getchar()) != EOF) {
    putchar(c);

    if (putchar(c) == 97)
      printf("ini a");

    if (c == '\n')
      printf(" <-- this is newline\n");

    // printf("%c", c);
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
      // '0' is 48
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

int power(int base, int n) {
  int i, p;
  p = 1;

  for (i = 1; i < n; ++i) {
    p = p * base;
  }
  return p;
}

int get_line(char s[], int lim) {
  int c, i;

  for (i = 0; i < lim - 1 && (c = getchar()) != EOF && c != '\n'; ++i) {
    s[i] = c;            // assigning the string to s?
    printf("%s s\n", s); // it's to get all the char
  }

  // if new line or enter assign \0? increment i by 1
  if (c == '\n') {
    s[i] = c;
    ++i;
  }
  s[i] = '\0'; // \0 referring to end of string
  return i;    // return the length of the string (this line)
}

// I don't understand this
void copy(char to[], char from[]) {
  int i;

  i = 0;
  while ((to[i] = from[i]) != '\0')
    ++i;
}

int mutatingFunction(int *a) {
  *a = *a + 2;
  return *a;
}
