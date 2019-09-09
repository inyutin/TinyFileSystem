#include "define.h"

#include "Filesystem/commands.h"
#include "daemonize.c"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static size_t max_length = MAX_COMMAND_LENGTH;

void help() {
    printf("Possible commands:\n");
    printf("! ls - show files and directories in current directory.\n");
    printf("! cd [path] - change the current directory to the one specified.\n");
    printf("! mkdir [name] - create empty directory with specified name in current directory.\n");
    printf("! touch [name] - create empty file with specified name in current directory.\n");
    printf("! cat [name] - read all from file specified.\n");
    printf("! echo [name] [text] - write [text] to the end of the file specified.\n");
    printf("! help - show information about commands.");
}

int run_shell() {
    int currentDirId = 0; //root

    help();

    while(1) {
        char command[10];

        char* str = NULL;

        printf("\n$ ");
        int result = getline(&str, &max_length, stdin);

        if (result == -1) {
            printf("getline() error. Exit.");
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
            continue;
        }

        if (strcmp(command, "cd") == 0) {
            char name[MAX_LENGTH_FILE_NAME];
            char* token = strtok(NULL, " ");

            strcpy(name, token);
            if (token == NULL) {
                printf("Specify name of the directory.");
                continue;
            }

            currentDirId = cd_t(currentDirId, name);
            continue;
        }

        if(strcmp(command, "mkdir") == 0){
            char name[MAX_LENGTH_FILE_NAME];
            char* token = strtok(NULL, " ");

            strcpy(name, token);
            if (token == NULL) {
                printf("Specify name of the directory.");
                continue;
            }

            mkdir_t(currentDirId, name);
            continue;
        }

        if (strcmp(command, "touch") == 0) {
            char name[MAX_LENGTH_FILE_NAME];
            char* token = strtok(NULL, " ");

            strcpy(name, token);
            if (token == NULL) {
                printf("Specify name of the file.");
                continue;
            }

            touch_t(currentDirId, name);
            continue;
        }

        if(strcmp(command, "cat") == 0) {
          char name[MAX_LENGTH_FILE_NAME];
          char* token = strtok(NULL, " ");

          strcpy(name, token);
          if (token == NULL) {
              printf("Specify name of the file.");
              continue;
          }

          cat_t(currentDirId, name);
          continue;
        }

        if(strcmp(command, "echo") == 0) {
            char* token = strtok(NULL, " ");
            char name[MAX_LENGTH_FILE_NAME];

            strcpy(name, token);
            if (token == NULL) {
                printf("Specify name of the file.");
                continue;
            }

            char text[MAX_LENGTH_FILE_NAME];
            token = strtok(NULL, " ");
            if (token==NULL) {
                printf("Specify text.");
                continue;
            }
            strcpy(text, token);

            echo_t(currentDirId, name, text);
            continue;
        }

        if(strcmp(command, "help") == 0) {
            help();
            continue;
        }

        if(strcmp(command, "exit") == 0) {
            printf("Shell exited.");
            exit(0);
            return 0;
        }

        printf("Unknown command %s", command);
  }
}

int main() {
    skeleton_daemon();

    while (1) {
        syslog (LOG_NOTICE, "FS daemon started.");
        int code = run_shell();
        syslog (LOG_NOTICE, "FS daemon terminated with error %d", code)
        return EXIT_SUCCESS;
    }
}
