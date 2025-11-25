
#include "helper.h"
#include <stdio.h>

int main() {

  printf("%d\n", add(1, 2));
  check_os();
  // count_occurances();

  for (int i = 0; i < 10; i++) {
    printf("%d %d %d\n", i, power(2, i), power(-3, i));
  }

  // int digits[10];
  // for (int i = 0; i < 10; i++) {
  //   digits[i] = i;
  //
  //   for (int i = 0; i < 10; i++) {
  //     printf("%d ", digits[i]);
  //   }
  //   printf("\n");
  // }

  return 0;
}
