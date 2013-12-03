#include "common.h"
#include "file.h"
#include "inode.h"
#include "block.h"

//struct V6_file curr_file;
//extern struct V6_file curr_dir;
//extern struct inode curr_inode;
//extern uint curr_inode_num;

struct V6_file root;

// int get_root_directory(struct V6_file *root) {
// 	root->inumber = 1;
// 	strncpy(root->filename, "/", FILENAME_LENGTH);
// 	return 1;
// }

int make_root_directory(struct V6_file *_root) {

	struct inode root_inode;
	read_inode(1, &root_inode);
	allocate_inode(&root_inode);
	_root->inumber = 1;
	strncpy((char *)_root->filename, (const char *)"/", 1);
	struct V6_file dot;
	struct V6_file dotdot;
	dot.inumber = 1;
	dotdot.inumber = 1;
	strncpy((char *)dot.filename, ".", FILENAME_LENGTH);
	strncpy((char *)dotdot.filename, "..", FILENAME_LENGTH);
	add_entry_to_inode(&dot, &root_inode);
	add_entry_to_inode(&dotdot, &root_inode);
	write_inode(1, &root_inode);
	memcpy(&root, _root, sizeof(struct V6_file));
	return 0;
}

int add_file_entry_to_directory(struct file_entry *file, struct inode *dir_inode) {
	return 0;
}

uint make_directory_in_directory(char *filename, struct V6_file *spec_dir) {
	// if(find_file_in_directory(filename, spec_dir) > 0)
	// 	return -1; // Directory already exists

	struct inode dir_inode;
	struct file_entry new_entry;
	uint inode = get_free_inode();
	read_inode(inode, &dir_inode);
	allocate_inode(&dir_inode);
	write_inode(inode, &dir_inode);
	strncpy((char *)new_entry.filename, (const char *)filename, FILENAME_LENGTH);
	new_entry.inumber = inode;
	//create_empty_directory_entry(filename, &new_entry);

	add_directory_to_directory(&new_entry, &root);
	

	return inode;
	// not write spec_dir to disk, it may be completed by upper call
}

int add_directory_to_directory(struct file_entry *dir_entry, struct file_entry *parent_dir_entry) {
	struct file_entry itself;
	memset(&itself, 0, FILE_ENTRY_SIZE);
	itself.inumber = dir_entry->inumber;
	strncpy((char *)itself.filename, ".", 2);
	
	struct file_entry parent;
	memset(&parent, 0, FILE_ENTRY_SIZE);

	memcpy(&parent, parent_dir_entry, FILE_ENTRY_SIZE);
	strncpy((char *)parent.filename, "..", 3);

	struct inode itself_dir_inode;
	struct inode parent_dir_inode;

	read_inode(dir_entry->inumber, &itself_dir_inode);
	read_inode(parent_dir_entry->inumber, &parent_dir_inode);

	add_entry_to_inode(&itself, &itself_dir_inode);
	add_entry_to_inode(&parent, &itself_dir_inode);
	write_inode(dir_entry->inumber, &itself_dir_inode);

	int ret = add_directory_to_inode(dir_entry, &parent_dir_inode);
	write_inode(parent_dir_entry->inumber, &parent_dir_inode);
	return ret;
}

inline int add_directory_to_inode(struct file_entry *dir_entry, struct inode *parent_dir_inode) {
	return add_entry_to_inode(dir_entry, parent_dir_inode);
}

// add dir_inode to new created file
int add_entry_to_inode(struct file_entry *entry, struct inode *dir_inode) {

	int full_blocks = dir_inode->size / BLOCKSIZE;
	if(full_blocks > INODE_ADDR_LEN)
		return -1;

	//if(is_directory(entry->inumber))


	uint block_index = dir_inode->addr[full_blocks];
	if(block_index == 0) {
		block_index = allocate_block();
		dir_inode->addr[full_blocks] = block_index; 
	}
	struct block last_block;
	read_block(block_index, &last_block, BLOCKSIZE);
	int extra_entry = (dir_inode->size % BLOCKSIZE) / FILE_ENTRY_SIZE;
	
	if(extra_entry == (BLOCKSIZE / FILE_ENTRY_SIZE))
		return -1;  // block is full of file entry
	dir_inode->size += FILE_ENTRY_SIZE;

	memcpy((void *)&last_block + extra_entry * FILE_ENTRY_SIZE, entry, FILE_ENTRY_SIZE);
	write_block(block_index, &last_block, BLOCKSIZE);

	return 0;
}// must be followed write_inode

