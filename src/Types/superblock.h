#pragma once

struct Superblock {				    /* The key information of filesystem */
    int inodeOffset; 			    /* The start offset of the inode region */
    int dataOffset; 			    /* The start offset of the data region */
    int maxInodeNum; 				/* The maximum number of inodes */
    int maxDataBlockNum; 			/* The maximum number of data blocks */
    int nextAvailableInode; 	    /* The index of the next free inode */
    int nextAvailableBlock; 	    /* The index of the next free block */
    int sizeOfBlock; 				/* The size per block */
};

/*
 * Function:  getSuperBlock
 * -------------------------------------------------------------------------
 * 	Just get superblock struct from FS-file
 *
 *  returns: superblock
 */

struct Superblock getSuperblock();

