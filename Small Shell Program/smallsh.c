#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

// String tokenization deliminators:
#define DELIMS " \t\r\n\a"

// Global variables:
int backPs = 0;
int argsNum = 0;

// Function Prototypes:
char *readCommand(); // Command becomes a string
char **splitCommand(char *command); // Tokenizes command string
int execOther(char **args); // Execute non built in commands (all others but exit, cd, and status)


int main() {
  int calledExit = 0;
  int exitValue = 0;

  do {
    char *command = NULL;
    char **args;
    backPs = 0;
    argsNum = 0;
    int fileDesc;

    // Prompt for each command line:
    printf(": ");

    // Read in command as a string:
    command = readCommand();
 
    // Tokenization of command string:
    args = splitCommand(command);
      
    // Check for background process:
    if (!(strncmp(args[argsNum-1], "&", 1))) {
      backPs = 1;
      args[argsNum-1] = NULL;
    }
 
    // Check for blanks or comments (lines beginning with #):
    if (args[0] == NULL || !(strncmp(args[0], "#", 1))) {
      exitValue = 0;
    }

    // Check for exit command:
    else if (strcmp(args[0], "exit") == 0) {
      calledExit = 1;
      exitValue = 0;
    }

    // Check for cd (change directory) command:
    else if (strcmp(args[0], "cd") == 0) {
      if (args[1]) { // Directory provided
        if (chdir(args[1]) != 0) {
	  printf("No such file or directory\n"); // If there's an error
	  exitValue = 1;
	}
      }
      else { // Default is home directory
        chdir(getenv("HOME"));
	exitValue = 0;
      }
    }

    // Check for status command:
    else if (strcmp(args[0], "status") == 0) {
      printf("exit value %d\n", exitValue);
      exitValue = 0;
    }

    // Check for redirection:
    else if (argsNum == 3 && ((strcmp(args[1], ">") == 0) || (strcmp(args[1], "<") == 0))) {
      int stdin = dup(0);
      int stdout = dup(1);

      // Open the file to write to:
      if (strcmp(args[1], ">") == 0) {
        fileDesc = open(args[2], O_WRONLY|O_CREAT|O_TRUNC, 0644);

	if (fileDesc == -1) {
	  printf("%s: no such file or direcotry\n", args[2]);
	  exitValue = 1;
	}
        else {
	  dup2(fileDesc, 1); // Point stdout to this file
	  args[1] = NULL; // Only save command
	  close(fileDesc); // Clean up
	  exitValue = execOther(args); // Execute command
	}
      }

      // Open the file to read from:
      else if (strcmp(args[1], "<") == 0) {
        fileDesc = open(args[2], O_RDONLY);

	if (fileDesc == -1) {
	  printf("smallsh: cannot open %s for input\n", args[2]);
	  exitValue = 1;
	}
        else {
	  dup2(fileDesc, 0); // Point stdout to this file
	  args[1] = NULL; // Only save command
          close(fileDesc); // Clean up
	  exitValue = execOther(args); // Execute command
	}
      }

      // Restore original stdin and stdout:
      dup2(stdin, 0);
      close(stdin);
      dup2(stdout, 1);
      close(stdout);
    }

    // Check for other commands:
    else {
      exitValue = execOther(args);
    }

    // Free memory allocated:
    free(args);
    free(command);

  } while (!calledExit);

  return 0;
}


char *readCommand() {
  char *command = NULL;
  ssize_t size = 0;

  // Read in command as a string:
  getline(&command, &size, stdin);

  return command;
}


char **splitCommand(char *command) {
  int pos = 0;
  int size = 64;
  char **tokens = malloc(size*sizeof(char*));
  char *token;

  // Get the command:
  token = strtok(command, DELIMS);

  // Get and count any arguments:
  while (token != NULL) {
    tokens[pos] = token;
    pos++;
    argsNum++;

    // If the size allocated is reached, add more space for token:
    if (pos >= size) {
      size += 64;
      tokens = realloc(tokens, size*sizeof(char*));
    }

    // Get the next token:
    token = strtok(NULL, DELIMS);
  }

  // NULL at the end of the command string array:
  tokens[pos] = NULL;

  return tokens;
}


int execOther(char **args) {
  pid_t pid;
  pid_t wpid;
  int value = 0;
  int exitValue = 0;

  // Create child process:
  pid = fork();

  if (pid == 0) {
    // Execute non built in commands:
    if (execvp(args[0], args) == -1) {
      printf("%s: no such file or directory\n", args[0]); // If there's an error executing
      exit(1);
    }
  }

  // Check if error forking:
  else if (pid < 0) {
    perror("smallsh");
  }

  // Parent process waits for child process (no orphans or zombies here):
  else {
    do {
      // Parent waits for child to exit or be killed:
      if (backPs == 0) {
        wpid = waitpid(pid, &value, WUNTRACED);
      }
      else if (backPs == 1) {
        wpid = waitpid(-1, &value, WNOHANG);
      }
    } while (!WIFSIGNALED(value) && !WIFEXITED(value));
  }

  // Background process done, display pid and exit value:
  if (backPs == 1) {
    printf("background pid %d exit value %d\n", pid, exitValue);
  }

  // Parent process sets exit value:
  if (WIFSIGNALED(value) || value != 0) {
    exitValue = 1;
  }

  return exitValue;
}