// add . to the entry
inline int create_empty_directory_entry(char *filename, struct inode *dir_inode) {
	return 0;
}


int read_file_by_inode(struct inode *file_inode, void *buf) {
	// if(check_allocation(file_inode) != 0)
	// 	return -1;
	// suspended

	// struct inode inode;
 //    read_inode(curr_file.inumber, &inode);
 //    if(inode.flags & 0100000 == 0 ) {
 //       	ERROR("Interna: error: not a");
 //    }

    struct inode_data data;
	read_inode_data(file_inode, &data);

	int i;
	int num_full_block = file_inode->size / BLOCKSIZE;
	struct block tmp_block;
	for(i = 0; i < num_full_block; i++) {
		read_block(data.addr[i], &tmp_block, BLOCKSIZE);
		memcpy(buf + BLOCKSIZE * i, &tmp_block, BLOCKSIZE);
	}

	int extra_byte_in_block = file_inode->size - num_full_block * BLOCKSIZE;
	read_block(data.addr[num_full_block], &tmp_block, BLOCKSIZE);
	memcpy(buf + BLOCKSIZE * num_full_block, tmp_block.data, (int)extra_byte_in_block);

	return file_inode->size;
}

int write_file_by_inode(struct inode *file_inode, void *buf, size_t count) {
	// if(check_allocation(file_inode) != 0)
	// 	return -1;
	// allocate_inode(file_inode);

	struct inode_data data;
	read_inode_data(file_inode, &data);

	ensure_enough_blocks(file_inode, file_inode->size + count);
	file_inode->size += count;

	int num_full_blocks = count / BLOCKSIZE;
	struct block tmp_block;
	memset(&tmp_block, 0, BLOCKSIZE);
	int i;
	for(i = 0; i < num_full_blocks; i++) {
		memcpy(&tmp_block, buf + BLOCKSIZE * i, BLOCKSIZE);
		write_block(file_inode->addr[i], &tmp_block, BLOCKSIZE);
	}

	int extra_byte_in_block = count - num_full_blocks * BLOCKSIZE;
	memcpy(tmp_block.data, buf + BLOCKSIZE * num_full_blocks, (int)extra_byte_in_block);
	write_block(file_inode->addr[num_full_blocks], &tmp_block, BLOCKSIZE);
	return 0;
}

int ensure_enough_blocks(struct inode * file_inode, size_t total_size) {
	int num_full_blocks = total_size / BLOCKSIZE;
	if((total_size - num_full_blocks * BLOCKSIZE) > 0)
		num_full_blocks++;

	int already_allocated_blocks = file_inode->size / BLOCKSIZE + 1;
	if((file_inode->size % BLOCKSIZE) == 0)
		already_allocated_blocks--;

	int i;
	for(i = already_allocated_blocks; i < num_full_blocks; i++) {
		file_inode->addr[i] = allocate_block();
	}

	return 0;
}

inline uint filename_to_inode(char *filename, struct V6_file *curr_dir, struct inode *file_inode) {
	int inode = find_file_in_directory(filename, curr_dir);
	//struct inode file_inode;
	read_inode(inode, file_inode);
	return inode;
}
int get_file_size(char *filename) {
	struct inode file_inode;
	int inode = filename_to_inode(filename, &root, &file_inode);
	if(inode <  0)
		return -1;
	else 
		return file_inode.size;
}

size_t read_file(char *filename, void *buf, size_t count) {
	struct inode file_inode;
	int inode = filename_to_inode(filename, &root, &file_inode);
	if(inode <  0)
		return -1;

	return read_file_by_inode(&file_inode, buf);
}

