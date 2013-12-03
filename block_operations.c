#include "common.h"
#include "block.h"
#include "inode.h"

void initiate_free_list();
void initiate_inode_list();

int curr_fd;
struct super_block curr_superblock;
static struct block curr_block;
static uint curr_block_num;
  
inline void write_superblock() {
        int offs = 0;
        if((offs = lseek(curr_fd, sizeof(struct block), SEEK_SET)) < 0) {
            fprintf(stderr, "Error in seek super block\n");   
            exit(errno);
        }

        int bytes = 0;
        if((bytes = write(curr_fd, &curr_superblock, sizeof(struct super_block)) < 0 )) {
                fprintf(stderr, "Error in write th superblock errno = %d\n", -bytes);
                exit(errno);
        }
}


ssize_t read_block(uint block_index, void* buf) {
        int bytes = 0;
        if (block_index > (curr_superblock.fsize - 1))
            return -1;
        
        int offset = block_index * BLOCKSIZE;

        // if(block_index == curr_block_num) {
        //     memcpy(buf, curr_block.data, BLOCKSIZE);
        //     return BLOCKSIZE;
        // }
    
       // curr_block_num = block_index;       
        int offs = 0;
        if((offs = lseek(curr_fd, offset, SEEK_SET)) < 0) {
            fprintf(stderr, "Error in seek block %d\n", block_index);   
            exit(errno);
        }

        if((bytes = read(curr_fd, (void *)(buf), BLOCKSIZE)) < 0) {
            fprintf(stderr, "Error in read block %d\n", block_index);
            exit(errno);
        }
        //memcpy(buf, curr_block.data, BLOCKSIZE);
        
        return bytes;
}

ssize_t write_block(uint block_index, void *buf) {
        if (block_index > (curr_superblock.fsize - 1))
            return -1;
        
        int offset = block_index * BLOCKSIZE;    
        memcpy(curr_block.data, buf, BLOCKSIZE);
        
        curr_block_num = block_index;
        int offs = 0;
        if((offs = lseek(curr_fd, offset, SEEK_SET)) < 0) {
                fprintf(stderr, "Error in seek block %d\n", block_index);
                exit(errno);
        }

        int bytes = 0;
        if((bytes = write(curr_fd, curr_block.data, BLOCKSIZE)) < 0) {
                fprintf(stderr, "Error in read block %d\n", block_index);
                exit(errno);
        }

        return bytes;
}

void free_block(uint free_block) {
    if(curr_superblock.nfree ==  MAX_SIZE) {
        struct head_free_block w;

        //int i;
        w.nfr = curr_superblock.nfree;
        memcpy(w.fr, curr_superblock.free, curr_superblock.nfree * sizeof(uint));
        write_block(free_block, (void*)&w); //write nfree and free array into block i;
        curr_superblock.nfree = 0;
    }
    curr_superblock.free[curr_superblock.nfree++] = free_block;
    write_superblock();
}


uint allocate_block() {
    curr_superblock.nfree--;
    uint block_id = curr_superblock.free[curr_superblock.nfree];
    if(block_id == 0) {
        fprintf(stderr, "Error: the data blocks are full, allocation failure\n");
        exit(-1);
    }
    if(curr_superblock.nfree == 0) {
        struct head_free_block w;
        //int i;
        read_block(block_id, (void*)&w);
        curr_superblock.nfree = w.nfr;
        memcpy(curr_superblock.free, w.fr, (curr_superblock.nfree + 1) * sizeof(uint));
    }
	write_superblock();
    return block_id;
}

void initiate_super_block(int fd, int total_block_number, int inode_block_number) {
    curr_fd = fd;
    curr_superblock.isize = inode_block_number;
    curr_superblock.fsize = total_block_number;
    curr_superblock.nfree = 0;
    curr_superblock.ninode = 0;
    initiate_free_list();
    initiate_inode_list();
    write_superblock();

}
void initiate_free_list() {
    free_block(0);
    uint max_inode_block = 1 + curr_superblock.isize;
    int i;
    for(i = curr_superblock.fsize - 1; i > max_inode_block; i--) {
        free_block(i);
    }
}

void initiate_inode_list() {

    int arr[512];
    memset(arr,0 ,sizeof(arr));
    int i;

    uint max_inode_block = 1 + curr_superblock.isize;
    for(i = 2; i <= max_inode_block; i++)
        write_block(i, arr);

    //int brr[512];
    //read_block(20,brr,512);

    uint inode_number = curr_superblock.isize * INODES_PER_BLOCK;
    for(i = 2; i <= inode_number; i++) {
        if(curr_superblock.ninode == MAX_SIZE) 
            break;
        struct inode ino;
        read_inode(i, &ino);
        if(is_allocated_inode(&ino) != 1) {
            free_inode(i);    
        }
    }
    if(curr_superblock.ninode == 0){ 
        fprintf(stderr,"Error: the inode blocks are full, allocation falure!");
        exit(-1);
    }
}

void print_superblock() {
    printf("isize: %d\n",curr_superblock.isize);
    printf("fsize: %d\n",curr_superblock.fsize);
    printf("nfree: %d\n",curr_superblock.nfree);
    int i;
    for(i = curr_superblock.nfree -1; i>=0; i--){
        printf("free block %d: %d  ",i, curr_superblock.free[i]);
    }
    printf("\n");
    printf("ninode: %d\n",curr_superblock.ninode);
    for(i = 0; i<curr_superblock.ninode; i++){
        printf("free inode %d: %d  ",i, curr_superblock.inode[i]);
    }
}
