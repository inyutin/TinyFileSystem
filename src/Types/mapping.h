#pragma once

#include "../define.h"

struct Mapping {			            /* Record file information in directory file */
    char name[MAX_LENGTH_FILE_NAME];    /* The file name in current directory */
    int id; 			                /* The corresponding inode number */
};


struct Mapping createMapping (char name[MAX_LENGTH_FILE_NAME], int id);