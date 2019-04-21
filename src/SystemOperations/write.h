/* TODO: redone this file */

#include "../define.h"
#include "../Types/superblock.h"

#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>



int write_t( int inode_number, int offset, void *buf, int count);
int createDirectBlk2(struct Inode inode);
int createIndirectBlk(struct Inode inode);
int craeteIndirectPointer(struct Inode inode);
//update nextAvailableBlock, size, numOfBlocks, direct[2], indirect
int write_t( int inode_number, int offset, void *buf, int count)
{
    struct Inode inode;
    int pointer;
    inode = getInode(inode_number);
    if(inode.id != inode_number || inode.numOfFiles != 0)
    {
        printf("File is directory or the file does not exist. Exit with code -1.\n");
        return -1; //error
    }

    if(offset>=BLOCK_SIZE*inode.numOfBlocks)
    {
        printf("Offset is at or past the end of file, no bytes are written.\n");
        return 0;
    }
    //offset within the size of file
    int blk;
    int i;
    for(i=0;i<MAX_DATA_BLOCK;i++) //find which block lies the offset
    {
        if(offset<BLOCK_SIZE*(i+1))
        {
            blk=i;
            if(blk > inode.numOfBlocks)
            {
                printf("Block index is > inode.numOfBlocks, no bytes are written.\n");
                return 0;
            }
            break;
        }
    }

    int offsetCal = offset;
    int remainingCount = count;
    int writeCount;
    int accumulateWrite = 0;
    int dirDataBlkIndex;
    int fd = open (FS_NAME, O_RDWR, 660);
    int blkCreateCount = 0;
    while(remainingCount > 0)
    {
        inode = getInode(inode_number);
        if(blk == 0)
        {
            if (inode.direct[0] == -1)
            {
                printf("Error: No direct block 1. Return bytes read (%d).\n",accumulateWrite);
                return accumulateWrite;
            }
            else
                dirDataBlkIndex = inode.direct[0];
        }else if(blk == 1)
        {
            if (inode.direct[1] == -1)
            {
                printf("Error: No direct block 2. Return bytes read (%d).\n",accumulateWrite);
                return accumulateWrite;
            }
            else
                dirDataBlkIndex = inode.direct[1];
        }else if(blk >= 2)
        {
            printf("%d\n", inode.direct[1]);
            if (inode.indirect == -1)
            {
                printf("Error: No indirect block. Return bytes read (%d).\n",accumulateWrite);
                return accumulateWrite;
            }
            else
            {
                //find the suitable block:
                //goto the indirect pointer datablock, get the i-th pointer
                //dirDataBlkIndex = that pointer(index)
                lseek(fd, DATA_OFFSET + inode.indirect * BLOCK_SIZE + (blk-2)*sizeof(int), SEEK_SET);
                read(fd, (void *)&pointer, sizeof(int));
                dirDataBlkIndex = pointer;
            }
        }
        if(offset > 0) //first writeCount: offset > 0
            offsetCal = offset-(blk*BLOCK_SIZE); //the offset in the block

        lseek(fd, DATA_OFFSET + dirDataBlkIndex * BLOCK_SIZE + offsetCal, SEEK_SET);

        if(remainingCount < BLOCK_SIZE-offsetCal) //can finish writeCount in this block
        {
            writeCount = remainingCount;
            remainingCount -= writeCount;
        }else
        {
            writeCount = BLOCK_SIZE-offsetCal;
            remainingCount -= writeCount;
        }
        write(fd, buf + accumulateWrite, writeCount);
        accumulateWrite += writeCount;

        if(remainingCount > 0) //still have to move to next block
        {
            //move to next blk
            offset = 0; //offset is 0 now
            offsetCal = 0;
            blk++;
//CODE FOR write_t--------------------------------------------------------------------------
            //create next block and update nextAvailableBlock here
            blkCreateCount++;
            if(blk == 1 && getInode(inode_number).direct[1] == -1) //create dirblock2, update numOfBlocks(blkCreateCount)
            {
                createDirectBlk2(getInode(inode_number));
            }else if(blk == 2 && getInode(inode_number).indirect == -1) //create indir + indir pointer, update numOfBlocks(blkCreateCount)
            {
                createIndirectBlk(getInode(inode_number)); //indirect datablock does not count as data block in numOfBlocks
                craeteIndirectPointer(getInode(inode_number));
            }else if(blk > 2) //create indir pointer, update numOfBlocks(blkCreateCount)
            {
                craeteIndirectPointer(getInode(inode_number));
            }
        }
    }
//update size (accumulateWrite)------------------------------------------------------------
    struct Inode updatedInode={};
    updatedInode.id = inode.id;
    updatedInode.size = inode.size+accumulateWrite; //update
    updatedInode.numOfBlocks = inode.numOfBlocks;
    updatedInode.direct[0] = inode.direct[0];
    updatedInode.direct[1] = inode.direct[1];
    updatedInode.indirect = inode.indirect;
    lseek(fd, INODE_OFFSET + inode.id*sizeof(struct Inode), SEEK_SET);
    write(fd, &updatedInode, sizeof(struct Inode));

//CODE FOR write_t end-----------------------------------------------------------------------
    close(fd);
    return accumulateWrite;
}

