/* otp_enc_d program */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>

// Function Prototypes:
int valid_input(char character); // Checks for bad characters.
int numerical_conversion(char character); // Converts character to number.

int main(int argc, char *argv[]) {
  if (argc < 2) { // Make sure there are the correct number of arguments.
    fprintf(stderr, "otp_enc_d usage:\n { otp_enc_d listening port }\n");
    exit(1);
  }

  // Create new child process, exit parent:
  pid_t pid = fork();
  if (pid < 0) {
    exit(EXIT_FAILURE);
  }
  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }
  umask(0); // Allow read/write.

  // Set up session ID for child process:
  pid_t sid = setsid();
  if (sid < 0) {
    exit(EXIT_FAILURE);
  }

  // Strip access to terminal:
  close(STDOUT_FILENO);
  close(STDIN_FILENO);

  // Open socket:
  int port_num = atoi(argv[1]);
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd < 0) {
    fprintf(stderr, "Error: Unable to open socket.\n");
    exit(1);
  }

  struct sockaddr_in server_add;
  struct sockaddr_in client_add;
  bzero((char*)&server_add, sizeof(server_add));
  server_add.sin_family = AF_INET;
  server_add.sin_port = htons(port_num);
  server_add.sin_addr.s_addr = htonl(INADDR_ANY);

  // Bind server to socket:
  if (bind(sock_fd, (struct sockaddr*)&server_add, sizeof(server_add)) < 0) {
    fprintf(stderr, "Error: Unable to bind server to socket.\n");
    exit(1);
  }

  // Listen for connections:
  listen(sock_fd, 5); // Allow FIVE connections.
  socklen_t client_add_length = sizeof(client_add);
  int accept_fd;
  while (1) {
    accept_fd = accept(sock_fd, (struct sockaddr*)&client_add, &client_add_length);
    if (accept_fd < 0) {
      fprintf(stderr, "Error: Unable to accept.\n");
      exit(1);
    }

    //Determine if correct signal:
    int size = 1024;
    char send[size];
    bzero(send, size);
    char receive[size];
    bzero(receive, size);
    int b = read(accept_fd, receive, size);
    if (b < 0) {

       exit(1);
    }
    printf("%c",receive[0]);
    if (receive[0] != '^') {
      snprintf(send, size, "%s", "error_3");
      b = write(accept_fd, send, strlen(send)); // Send error to client.
    	if (b < 0) {
        fprintf(stderr, "Error: Unable to read from socket.\n");
    	  exit(1);
    	}
      continue; // Keep waiting!
    }

    // Receive plaintext:
    bzero(receive, size);
    b = read(accept_fd, receive, size);
    if (b < 0) {
      fprintf(stderr, "Error: Unable to read from socket.\n");
      exit(1);
    }

    // Open plaintext:
    FILE *f;
    f = fopen(receive, "r");
    if (f == NULL) {
      fprintf(stderr, "Error: Unable to open plaintext.\n");
      exit(1);
    }

    // Convert plaintext to uppercase character:
    char plaintext[size];
    fgets(plaintext, size, f);
    int i;
    for (i=0; i<strlen(plaintext); i++) {
      plaintext[i] = toupper(plaintext[i]);
    }
    plaintext[(strlen(plaintext))+1] = '\0';

    // Return error on bad character:
    bzero(send, size);
    for (i=0; i<(strlen(plaintext)-1); i++) {
      if (valid_input(plaintext[i]) == 1) {
	snprintf(send, size, "%s", "error_2");
        b = write(accept_fd, send, strlen(send)); // Send error to client.
	if (b < 0) {
	  fprintf(stderr, "Error: Unable to write to socket.\n");
	  exit(1);
	}
	break;
      }
      continue;
    }
    fclose(f);

    // Receive key:
    bzero(receive, size);
    b = read(accept_fd, receive, size);
    if (b < 0) {
      fprintf(stderr, "Error: Unable to read from socket.\n");
      exit(1);
    }

    // Open key:
    f = fopen(receive, "r");
    if (f == NULL) {
      fprintf(stderr, "Error: Unable to open key.\n");
      exit(1);
    }

    // Store key:
    char key[size];
    fgets(key, size, f);
    fclose(f);

    // Make sure key is as long as plaintext:
    if (strlen(key) < strlen(plaintext)) {
      bzero(send, size);
      snprintf(send, size, "%s", "error_1");
      b = write(accept_fd, send, strlen(send)); // Send error to client.
      if (b < 0) {
        fprintf(stderr, "Error: Unable to write to socket.\n");
        exit(1);
      }
      continue; // Keep waiting!
    }

    // Begin Encryption:
    char characters[28] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    char ciphertext[strlen(plaintext)];
    int key_num[strlen(plaintext)];
    int plaintext_num[strlen(plaintext)];
    int cipher_num[strlen(plaintext)];

    // For each character of key, plaintext, ciphertest, convert to numerical value:
    for (i=0; i<strlen(plaintext); i++) {
      key_num[i] = numerical_conversion(key[i]);
      plaintext_num[i] = numerical_conversion(plaintext[i]);
      cipher_num[i] = plaintext_num[i] + key_num[i]; // Ciphertext character is sum of key and plaintext.
      if (cipher_num[i] > 26) { // Modular addition.
        cipher_num[i] -= 27;
      }
      ciphertext[i] = characters[cipher_num[i]]; // Generate ciphertext.
    }
    ciphertext[strlen(plaintext)-1] = '\0';

    // Send ciphertext back to client:
    bzero(send, size);
    snprintf(send, size, "%s", ciphertext);
    b = write(accept_fd, send, strlen(send));
    if (b < 0) {
      fprintf(stderr, "Error: Unable to write to socket.\n");
      exit(1);
    }
  }

  return 0;
}

int valid_input(char character) {
  switch (character) {
    case 'A':
      return 0;
    case 'B':
      return 0;
    case 'C':
      return 0;
    case 'D':
      return 0;
    case 'E':
      return 0;
    case 'F':
      return 0;
    case 'G':
      return 0;
    case 'H':
      return 0;
    case 'I':
      return 0;
    case 'J':
      return 0;
    case 'K':
      return 0;
    case 'L':
      return 0;
    case 'M':
      return 0;
    case 'N':
      return 0;
    case 'O':
      return 0;
    case 'P':
      return 0;
    case 'Q':
      return 0;
    case 'R':
      return 0;
    case 'S':
      return 0;
    case 'T':
      return 0;
    case 'U':
      return 0;
    case 'V':
      return 0;
    case 'W':
      return 0;
    case 'X':
      return 0;
    case 'Y':
      return 0;
    case 'Z':
      return 0;
    case ' ':
      return 0;
    default:
      return 1;
  }
}

int numerical_conversion(char character) {
  switch (character) {
    case 'A':
      return 0;
    case 'B':
      return 1;
    case 'C':
      return 2;
    case 'D':
      return 3;
    case 'E':
      return 4;
    case 'F':
      return 5;
    case 'G':
      return 6;
    case 'H':
      return 7;
    case 'I':
      return 8;
    case 'J':
      return 9;
    case 'K':
      return 10;
    case 'L':
      return 11;
    case 'M':
      return 12;
    case 'N':
      return 13;
    case 'O':
      return 14;
    case 'P':
      return 15;
    case 'Q':
      return 16;
    case 'R':
      return 17;
    case 'S':
      return 18;
    case 'T':
      return 19;
    case 'U':
      return 20;
    case 'V':
      return 21;
    case 'W':
      return 22;
    case 'X':
      return 23;
    case 'Y':
      return 24;
    case 'Z':
      return 25;
    case ' ':
      return 26;
  }
}
