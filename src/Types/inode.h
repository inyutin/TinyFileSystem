#pragma once

struct Inode {					/* The structure of inode, each file has only one inode */
    int id; 				    /* The inode number */
    int size; 				    /* The size of file */
    int numOfBlocks; 			/* The total numbers of data blocks */
    int direct[2]; 			    /* Two direct data block pointers */
    int indirect; 			    /* One indirect data block pointer */
    int numOfFiles; 			/* Amount of files in directory, 0 if file */
};


/*
 * Function:  getInode
 * -------------------------------------------------------------------------
 *   Just get Inode by it's id
 *
 *   id: Inode id to return
 *
 *   returns: Inode
 */
struct Inode getInode(int id);

void createInode(int id, int parentId, int block, int type);
