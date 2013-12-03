#ifndef __INODE__FILESYSTEM_JUN__
#define __INODE__FILESYSTEM_JUN__

#include "common.h"

#define INODE_ADDR_LEN 20

struct inode {
    ushort flags;
    char nlinks;
    char uid;
    char gid;
    char padding[3];
    uint size;
    uint pos;    // file pointer
    uint addr[20];
    long actime[2];
    long modtime[2];
};

struct inode_data {
	uint addr[20];
};

#define INODESIZE sizeof(struct inode)
#define INODES_PER_BLOCK (BLOCKSIZE/INODESIZE)
int read_inode(uint inode, struct inode *inode_buf);
int write_inode(uint inode, struct inode *inode_buf);
int free_inode(uint free_inode);
uint get_free_inode();  //replace allocate_inode()
int is_free_inode(struct inode* ino);
inline int allocate_inode_by_number(uint inode);
inline int allocate_inode(struct inode *file_inode);
inline int is_allocated_inode(struct inode *file_inode);
#endif
