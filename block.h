#ifndef __BLOCK_FILESYSTEM__
#define __BLOCK_FILESYSTEM__
#include "common.h"
#define MAX_SIZE 251
struct super_block {
    uint isize;
    uint fsize;
    uint nfree;
    uint ninode;
    uint free[MAX_SIZE];
    uint inode[MAX_SIZE];
    char flock;
    char ilock;
    char fmod;
    char padding;
    long time[2];
};
#define BLOCKSIZE sizeof(struct super_block)
//for the head of free array
struct head_free_block {
    uint nfr;
    uint fr[MAX_SIZE];
    uint padding[512 - MAX_SIZE - 1];
};

struct block {
    uint data[512];
};

struct data_block {
    uint data[512];
};

extern int curr_fd;

inline void write_superblock();
ssize_t read_block(uint block, void* buf);
ssize_t write_block(uint block, void *buf);
void free_block(uint free_block); //add a freed block number to free array;
uint allocate_block();   //return a freed block number

void initiate_super_block(int fd, int total_block_number, int inode_block_number); 
void print_superblock();    
#endif

