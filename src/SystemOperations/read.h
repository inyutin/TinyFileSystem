#include "../define.h"
#include "../Types/inode.h"

#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

/*
 * Function:  read_t
 * -------------------------------------------------------------------------
 *  Read bytes from file by it id (inode id)
 *
 *  id: id of inode associated with those blocks where we want to read
 *  offset: the number of byte from which we start reading
 *  buf: the buffer for information weread
 *  count: the number of bytes to read
 *
 *  returns: The number of bytes read
 *           returns -1 on error (if file didn't exist or it is a dir)
 */

int read_t(int id, int offset, void* buf, int count) {
    struct Inode iNode = getInode(id);
    if (iNode.id != id || iNode.numOfFiles != 0) {
        return -1;
    }

    if (offset >= iNode.size) {
        return 0;
    }

    int block = (int) floor (offset / BLOCK_SIZE);

    int fd = open (FS_NAME, O_RDWR, 660);
    int readCount;
    int accumulateRead = 0;
    int dirDataBlkIndex = 0;

    while(count > 0) {
        if (block == 0) {
            if (iNode.direct[0] == -1) {
                printf("Error: No direct block 1. Return bytes read.\n");
                return accumulateRead;
            } else {
                dirDataBlkIndex = iNode.direct[0];
            }
        } else if (block == 1) {
            if (iNode.direct[1] == -1) {
                printf("Error: No direct block 2. Return bytes read.\n");
                return accumulateRead;
            } else {
                dirDataBlkIndex = iNode.direct[1];
            }
        } else if (block >= 2) {
            if (iNode.indirect == -1) {
                printf("Error: No indirect block. Return bytes read.\n");
                return accumulateRead;
            } else {
                lseek(fd, DATA_OFFSET + iNode.indirect * BLOCK_SIZE + (block-2)*sizeof(int), SEEK_SET);
                read(fd, (void *)& dirDataBlkIndex, sizeof(int));
            }
        }

        if (offset > 0) {
            offset = offset - (block * BLOCK_SIZE);
        }

        lseek(fd, DATA_OFFSET + dirDataBlkIndex * BLOCK_SIZE + offset, SEEK_SET);

        if (count < BLOCK_SIZE-offset) {
            readCount = count;
            count -= readCount;
        } else {
            readCount = BLOCK_SIZE-offset;
            count -= readCount;
        }

        read(fd, buf+accumulateRead, readCount);
        accumulateRead += readCount;
        if (count > 0) {
            offset = 0;
            block++;
        }
    }

    close(fd);
    return accumulateRead;
}