size_t write_file(char *filename, void *buf, size_t count) {
	struct V6_file file_entry;
	
	//int inode = filename_to_inode(filename, &file_unode);
	struct inode file_inode;
	memset(&file_inode, 0, INODESIZE);
	int inode = get_free_inode();
	read_inode(inode, &file_inode);
	allocate_inode(&file_inode);
	//read_inode(inode, &file_inode);
	
	struct inode dir_inode;
	read_inode(root.inumber, &dir_inode);
	
	file_entry.inumber = inode;
	strncpy((char *)file_entry.filename, (const char *)filename, FILENAME_LENGTH);
	
	add_entry_to_inode(&file_entry, &dir_inode);
	write_inode(root.inumber, &dir_inode);

	int ret =  write_file_by_inode(&file_inode, buf, count);
	write_inode(inode, &file_inode);
	return ret;
}

// uint current_directory(const char *filename) {
// 	uint tmp_inode = find_file_in_current_directory(filename);
// 	if(tmp_inode < 1) {
// 		return -1;
// 	}
// }

inline int read_inode_data(struct inode *file_inode, struct inode_data *data) {
	memcpy(data, file_inode->addr, sizeof(uint) * INODE_ADDR_LEN);
	return 0;
}



int is_this_file(struct file_entry *entry, const char* filename) {
	if(strcmp((const char *)entry->filename, filename) == 0)
		return 1;
	else 
		return 0;
}

inline int is_directory(uint inode) {
	struct inode file_inode;
	read_inode(inode, &file_inode);
	return is_directory_inode(&file_inode);
}

inline int is_directory_inode(struct inode *file_inode) {
	if((file_inode->flags & 040000) == 0)
		return 0;
	return 1;
}

inline uint file_to_inode(struct V6_file* file) {
	return file->inumber;
}

// inline uint inode_to_file(uint inode, struct V6_file* file) {
// 	//struct inode inode_buf;
// 	//read_inode(inode, &inode_buf);
// 	//return
// }

int find_directory_in_directory(const char *filename, struct V6_file *spec_dir) {
	uint inode = find_file_in_directory(filename, spec_dir);
	
	if(inode < 0)
		return -1;

	if(is_directory(inode) == 0)
		return -1;
	return inode;
}

int read_directory(struct inode *dir_inode, struct file_entry **entries, int *entry_num) {
	// if(*entries == NULL)
	// 	return -1;

	*entry_num = dir_inode->size / FILE_ENTRY_SIZE; 
	*entries = malloc((*entry_num) * FILE_ENTRY_SIZE);

	return read_file_by_inode(dir_inode, (void *)*entries);
}

//return inode number 
int find_file_in_directory(const char *filename, struct V6_file *spec_dir) {
	
	struct inode curr_inode;
	read_inode(spec_dir->inumber, &curr_inode);
	

	if((curr_inode.flags | 0100000) == 0) {
		ERROR("Internal error: an existing file contains unallocated inode\n");
	}

	if((curr_inode.flags | 040000) == 0) {
		ERROR("Internal error: the current file is NOT directory\n");
	}

	struct file_entry *entries; //allocated in read_directory, you need to free it after use
	int entry_num = 0;
	read_directory(&curr_inode, &entries, &entry_num);

	int i = 0;
	for(i = 0; i < entry_num; i++) {
		if(is_this_file(&entries[i], filename)) {
			int ret = entries[i].inumber;
			//free(entries);
			return ret;
		}
	}

	return -1;
}

//return inode number
uint find_file_in_current_directory(const char *filename) {
	return find_file_in_directory(filename, &root);
}

int list_directory(char ***all_filename, struct V6_file *spec_dir) {
	struct file_entry *entries; //allocated in read_directory, you need to free it after use
	int entry_num = 0;
	uint inode = root.inumber;
	struct inode dir_inode;
	read_inode(inode, &dir_inode);
	read_directory(&dir_inode, &entries, &entry_num);
 	
	//*count = entry_num;
	*all_filename = malloc(FILENAME_LENGTH * entry_num);
	int i = 0;
	for(i = 0; i < entry_num; i++) {
		(*all_filename)[i] = malloc(FILENAME_LENGTH);
		strncpy((*all_filename)[i], (const char *)entries[i].filename, FILENAME_LENGTH);
	}
	// for (i = 0; i < entry_num; ++i)
	// {
	// 	free(entries[i]);
	// }
	free(entries);

	return entry_num;
}