int createDirectBlk2(struct Inode inode){
    struct Superblock sb;
    sb = getSuperblock();
    int nextAvailableBlock = sb.nextAvailableBlock;
    if(nextAvailableBlock == MAX_DATA_BLOCK)
    {
        printf("Reached MAX_DATA_BLOCK! Abort.\n");
        return -1;
    }
    int fd = open (FS_NAME, O_RDWR, 660);
    //update inode
    struct Inode updatedInode={};
    updatedInode.id = inode.id;
    updatedInode.size = inode.size;
    updatedInode.numOfBlocks = 2;
    updatedInode.direct[0] = inode.direct[0];
    updatedInode.direct[1] = nextAvailableBlock; //update
    updatedInode.indirect = inode.indirect;
    lseek(fd, INODE_OFFSET + inode.id*sizeof(struct Inode), SEEK_SET);
    //printf("d2: updatedInode.direct[1] = %d\n", updatedInode.direct[1]);
    write(fd, &updatedInode, sizeof(struct Inode));

    //update superblock
    struct Superblock updatedSb={};
    updatedSb.inodeOffset = INODE_OFFSET;
    updatedSb.dataOffset = DATA_OFFSET;
    updatedSb.maxInodeNum = MAX_INODE;
    updatedSb.maxDataBlockNum = MAX_DATA_BLOCK;
    updatedSb.sizeOfBlock = BLOCK_SIZE;
    updatedSb.nextAvailableInode = sb.nextAvailableInode;
    updatedSb.nextAvailableBlock = (nextAvailableBlock+1); //update here

    lseek(fd, SB_OFFSET, SEEK_SET);
    write(fd, &updatedSb, sizeof(struct Superblock));

    close(fd);
    return nextAvailableBlock;
}

int createIndirectBlk(struct Inode inode){
    struct Superblock sb;
    sb = getSuperblock();
    int nextAvailableBlock = sb.nextAvailableBlock;
    int fd = open (FS_NAME, O_RDWR, 660);
    if(nextAvailableBlock == MAX_DATA_BLOCK)
    {
        printf("Reached MAX_DATA_BLOCK! Abort.\n");
        return -1;
    }
    //update inode
    struct Inode updatedInode={};
    updatedInode.id = inode.id;
    updatedInode.size = inode.size;
    updatedInode.numOfBlocks = 2; //dont change, indirect block does not count as block here
    updatedInode.direct[0] = inode.direct[0];
    updatedInode.direct[1] = inode.direct[1];
    updatedInode.indirect = nextAvailableBlock; //update
    lseek(fd, INODE_OFFSET + inode.id*sizeof(struct Inode), SEEK_SET);
    //printf("Indirect updatedInode.indirect = %d\n", updatedInode.indirect);
    write(fd, &updatedInode, sizeof(struct Inode));

    //update superblock
    struct Superblock updatedSb={};
    updatedSb.inodeOffset = INODE_OFFSET;
    updatedSb.dataOffset = DATA_OFFSET;
    updatedSb.maxInodeNum = MAX_INODE;
    updatedSb.maxDataBlockNum = MAX_DATA_BLOCK;
    updatedSb.sizeOfBlock = BLOCK_SIZE;
    updatedSb.nextAvailableInode = sb.nextAvailableInode;
    updatedSb.nextAvailableBlock = (nextAvailableBlock+1); //update here

    lseek(fd, SB_OFFSET, SEEK_SET);
    write(fd, &updatedSb, sizeof(struct Superblock));

    close(fd);
    return nextAvailableBlock;
}


int craeteIndirectPointer(struct Inode inode){
    struct Superblock sb;
    sb = getSuperblock();
    int nextAvailableBlock = sb.nextAvailableBlock;
    int fd = open (FS_NAME, O_RDWR, 660);
    if(nextAvailableBlock == MAX_DATA_BLOCK)
    {
        printf("Reached MAX_DATA_BLOCK! Abort.\n");
        return -1;
    }
    //update inode
    struct Inode updatedInode={};
    updatedInode.id = inode.id;
    updatedInode.size = inode.size;
    updatedInode.numOfBlocks = inode.numOfBlocks+1; //update
    updatedInode.direct[0] = inode.direct[0];
    updatedInode.direct[1] = inode.direct[1];
    updatedInode.indirect = inode.indirect;
    lseek(fd, INODE_OFFSET + inode.id*sizeof(struct Inode), SEEK_SET);
    write(fd, &updatedInode, sizeof(struct Inode));

    //update indirect data block
    int pointer;
    pointer = nextAvailableBlock;
    lseek(fd, DATA_OFFSET + inode.indirect*BLOCK_SIZE + (inode.numOfBlocks-2)*sizeof(int), SEEK_SET);
    write(fd, &pointer, sizeof(int));

    //update superblock
    struct Superblock updatedSb={};
    updatedSb.inodeOffset = INODE_OFFSET;
    updatedSb.dataOffset = DATA_OFFSET;
    updatedSb.maxInodeNum = MAX_INODE;
    updatedSb.maxDataBlockNum = MAX_DATA_BLOCK;
    updatedSb.sizeOfBlock = BLOCK_SIZE;
    updatedSb.nextAvailableInode = sb.nextAvailableInode;
    updatedSb.nextAvailableBlock = (nextAvailableBlock+1); //update here

    lseek(fd, SB_OFFSET, SEEK_SET);
    write(fd, &updatedSb, sizeof(struct Superblock));

    close(fd);
    return nextAvailableBlock;
}