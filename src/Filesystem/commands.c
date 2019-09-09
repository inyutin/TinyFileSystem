#include "commands.h"
#include "filesystem.h"

#include "../define.h"

#include "../Types/superblock.h"
#include "../Types/inode.h"
#include "../Types/mapping.h"

#include "../SystemOperations/open.h"
#include "../SystemOperations/read.h"
#include "../SystemOperations/write.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>

void ls_t (int currentDirId, int sock) {
    struct Inode currentDir = getInode(currentDirId);
    int offset = DATA_OFFSET + currentDir.direct[0] * BLOCK_SIZE;
    char *result = "";
    for (int i = 0; i < currentDir.numOfFiles; i++) {
        struct Mapping mapping;
        readFromFilesystem(offset + i*sizeof(struct Mapping),
                           (void*)& mapping, sizeof(struct Mapping));

        struct Inode tmp = getInode(mapping.id);
        if (tmp.numOfFiles == 0) {
            char* left = " \033[22;34m ";
            char* right = " \033[0m";
            int len = strlen(result) + strlen(left) + strlen(mapping.name) + strlen(right) + 1;
            char* new_result = malloc(len);
            strcpy(new_result, result);
            strcat(new_result, left);
            strcat(new_result, mapping.name);
            strcat(new_result, right);
            result = new_result;
        } else {
            char* left = "\033[22;34m ";
            char* right = "/ \033[0m";
            int len = strlen(result) + strlen(left) + strlen(mapping.name) + strlen(right) + 1;
            char* new_result = malloc(len);
            strcpy(new_result, result);
            strcat(new_result, left);
            strcat(new_result, mapping.name);
            strcat(new_result, right);
            result = new_result;
        }
    }
    send(sock, result, strlen(result), 0);
}

int cd_t (int currentDirId, char name[MAX_LENGTH_FILE_NAME], int sock) {
    if (strcmp(name, "/") == 0) {
        char* result = "Current directory is root";
        send(sock, result, strlen(result), 0);
        return 0;
    }

    if (name[0] == '/') {
        currentDirId = 0;
    }

    int temp = open_t(currentDirId, name);
    if (temp == -1) {
        char* result = "Can't find path. Please try again.";
        send(sock, result, strlen(result), 0);
        return currentDirId;
    } else {
        if (getInode(temp).numOfFiles == 0) {
            char* result = "This is a file, not a directory. Current directory does not change. Try again please.";
            send(sock, result, strlen(result), 0);
            return currentDirId;
        }
        char* left = "Current directory is ";
        int len = strlen(left) + strlen(name) + 1;
        char *result = malloc(len);
        strcpy(result, left);
        strcat(result, name);
        send(sock, result, len, 0);
        return temp;
    }
}

void mkdir_t(int currentDirId, char name[MAX_LENGTH_FILE_NAME], int sock) {
    //Check if there exist a file with the same name
    struct Inode currentDir = getInode(currentDirId);
    int offset = DATA_OFFSET + currentDir.direct[0] * BLOCK_SIZE;
    for (int i = 0; i < currentDir.numOfFiles; i++) {
        struct Mapping mapping;
        readFromFilesystem(offset + (i*sizeof(struct Mapping)),
                           (void*)& mapping, sizeof(struct Mapping));

        if(strcmp(name,mapping.name)==0) {
            char* result = "A directory with the same name exists.";
            send(sock, result, strlen(result), 0);
            return;
        }
    }

    // Get next free inode and block
    struct Superblock superblock = getSuperblock();
    int nextAvailableInode = superblock.nextAvailableInode;
    int nextAvailableBlock = superblock.nextAvailableBlock;

    // Create inode for new directory
    createInode(nextAvailableInode, currentDirId, nextAvailableBlock, 1);

    // Add new mapping for parent directory
    struct Mapping mapping = createMapping(name, nextAvailableInode);
    offset = DATA_OFFSET + currentDir.direct[0] * BLOCK_SIZE;
    writeToFilesystem (offset + currentDir.numOfFiles * sizeof(struct Mapping),
                       (void*)& mapping, sizeof(struct Mapping));

    // Update number of sons of parent dir
    currentDir.numOfFiles = currentDir.numOfFiles+1;
    writeToFilesystem (INODE_OFFSET+currentDirId*sizeof(struct Inode),
                       (void*)& currentDir, sizeof(struct Inode));

    // Finally update superblock
    superblock.nextAvailableInode = nextAvailableInode + 1;
    superblock.nextAvailableBlock = nextAvailableBlock + 1;
    writeToFilesystem(SB_OFFSET, (void*)& superblock, sizeof(struct Superblock));

    char* left = "Directory ";
    char* right = " was created";
    int len = strlen(left) + strlen(name) + strlen(right) + 1;
    char *result = malloc(len);
    strcpy(result, left);
    strcat(result, mapping.name);
    strcat(result, right);
    send(sock, result, len, 0);
}

