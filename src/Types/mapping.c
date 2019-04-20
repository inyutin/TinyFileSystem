#include "../define.h"
#include "inode.h"
#include "mapping.h"

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

struct Mapping createMapping (char name[MAX_LENGTH_FILE_NAME], int id) {
    struct Mapping toReturn;
    strcpy(toReturn.name, name);
    toReturn.id = id;
    return toReturn;
}