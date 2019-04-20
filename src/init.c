#include "define.h"
#include "Filesystem/filesystem.h"

#include "Types/superblock.h"
#include "Types/inode.h"
#include "Types/mapping.h"

int main(int argc, char *argv[]) {

    // Initializing Superblock
    struct Superblock superblock;
    superblock.inodeOffset = INODE_OFFSET;
    superblock.dataOffset = DATA_OFFSET;
    superblock.maxInodeNum = MAX_INODE;
    superblock.maxDataBlockNum = MAX_DATA_BLOCK;
    superblock.sizeOfBlock = BLOCK_SIZE;
    superblock.nextAvailableInode = 1;
    superblock.nextAvailableBlock = 1;
    writeToFilesystem (SB_OFFSET, (void*) &superblock, sizeof(struct Superblock));

    // Initializing root
    struct Inode root;
    root.id = 0;
    root.size = 0;
    root.numOfBlocks = 1;
    root.direct[0] = 0;
    root.direct[1] = -1;
    root.indirect = -1;
    root.numOfFiles = 1;
    writeToFilesystem (INODE_OFFSET, (void*) &root, sizeof(struct Inode));

    // Map root to itself
	struct Mapping rootDir = createMapping(".", 0);
    writeToFilesystem (DATA_OFFSET, (void*) &rootDir, sizeof(struct Mapping));

	return 0;
}