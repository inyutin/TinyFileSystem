#include "../define.h"
#include "../Types/superblock.h"

int write_t(int id, int offset, void *buf, int count);

int write_t(int id, int offset, void *buf, int count) {
    struct Inode inode = getInode(id);

    if (inode.id != id || inode.numOfFiles != 0) {
        printf("File is directory or the file does not exist. Exit with code -1.\n");
        return -1;
    }

    if (offset>=BLOCK_SIZE*inode.numOfBlocks) {
        printf("Offset is at or past the end of file, no bytes are written.\n");
        return 0;
    }

    int blk;
    for (int i = 0; i<MAX_DATA_BLOCK; i++) {
        if (offset < BLOCK_SIZE*(i+1)) {
            blk=i;
            if (blk > inode.numOfBlocks) {
                printf("Block index is > inode.numOfBlocks, no bytes are written.\n");
                return 0;
            }
            break;
        }
    }

    int fd = open (FS_NAME, O_RDWR, 660);
    int accumulateWrite = 0;
    int dirDataBlkIndex = 0;
    int blkCreateCount = 0;
    while(count > 0) {
        if(blk == 0) {
            if (inode.direct[0] == -1) {
                printf("Error: No direct block 1. Return bytes read (%d).\n",accumulateWrite);
                return accumulateWrite;
            } else {
                dirDataBlkIndex = inode.direct[0];
            }
        } else if(blk == 1) {
            if (inode.direct[1] == -1) {
                printf("Error: No direct block 2. Return bytes read (%d).\n",accumulateWrite);
                return accumulateWrite;
            } else {
                dirDataBlkIndex = inode.direct[1];
            }
        } else if(blk >= 2) {
            printf("%d\n", inode.direct[1]);
            if (inode.indirect == -1) {
                printf("Error: No indirect block. Return bytes read (%d).\n",accumulateWrite);
                return accumulateWrite;
            }
            else {
                lseek(fd, DATA_OFFSET + inode.indirect * BLOCK_SIZE + (blk)*sizeof(int), SEEK_SET);
                read(fd, (void*)& dirDataBlkIndex, sizeof(int));
            }
        }
        if (offset > 0) {
            offset = offset - (blk * BLOCK_SIZE);
        }

        int writeCount = 0;
        if (count < BLOCK_SIZE-offset) {
            writeCount = count;
            count-= writeCount;
        } else {
            writeCount = BLOCK_SIZE-offset;
            count -= writeCount;
        }

        lseek(fd, DATA_OFFSET + dirDataBlkIndex * BLOCK_SIZE + offset, SEEK_SET);
        write(fd, buf + accumulateWrite, writeCount);

        accumulateWrite += writeCount;

        if (count > 0) {
            offset = 0;
            blk++;

            blkCreateCount++;
            if (blk == 1 && getInode(id).direct[1] == -1) {
                struct Superblock superblock = getSuperblock();
                int nextAvailableBlock = superblock.nextAvailableBlock;

                if (nextAvailableBlock == MAX_DATA_BLOCK) {
                    printf("Reached MAX_DATA_BLOCK! Abort.\n");
                    return -1;
                }

                inode.numOfBlocks = 2;
                inode.direct[1] = nextAvailableBlock; //update

                lseek(fd, INODE_OFFSET + inode.id*sizeof(struct Inode), SEEK_SET);
                write(fd, &inode, sizeof(struct Inode));

                superblock.nextAvailableBlock = nextAvailableBlock+1; //update here

                lseek(fd, SB_OFFSET, SEEK_SET);
                write(fd, &superblock, sizeof(struct Superblock));

            } else if(blk == 2 && getInode(id).indirect == -1) {
                struct Superblock superblock = getSuperblock();
                int nextAvailableBlock = superblock.nextAvailableBlock;

                if (nextAvailableBlock == MAX_DATA_BLOCK) {
                    printf("Reached MAX_DATA_BLOCK! Abort.\n");
                    return -1;
                }

                inode.indirect = nextAvailableBlock; //update

                lseek(fd, INODE_OFFSET + inode.id*sizeof(struct Inode), SEEK_SET);
                write(fd, &inode, sizeof(struct Inode));

                superblock.nextAvailableBlock = nextAvailableBlock+1; //update here

                lseek(fd, SB_OFFSET, SEEK_SET);
                write(fd, &superblock, sizeof(struct Superblock));

                nextAvailableBlock = nextAvailableBlock+1;

                inode.numOfBlocks = inode.numOfBlocks+1;

                lseek(fd, INODE_OFFSET + inode.id*sizeof(struct Inode), SEEK_SET);
                write(fd, &inode, sizeof(struct Inode));

                lseek(fd, DATA_OFFSET + inode.indirect*BLOCK_SIZE + (inode.numOfBlocks)*sizeof(int), SEEK_SET);
                write(fd, &nextAvailableBlock, sizeof(int));

                superblock.nextAvailableBlock = nextAvailableBlock+14;

                lseek(fd, SB_OFFSET, SEEK_SET);
                write(fd, &superblock, sizeof(struct Superblock));
            } else if(blk > 2) {
                struct Superblock superblock = getSuperblock();
                int nextAvailableBlock = superblock.nextAvailableBlock;

                if (nextAvailableBlock == MAX_DATA_BLOCK) {
                    printf("Reached MAX_DATA_BLOCK! Abort.\n");
                    return -1;
                }

                inode.numOfBlocks = inode.numOfBlocks+1;

                lseek(fd, INODE_OFFSET + inode.id*sizeof(struct Inode), SEEK_SET);
                write(fd, &inode, sizeof(struct Inode));

                lseek(fd, DATA_OFFSET + inode.indirect*BLOCK_SIZE + (inode.numOfBlocks)*sizeof(int), SEEK_SET);
                write(fd, &nextAvailableBlock, sizeof(int));

                superblock.nextAvailableBlock = nextAvailableBlock+14;

                lseek(fd, SB_OFFSET, SEEK_SET);
                write(fd, &superblock, sizeof(struct Superblock));
            }
        }
    }

    inode.size = inode.size+accumulateWrite;
    lseek(fd, INODE_OFFSET + inode.id*sizeof(struct Inode), SEEK_SET);
    write(fd, &inode, sizeof(struct Inode));

    close(fd);
    return accumulateWrite;
}

int createIndirectBlk(struct Inode inode) {
    struct Superblock superblock = getSuperblock();
    int nextAvailableBlock = superblock.nextAvailableBlock;

    if (nextAvailableBlock == MAX_DATA_BLOCK) {
        printf("Reached MAX_DATA_BLOCK! Abort.\n");
        return -1;
    }

    inode.indirect = nextAvailableBlock; //update

    int fd = open (FS_NAME, O_RDWR, 660);
    lseek(fd, INODE_OFFSET + inode.id*sizeof(struct Inode), SEEK_SET);
    write(fd, &inode, sizeof(struct Inode));

    superblock.nextAvailableBlock = nextAvailableBlock+1; //update here

    lseek(fd, SB_OFFSET, SEEK_SET);
    write(fd, &superblock, sizeof(struct Superblock));

    close(fd);
    return nextAvailableBlock;
}
