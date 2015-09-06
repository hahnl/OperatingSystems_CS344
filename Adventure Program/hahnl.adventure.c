/* Author: Larissa Hahn
** Course: CS344 - Program #2
** Filename: hahnl.adventure.c
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>

char *createDir(int pid);
void shuffleRooms(char **arr, size_t x);
struct Location fillRooms(char *roomDir);
void interface(struct Location userLoc);

struct Location {
  char *path;
  char *start;
  char *end;
};

int main(void) {
  srand(time(NULL));
  printf("\n");

  int pid = getpid();
  char *roomDir = createDir(pid);

  //Fill Room Files:
  struct Location userLoc = fillRooms(roomDir);

  //User Interface Presented:
  interface(userLoc);

  free(roomDir);

  return 0;
}


char *createDir(int pid) {
  int size = 20;
  char *dirName = malloc(size);
  char *name = "hahnl.rooms.";

  //Generate Directory Name:
  snprintf(dirName, size, "%s%d", name, pid);

  //Make Directory Using that Name:
  struct stat st = {0};
  if (stat(dirName, &st) == -1) {
    mkdir(dirName, 0755);
  }

  return dirName;
}


void shuffleRooms(char **arr, size_t x) {
  struct timeval val;
  gettimeofday(&val, NULL);
  int value = val.tv_usec;
  srand48(value);

  if (x>1) {
    size_t i;
    for (i=x-1; i>0; i--) {
      size_t j = (unsigned int)(drand48()*(i+1));
      char* v = arr[j];  //Swapping positions here
      arr[j] = arr[i];   //in the room array for randomness
      arr[i] = v;
    }
  }
}


struct Location fillRooms(char *roomDir) {
  struct Location loc;
  loc.path = roomDir;
  char *room[10];

  //Hardcoded Room Names:
  room[0] = "Dungeon";
  room[1] = "Bathroom";
  room[2] = "Garage";
  room[3] = "Attic";
  room[4] = "Bedroom";
  room[5] = "Library";
  room[6] = "Closet";
  room[7] = "Kitchen";
  room[8] = "Balcony";
  room[9] = "Den";

  //Random Order for Room Names:
  shuffleRooms(room, 10);

  //Create Room Files:
  int size = 128;
  char *file = malloc(size);
  int i;
  for (i=0; i<7; i++) {
    snprintf(file, size, "%s/%s", roomDir, room[i]);
    FILE *f = fopen(file, "w");
    if (f == NULL) {
      perror("Unable to open.\n");
      exit(1);
    }
    else {
      //Add Room Name to Files:
      fprintf(f, "ROOM NAME: %s\n", room[i]);
    }
    fclose(f);
  }

  //Save Room Names for Room Connections:
  char *roomName[7];
  for (i=0;i<7; i++) {
    roomName[i] = room[i];
  }

  //Start and End Rooms:
  int endRoom = rand()%7;
  int startRoom = rand()%7;
  while (startRoom == endRoom) {  //Ensures same room not picked
    endRoom = rand()%7;
  }

  //Room Connections:
  int totalConnects;
  char *connect;
  char currRoom[100];

  //Open Files Again to Add Room Connections:
  for (i=0; i<7; i++) {
    shuffleRooms(roomName, 7);
    snprintf(file, size, "%s/%s", roomDir, room[i]);
    FILE *f = fopen(file, "a");

    if (f == NULL) {
      perror("Unable to open file.\n");
      exit(1);
    }
    else {
      totalConnects = rand()%4+3;

      int k=0;
      int j;
      for (j=0; j<totalConnects; j++) {
        connect = roomName[k];
        if (connect == room[i]) {
          k++;
          connect = roomName[k];
        }
        //Add Room Connections to Room Files:
        fprintf(f, "CONNECTION %d: %s\n", j+1, connect);
        k++;
      }

      //Include Room Types:
      if (i == startRoom) {
        fprintf(f, "ROOM TYPE: START_ROOM\n");
        loc.start = room[i];
      }
      else if (i == endRoom) {
        fprintf(f, "ROOM TYPE: END_ROOM\n");
        loc.end = room[i];
      }
      else {
        fprintf(f, "ROOM TYPE: MID_ROOM\n");
      }
    }

    fclose(f);
  }

  free(file);

  return loc;
}

void interface(struct Location userLoc) {
  char *curr = userLoc.start;
  char *endRoom = userLoc.end;
  char *dir = userLoc.path;
  int stepCount = 0;
  int connectCount = 0;

  char (*steps)[15] = malloc(sizeof *steps * 8);
  char (*contents)[15] = malloc(sizeof *contents * 8);
  char destination[15];
  int size = 128;
  char *file = malloc(size);

  //Read the Room File for Room Connections:
  int c;
  while (!(strcmp(curr, endRoom)) == 0) {
    snprintf(file, size, "%s/%s", dir, curr);
    FILE *f = fopen(file, "r");

    connectCount = 0;

    if (f) {
      while ((c = getc(f)) != EOF) {
        if (c == '\n') {  //For every newline, count the number
          connectCount++; //of room connections
        }
      }
    }

    //Total Number of Connections (minus room name and room type lines):
    connectCount = connectCount-2;

    //Grab the Room Name from Room File:
    char str[20];
    fseek(f, 11, SEEK_SET); //Read after label
    fgets(str, 20, f); //Everything on the line
    int len = strlen(str);
    if (str[len-1] == '\n') { //Remove ending newline so saving room name only
      str[len-1] = 0;
    }
    strcpy(contents[0], str); //Store room name as room file contents

    int i;
    for (i=1; i<=connectCount; i++) {
      fseek(f, 14, SEEK_CUR); //Read after label
      fgets(str, 20, f); //Everything on the line
      len = strlen(str);
      if (str[len-1] == '\n') { //Remove ending newline so saving room name only
        str[len-1] = 0;
      }
      strcpy(contents[i], str); //Store connection as room file contents
    }

    //Display the Room File Contents, Begin User Interaction:
    int correct = 0;
    while (correct == 0) { //While correct input from user
      printf("CURRENT LOCATION: %s\n", contents[0]);
      printf("POSSIBLE CONNECTIONS: ");
    
      for (i=1; i<=connectCount; i++) {
	if (i == connectCount) {
	  printf("%s.\n", contents[i]);
        }
	else {
          printf("%s, ", contents[i]);
        }
      }

      //User Input:
      printf("WHERE TO? >");
      scanf("%s", destination);

      //Was it correct input?
      for (i=1; i<=connectCount; i++) {
	if (strcmp(destination, contents[i]) == 0) {
	  correct = 1;
	  curr = destination; //User moves to new room
	}
      }

      if (correct != 1) {
        printf("\nHUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
      }
    }

    //Add Room to User Path for Steps to Victory:
    printf("\n");
    strcpy(steps[stepCount], curr);
    stepCount++;
    fclose(f);
  }

  printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
  printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", stepCount);

  //Display Steps to Victory:
  int i;
  for (i=0; i<stepCount; i++) {
    printf("%s\n", steps[i]);
  }
  printf("%c\n\n", 37); //End marker (%)

  free(steps);
  free(contents);
  free(file);
}
