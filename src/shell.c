#include "define.h"

#include "Filesystem/commands.h"
#include <string.h>
#include <stdlib.h>

static size_t max_length = MAX_COMMAND_LENGTH;

void help() {
    printf("Possible commands:\n");
    printf("! ls - show files and directories in current directory\n");
    printf("! cd [path] - change the current directory to the one specified\n");
    printf("! mkdir [name] - create empty directory with specified name in current directory\n");
    printf("! touch [name] - create empty file with specified name in current directory\n");
    printf("! cat [name] - read all from file specified\n");
    printf("! echo [name] [text] - write [text] to the end of the file specified\n");
    printf("! help - show information about commands \n");
    printf("\n$ ");
}

int main() {
    help();

    int currentDirId = 0; //root

    while(1) {
      char command[10];

      char* str = NULL;
      int result = getline(&str, &max_length, stdin);

      if (result == -1) {
          printf("getline() error. Exit.\n");
          return -1;
      } else if (str[0] == '\n') {
          command[0] = '\0';
      } else {
          str[result-1] = '\0';
          char* token = strtok(str, " ");
          strcpy(command, token);
      }

      /* Command Received */

      if (strcmp(command, "ls") == 0) {
          ls_t(currentDirId);
          printf("\n$ ");
      } else if(strcmp(command, "cd") == 0){
          char name[MAX_LENGTH_FILE_NAME];
          char* token = strtok(NULL, " ");

          strcpy(name, token);
          if (token == NULL) {
              printf("Incorrect parameter.\n");
              break;
          }

          currentDirId = cd_t(currentDirId, name);
          printf("\n$ ");
      } else if(strcmp(command, "mkdir") == 0){
          char name[MAX_LENGTH_FILE_NAME];
          char* token = strtok(NULL, " ");

          strcpy(name, token);
          if (token == NULL) {
              printf("Incorrect parameter.\n");
              break;
          }

          mkdir_t(currentDirId, name);
          printf("\n$ ");
      } else if(strcmp(command, "touch") == 0){
          char name[MAX_LENGTH_FILE_NAME];
          char* token = strtok(NULL, " ");

          strcpy(name, token);
          if (token == NULL) {
              printf("Incorrect parameter.\n");
              break;
          }

          touch_t(currentDirId, name);
          printf("$ ");
      } else if(strcmp(command, "cat") == 0) {
          char name[MAX_LENGTH_FILE_NAME];
          char* token = strtok(NULL, " ");

          strcpy(name, token);
          if (token == NULL) {
              printf("Incorrect parameter.\n");
              break;
          }

          cat_t(currentDirId, name);
          printf("\n$ ");
      } else if(strcmp(command, "echo") == 0) {
          char* token = strtok(NULL, " ");
          char name[MAX_LENGTH_FILE_NAME];

          strcpy(name, token);
          if (token == NULL) {
              printf("Incorrect parameter.\n");
              break;
          }

          char text[MAX_LENGTH_FILE_NAME];
          token = strtok(NULL, " ");
          if (token==NULL)
          {
              printf("Incorrect parameter.\n");
              break;
          }
          strcpy(text, token);

          echo_t(currentDirId, name, text);
          printf("\n$ ");;
      } else if(strcmp(command, "exit") == 0) {
          new();
      }
      else if(strcmp(command, "exit") == 0) {
              printf("Shell exited.\n");
              exit(0);
              return 0;
          }
      } else{
          printf("Unknown command %s.\n", command);
          printf("\n$ ");
      }
  }
}
