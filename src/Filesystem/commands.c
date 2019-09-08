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

void ls_t (int currentDirId) {
    struct Inode currentDir = getInode(currentDirId);
    int offset = DATA_OFFSET + currentDir.direct[0] * BLOCK_SIZE;
    for (int i = 0; i < currentDir.numOfFiles; i++) {
        struct Mapping mapping;
        readFromFilesystem(offset + i*sizeof(struct Mapping),
                           (void*)& mapping, sizeof(struct Mapping));

        struct Inode tmp = getInode(mapping.id);
        if (tmp.numOfFiles == 0) {
            printf("\033[22;34m %s \033[0m", mapping.name);
        } else {
            printf("\033[22;34m %s/ \033[0m", mapping.name);
        }
    }
}

int cd_t (int currentDirId, char name[MAX_LENGTH_FILE_NAME]) {
    if (strcmp(name, "/") == 0) {
        printf("Current directory is root");
        return 0;
    }

    if (name[0] == '/') {
        currentDirId = 0;
    }

    int temp = open_t(currentDirId, name);
    if (temp == -1) {
        printf("Can't find path. Please try again.");
        return currentDirId;
    } else {
        if (getInode(temp).numOfFiles == 0) {
            printf("This is a file, not a directory. Current directory does not change. Try again please.");
            return currentDirId;
        }
        printf("Current directory is %s", name);
        return temp;
    }
}

void mkdir_t(int currentDirId, char name[MAX_LENGTH_FILE_NAME]) {
    //Check if there exist a file with the same name
    struct Inode currentDir = getInode(currentDirId);
    int offset = DATA_OFFSET + currentDir.direct[0] * BLOCK_SIZE;
    for (int i = 0; i < currentDir.numOfFiles; i++) {
        struct Mapping mapping;
        readFromFilesystem(offset + (i*sizeof(struct Mapping)),
                           (void*)& mapping, sizeof(struct Mapping));

        if(strcmp(name,mapping.name)==0) {
            printf("A directory with the same name exists.");
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
}

void touch_t(int currentDirId, char name[MAX_LENGTH_FILE_NAME]) {
    //Check if there exist a file with the same name
    struct Inode currentDir = getInode(currentDirId);
    int offset = DATA_OFFSET + currentDir.direct[0] * BLOCK_SIZE;
    for (int i = 0; i < currentDir.numOfFiles; i++) {
        struct Mapping mapping;
        readFromFilesystem(offset + (i*sizeof(struct Mapping)),
                           (void*)& mapping, sizeof(struct Mapping));

        if(strcmp(name, mapping.name) == 0) {
            printf("A file with the same name exists.");
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
    printf("File %s was created", name);
}

void cat_t (int currentDirId, char name[MAX_LENGTH_FILE_NAME]) {
    int id = open_t (currentDirId, name);
    if (id == -1) {
        printf("File not found. Try again please.");
        return;
    }
    struct Inode inode = getInode(id);
    char *buf = malloc(inode.size);
    int code = read_t(inode.id, 0, buf, inode.size);

    if (code == -1) {
        printf("File is directory or the file does not exist. Exit.");
        return;
    } else {
        printf("%s", buf);
        return;
    }
}

void echo_t (int currentDirId, char name[MAX_LENGTH_FILE_NAME], char text[MAX_LENGTH_TEXT]) {
    int num = open_t (currentDirId, name);
    struct Inode inode = getInode(num);
    if (num == -1) {
        printf("File not found. Try again please.");
        return;
    }

    write_t(num, inode.size, text, strlen(text));
    printf("Write successful.");
}