void touch_t(int currentDirId, char name[MAX_LENGTH_FILE_NAME], int sock) {
    //Check if there exist a file with the same name
    struct Inode currentDir = getInode(currentDirId);
    int offset = DATA_OFFSET + currentDir.direct[0] * BLOCK_SIZE;
    for (int i = 0; i < currentDir.numOfFiles; i++) {
        struct Mapping mapping;
        readFromFilesystem(offset + (i*sizeof(struct Mapping)),
                           (void*)& mapping, sizeof(struct Mapping));

        if(strcmp(name, mapping.name) == 0) {
            char* result = "A file with the same name exists.";
            send(sock, result, strlen(result), 0);
            return;
        }
    }

    // Get next free inode and block
    struct Superblock superblock = getSuperblock();
    int nextAvailableInode = superblock.nextAvailableInode;
    int nextAvailableBlock = superblock.nextAvailableBlock;

    // Create inode for new directory
    createInode(nextAvailableInode, currentDirId, nextAvailableBlock, 0);

    // Add new mapping for parent directory
    struct Mapping mapping = createMapping(name, nextAvailableInode);
    offset = DATA_OFFSET + currentDir.direct[0] * BLOCK_SIZE;
    writeToFilesystem (offset + currentDir.numOfFiles * sizeof(struct Mapping),
                       (void*)& mapping, sizeof(struct Mapping));

    // Update number of sons of parent dir
    currentDir.numOfFiles = currentDir.numOfFiles+1;
    writeToFilesystem (INODE_OFFSET+currentDirId*sizeof(struct Inode),
                       (void*)& currentDir, sizeof(struct Inode));

    // Finally update superblock
    superblock.nextAvailableInode = nextAvailableInode + 1;
    superblock.nextAvailableBlock = nextAvailableBlock + 1;
    writeToFilesystem(SB_OFFSET, (void*)& superblock, sizeof(struct Superblock));

    char* left = "File ";
    char* right = " was created";
    int len = strlen(left) + strlen(name) + strlen(right) + 1;
    char *result = malloc(len);
    strcpy(result, left);
    strcat(result, mapping.name);
    strcat(result, right);
    send(sock, result, len, 0);
}

void cat_t (int currentDirId, char name[MAX_LENGTH_FILE_NAME], int sock) {
    int id = open_t (currentDirId, name);
    if (id == -1) {
        char* result = "File not found. Try again please.";
        send(sock, result, strlen(result), 0);
        return;
    }
    struct Inode inode = getInode(id);
    char *buf = malloc(inode.size);
    int code = read_t(inode.id, 0, buf, inode.size);

    if (code == -1) {
        char* result = "File is directory or the file does not exist. Exit.";
        send(sock, result, strlen(result), 0);
        return;
    } else {
        send(sock, buf, strlen(buf), 0);
        return;
    }
}

void echo_t (int currentDirId, char name[MAX_LENGTH_FILE_NAME], char text[MAX_LENGTH_TEXT], int sock) {
    int num = open_t (currentDirId, name);
    struct Inode inode = getInode(num);
    if (num == -1) {
        char* result = "File not found. Try again please.";
        send(sock, result, strlen(result), 0);
        return;
    }

    write_t(num, inode.size, text, strlen(text));
    char* result = "Write successful.";
    send(sock, result, strlen(result), 0);
}