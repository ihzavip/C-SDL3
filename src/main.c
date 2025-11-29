
#include "helper.h"
#include <stdio.h>
#define MAXLINE 1000

int main() {

  char s[5] = "nice";
  printf("%s\n", s);

  int len;               /* current line length */
  int max;               /* maximum length seen so far */
  char line[MAXLINE];    /* current input line */
  char longest[MAXLINE]; /* longest line saved here */
  max = 0;
  while ((len = get_line(line, MAXLINE)) > 0)
    if (len > max) {
      max = len;
      copy(longest, line);
    }
  if (max > 0) /* there was a line */
    printf("%s", longest);

  return 0;
}
