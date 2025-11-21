
#include "helper.h"
#include <stdio.h>

int main()
{

  printf("nice\n");
  printf("%d\n", add(1, 2));

#if defined(_WIN32)
  printf("Windows\n");
#elif defined(__linux__)
  printf("Linux\n");
#elif defined(__APPLE__)
  printf("macOS\n");
#else
  printf("Unknown OS\n");
#endif

  // echo_char();
  // echo_char2();
  // count_characters();
  return 0;
}
