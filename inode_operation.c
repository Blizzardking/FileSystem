#include "inode.h"
#include "block.h"

//extern int curr_fd;
extern struct super_block curr_superblock;
//struct inode curr_inode;
//uint curr_inode_num;


inline int allocate_inode_by_number(uint inode) {
    struct inode file_inode;
    read_inode(inode, &file_inode);
    int ret = allocate_inode(&file_inode);
    write_inode(inode, &file_inode);
    return ret;
}

// inline uint get_and_allocate_inode(struct inode *out_inode) {
    
    
//  if(is_allocated_inode(out_inode) == 1)
//      return -1;

//  allocate_inode(out_inode);
//  return inode;
// }

inline int allocate_inode(struct inode *file_inode) {
    file_inode->flags |= 0100000;
    return 0;
}

// return 0 if no error, otherwise return -1
int read_inode(uint inode, struct inode *inode_buf) {
    assert(inode > 0);
    //error: inode_offset may be negative
    //error: curr_inode and curr_block are not updated;
    if(inode > curr_superblock.isize * (INODES_PER_BLOCK))
        return -1;

    int block_index = (inode - 1) / INODES_PER_BLOCK + 2;
    int inode_offset = (inode - 1) % INODES_PER_BLOCK;
    int byte_offset = inode_offset * INODESIZE;
    struct block curr_block;

    read_block(block_index, &curr_block);
    
    memcpy((void *)inode_buf, (void *)&curr_block + byte_offset, INODESIZE);

    return 0;
}

// return 0 if no error, otherwise return -1
int write_inode(uint inode, struct inode *inode_buf) {
    assert(inode > 0);
    if(inode > curr_superblock.isize * (INODES_PER_BLOCK))
        return -1;

    int block_index = (inode - 1) / INODES_PER_BLOCK + 2;
    int inode_offset = (inode -1) % INODES_PER_BLOCK;
    int byte_offset = inode_offset * INODESIZE;
    struct block curr_block;

    //memcpy(&curr_inode, inode_buf, INODESIZE);

    
    read_block(block_index, &curr_block);
    
    memcpy((void *)&curr_block + byte_offset, (void *)inode_buf, INODESIZE);
    write_block(block_index, &curr_block);

    return 0;
}

int free_inode(uint free_inode) {
    if(curr_superblock.ninode == MAX_SIZE)
        return -1;
    curr_superblock.inode[curr_superblock.ninode] = free_inode;
    curr_superblock.ninode++;
    write_superblock();
    return 0; 
}

uint get_free_inode() {
    if(curr_superblock.ninode == 0) 
        return -1;
    curr_superblock.ninode--;
    uint inode_id = curr_superblock.inode[curr_superblock.ninode];
    write_superblock();
    return inode_id; 
           
}


// 1 if already allocated
inline int is_allocated_inode(struct inode *file_inode) {
    if((file_inode->flags & 0100000) != 0)
        return 1;
    return 0;
}
