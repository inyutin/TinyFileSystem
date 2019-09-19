#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

#include "define.h"
#include "Filesystem/commands.h"

static size_t max_length = MAX_COMMAND_LENGTH;

static size_t max_message_length = MAX_MESSAGE_LENGTH;

int run_shell(char str[MAX_MESSAGE_LENGTH], int currentDirId, int sock) {
    char command[10];

    if (str[0] == '\n') {
        command[0] = '\0';
    } else {
        char* token = strtok(str, " ");
        strcpy(command, token);
    }

    if (strcmp(command, "exit") == 0) {
        close(sock);
        return currentDirId;
    }

    if (strcmp(command, "ls") == 0) {
        ls_t(currentDirId, sock);
        return currentDirId;
    }

    if (strcmp(command, "cd") == 0) {
        char name[MAX_LENGTH_FILE_NAME];
        char* token = strtok(NULL, " ");

        strcpy(name, token);
        if (token == NULL) {
            printf("Specify name of the directory.");
            return currentDirId;
        }

        return cd_t(currentDirId, name, sock);
    }

    if(strcmp(command, "mkdir") == 0){
        char name[MAX_LENGTH_FILE_NAME];
        char* token = strtok(NULL, " ");

        strcpy(name, token);
        if (token == NULL) {
            printf("Specify name of the directory.");
            return currentDirId;
        }

        mkdir_t(currentDirId, name, sock);
        return currentDirId;
    }

    if (strcmp(command, "touch") == 0) {
        char name[MAX_LENGTH_FILE_NAME];
        char* token = strtok(NULL, " ");

        strcpy(name, token);
        if (token == NULL) {
            printf("Specify name of the file.");
            return currentDirId;
        }

        touch_t(currentDirId, name, sock);
        return currentDirId;
    }

    if (strcmp(command, "cat") == 0) {
        char name[MAX_LENGTH_FILE_NAME];
        char* token = strtok(NULL, " ");

        strcpy(name, token);
        if (token == NULL) {
            printf("Specify name of the file.");
            return currentDirId;
        }

        cat_t(currentDirId, name, sock);
        return currentDirId;
    }

    if (strcmp(command, "echo") == 0) {
        char* token = strtok(NULL, " ");
        char name[MAX_LENGTH_FILE_NAME];

        strcpy(name, token);
        if (token == NULL) {
            printf("Specify name of the file.");
            return currentDirId;
        }

        char text[MAX_LENGTH_FILE_NAME];
        token = strtok(NULL, " ");
        if (token==NULL) {
            printf("Specify text.");
            return currentDirId;
        }
        strcpy(text, token);

        echo_t(currentDirId, name, text, sock);
        return currentDirId;
    }

    char* tmp = "Unknown command ";
    int len = strlen(tmp) + strlen(command) + 1;
    char *result = malloc(len);
    strcpy(result, tmp);
    strcat(result, command);
    send(sock, result, len, 0);
}

int main(int argc, char const *argv[]) {
    // Creating socket file descriptor
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        printf("error");
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        printf("error");
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr*) &address, sizeof(address)) < 0) {
        printf("error");
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        printf("error");
        perror("listen");
        exit(EXIT_FAILURE);
    }

    int address_len = sizeof(address);
    int currentDirId = 0; //root

    daemon(1, 1);

    while (1) {
        int new_socket = accept(server_fd, (struct sockaddr*) &address, (socklen_t*) &address_len);
        if (new_socket < 0) {
            printf("error");
            perror("accept");
            exit(EXIT_FAILURE);
        }

        char buffer[MAX_MESSAGE_LENGTH] = {0};
        int command = read(new_socket, buffer, 1024);
        currentDirId = run_shell(buffer, currentDirId, new_socket);
        close(new_socket);
    }

}