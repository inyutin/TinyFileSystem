#include "../define.h"

#include "filesystem.h"

#include "../Types/superblock.h"
#include "../Types/inode.h"
#include "../Types/mapping.h"

#include "../SystemOperations/open.h"

#include <stdio.h>
#include <string.h>

/* TODO: add description */
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

/* TODO: add description */
int cd_t (int currentDirId, char name[MAX_LENGTH_FILE_NAME]) {
    if(strcmp(name,"/")==0) {
        printf("Current directory is root\n");
        return 0;
    }

    if (name[0] == '/') {
        currentDirId = 0;
    }

    int temp = open_t(currentDirId, name);
    if (temp == -1) {
        printf("Can't find path. Please try again.\n");
        return currentDirId;
    } else{
        if (getInode(temp).numOfFiles == 0) {
            printf("This is a file, not a directory. Current directory does not change. Try again please.\n");
            return currentDirId;
        }
        return temp;
    }
}

/* TODO: add description */
void mkdir_t(int currentDirId, char name[MAX_LENGTH_FILE_NAME]) {
    //Check if there exist a file with the same name
    struct Inode currentDir = getInode(currentDirId);
    int offset = DATA_OFFSET + currentDir.direct[0] * BLOCK_SIZE;
    for (int i = 0; i < currentDir.numOfFiles; i++) {
        struct Mapping mapping;
        readFromFilesystem(offset + (i*sizeof(struct Mapping)),
                           (void*)& mapping, sizeof(struct Mapping));

        if(strcmp(name,mapping.name)==0) {
            printf("A directory with the same name exists.\n");
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

/* TODO: add description */
void touch_t(int currentDirId, char name[MAX_LENGTH_FILE_NAME]) {
    //Check if there exist a file with the same name
    struct Inode currentDir = getInode(currentDirId);
    int offset = DATA_OFFSET + currentDir.direct[0] * BLOCK_SIZE;
    for (int i = 0; i < currentDir.numOfFiles; i++) {
        struct Mapping mapping;
        readFromFilesystem(offset + (i*sizeof(struct Mapping)),
                           (void*)& mapping, sizeof(struct Mapping));

        if(strcmp(name,mapping.name)==0) {
            printf("A directory with the same name exists.\n");
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
}
