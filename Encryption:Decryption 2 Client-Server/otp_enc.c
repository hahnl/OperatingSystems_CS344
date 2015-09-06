/* otp_enc program */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

int main(int argc, char *argv[]) {
  if (argc < 4) { // Make sure there are the correct number of arguments.
    fprintf(stderr, "otp_enc usage:\n { otp_enc plaintext key port }\n");
    exit(1);
  }

  // Open socket:
  int port_num = atoi(argv[3]);
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd < 0) {
    fprintf(stderr, "Error: Unable to open socket.\n");
    exit(1);
  }

  struct sockaddr_in server_add;
  bzero((char*)&server_add, sizeof(server_add));
  server_add.sin_family = AF_INET;
  server_add.sin_port = htons(port_num);
  server_add.sin_addr.s_addr = htonl(INADDR_ANY);

  // Connect to port:
  if (connect(sock_fd, (struct sockaddr*)&server_add, sizeof(server_add)) < 0) {
    fprintf(stderr, "Error: could not contact otp_enc_d on port %d\n", port_num);
    exit(2);
  }

  // Write sentinel for signal identification:
  int size = 1024;
  char buff[size];
  char sentinel[size];
  sentinel[1] = '^';
  snprintf(buff, size, "%c", sentinel[1]);
  int b = write(sock_fd, buff, strlen(buff));
  if (b < 0) {
    fprintf(stderr, "Error: Unable to write plaintext filename to socket.\n");
    exit(1);
  }
  sleep(1);

  // Write plaintext filename:
  snprintf(buff, size, "%s", argv[1]);
  b = write(sock_fd, buff, strlen(buff));
  if (b < 0) {
    fprintf(stderr, "Error: Unable to write plaintext filename to socket.\n");
    exit(1);
  }
  sleep(1);

  // Write key filename:
  snprintf(buff, size, "%s", argv[2]);
  b = write(sock_fd, buff, strlen(buff));
  if (b < 0) {
    fprintf(stderr, "Error: Unable to write key filename to socket.\n");
    exit(1);
  }

  // Get server response:
  bzero(buff, size);
  b = read(sock_fd, buff, size);
  if (b < 0) {
    fprintf(stderr, "Error: could not contact otp_end_d on port %d", port_num);
    exit(2);
  }

  if (strcmp(buff,"error_1") == 0) {
    close(STDOUT_FILENO); // Trying to keep it from creating a file
    fprintf(stderr, "Error: key '%s' is too short\n", argv[2]);
    exit(1);
  }
  else if (strcmp(buff,"error_2") == 0) {
    close(STDOUT_FILENO); // No file creation hopefully
    fprintf(stderr, "otp_enc_d error: input contains bad characters\n");
    exit(1);
  }
  else if (strcmp(buff,"error_3") == 0) {
    close(STDOUT_FILENO); // No FILE
    fprintf(stderr, "otp_enc error: unable to connect to otp_enc_d on port %d\n", port_num);
  }
  else {
    printf("%s\n", buff);
  }

  // Close socket:
  close(sock_fd);

  return 0;
}
