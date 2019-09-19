#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "define.h"
#include "stdlib.h"

static size_t max_message_length = MAX_MESSAGE_LENGTH;

void help() {
    printf("Possible commands:\n");
    printf("! ls - show files and directories in current directory.\n");
    printf("! cd [path] - change the current directory to the one specified.\n");
    printf("! mkdir [name] - create empty directory with specified name in current directory.\n");
    printf("! touch [name] - create empty file with specified name in current directory.\n");
    printf("! cat [name] - read all from file specified.\n");
    printf("! echo [name] [text] - write [text] to the end of the file specified.\n");
    printf("! help - show information about commands.\n");
    printf("! exit - exit client.");
}

int main(int argc, char const *argv[]) {
    help();
    while(1) {
        int sock = 0;
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("\n Socket creation error \n");
            return -1;
        }

        struct sockaddr_in server_address;
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(PORT);

        // Convert IPv4 and IPv6 addresses from text to binary form
        if (inet_pton(AF_INET, SERVER, &server_address.sin_addr) <= 0) {
            printf("Invalid address/ Address not supported \n");
            return -1;
        }

        while (connect(sock, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
            printf("Connection Failed \n");
            sleep(5);
        }

        char* command = NULL;
        char buffer[MAX_MESSAGE_LENGTH] = {0};

        printf("\n$ ");
        int result = getline(&command, &max_message_length, stdin);
        if (result == -1) {
            printf("getline() error. Try again.");
            return -1;
        }

        int len = strlen(command);
        command[len-1] = '\0';

        if(strcmp(command, "exit") == 0) {
            send(sock, command, strlen(command), 0);
            printf("Shell exited.");
            close(sock);
            exit(0);
            return 0;
        }

        send(sock, command, strlen(command), 0);
        int valueRead = read(sock, buffer, 1024);
        printf(buffer);
        close(sock);
    }
}