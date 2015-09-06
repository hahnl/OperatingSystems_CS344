/* keygen program */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[]) {
  if (argc != 2) { // Make sure there are the correct number of arguments.
    printf("keygen usage:\n { keygen keylength }\n");
    exit(1);
  }
  else {
    srand(time(NULL));
    int length = atoi(argv[1]);
    char key[length];

    // Create each character randomly in range key length:
    int i;
    char curr;
    for (i=0; i<length; i++) {
      curr = "ABCDEFGHIJKLMNOPQRSTUVWXYZ "[random()%27];
      key[i] = curr;
    }
    key[length] = '\0';

    printf("%s\n", key); // Output key.
  }

  return 0;
}
