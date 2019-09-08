#include "../define.h"

#include "../Types/superblock.h"

#include <string.h>
#include <stdio.h>

/*
 * Function:  open_t
 * -------------------------------------------------------------------------
 *  Open inode by pathname from directory tied with currentId
 *
 *  currentId: id of current dir
 *  pathname: the name of file
 *
 *  returns: The id of file
 *           returns -1 on error (if file didn't exist)
 */
int open_t(int currentId, const char* pathname) {
    char* string = strdup(pathname);

    if (string != NULL) {
        char* token = strsep(&string, "/");
        while (token != NULL) {
            if (strcmp(token, "") != 0) {
                struct Inode currentDir = getInode(currentId);
                int offset = DATA_OFFSET + currentDir.direct[0] * BLOCK_SIZE;
                int flag = 1;

                for (int i = 0; i < currentDir.numOfFiles; i++) {
                    struct Mapping mapping;
                    readFromFilesystem(offset + i*sizeof(struct Mapping),
                                       (void*)& mapping, sizeof(struct Mapping));

                    if (strcmp(token, mapping.name) == 0) {
                        currentId = mapping.id;
                        flag = 0;
                        break;
                    }
                }

                if (flag) {
                    return -1;
                }
            }
            token = strsep(&string, "/");
        }
        return currentId;
	}
    return -1;
}
