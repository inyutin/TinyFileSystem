#include "superblock.h"

#include "../define.h"
#include "../Filesystem/filesystem.h"

struct Superblock getSuperblock() {
    struct Superblock superblock;
    readFromFilesystem(SB_OFFSET, (void*)& superblock, sizeof(struct Superblock));
    return superblock;
}