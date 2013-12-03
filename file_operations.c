#include "common.h"
#include "file.h"
#include "inode.h"
#include "block.h"

//struct V6_file curr_file;
//extern struct V6_file curr_dir;
//extern struct inode curr_inode;
//extern uint curr_inode_num;
int mark_directory(struct inode *dir_inode);
inline int is_directory(uint inode);
ssize_t read_file_by_inode(struct inode *file_inode, void *buf, size_t count);
ssize_t write_file_by_inode(struct inode *file_inode, void *buf, size_t count);
inline int filename_to_inode(char *filename, struct V6_file *curr_dir, struct inode *file_inode);
int create_file_inode(struct inode *file_inode);

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
	mark_directory(&root_inode);
	add_entry_to_inode(&dot, &root_inode);
	add_entry_to_inode(&dotdot, &root_inode);
	write_inode(1, &root_inode);
	memcpy(&root, _root, sizeof(struct V6_file));
	return 0;
}

int add_file_entry_to_directory(struct file_entry *file, struct inode *dir_inode) {
	return 0;
}

int create_directory(char *filename, struct V6_file *spec_dir) {
	int curr_inode_number = find_file_in_directory(filename, spec_dir);
    if (curr_inode_number != -1) {
        printf("Diretory %s exists! Can't override an existing file\n", filename);
        return -1;
    }

    struct V6_file dir_entry;
	struct inode dir_inode;
	memset(&dir_inode, 0, INODESIZE);
	
	int inode = create_file_inode(&dir_inode);
	if(inode < 0)
		ERROR("No available inode\n");
	mark_directory(&dir_inode);
	write_inode(inode, &dir_inode);

	dir_entry.inumber = inode;
	strncpy((char *)dir_entry.filename, (const char *)filename, FILENAME_LENGTH);

	if(add_directory_to_directory(&dir_entry, spec_dir) < 0) {
		INFO("The directory %s is full\n", spec_dir->filename);
		return -1;
	}

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
	read_block(block_index, &last_block);
	int extra_entry = (dir_inode->size % BLOCKSIZE) / FILE_ENTRY_SIZE;
	
	if(extra_entry == (BLOCKSIZE / FILE_ENTRY_SIZE))
		return -1;  // block is full of file entry
	dir_inode->size += FILE_ENTRY_SIZE;

	memcpy((void *)&last_block + extra_entry * FILE_ENTRY_SIZE, entry, FILE_ENTRY_SIZE);
	write_block(block_index, &last_block);

	return 0;
}// must be followed write_inode

// add . to the entry
inline int create_empty_directory_entry(char *filename, struct inode *dir_inode) {
	return 0;
}


// try to open a file, if not exist, then create it
// 0 for no error
int open_file(char *filename, struct V6_file *spec_dir, struct V6_file *opened_file) {
	struct inode file_inode;
	int inode = find_file_in_directory(filename, spec_dir);
	read_inode(inode, &file_inode);
	
	if(is_directory_inode(&file_inode)) {
		INFO("Can NOT open a directory\n");
		return -1;
	}
	if(inode <  0) {// file not exist, so create it
		inode = create_file(filename, spec_dir);
	}

	if(inode < 0)
		return -1;

	strncpy((char *)opened_file->filename, filename, FILENAME_LENGTH);
	opened_file->inumber = inode;

	return 0;
}

ssize_t read_file(struct V6_file *opened_file, void *buf, size_t count) {
	struct inode file_inode;
	 
	assert(read_inode(opened_file->inumber, &file_inode) == 0);

	ssize_t ret = read_file_by_inode(&file_inode, buf, count);
	write_inode(opened_file->inumber, &file_inode);
	return ret;
}

ssize_t write_file(struct V6_file *opened_file, void *buf, size_t count) {
	
	struct inode file_inode;
	memset(&file_inode, 0, INODESIZE);
	
	assert(read_inode(opened_file->inumber, &file_inode) == 0);
	file_inode.size = count;   // overwrite it, the file pointer is pending
	
	ssize_t ret = write_file_by_inode(&file_inode, buf, count);
	write_inode(opened_file->inumber, &file_inode);
	return ret;
}

int filename_contains_slash(const char *filename) {
	while(filename) {
		if(*filename++ == '/')
			return 1;
	}
	return 0;
}

int create_file(char *filename, struct V6_file *spec_dir) {
	// if(filename_contains_slash(filename) == 1) {
	// 	INFO("create file not support recursively creating file now\n");
	// }

	struct V6_file file_entry;
	struct inode file_inode;
	memset(&file_inode, 0, INODESIZE);
	
	int inode = create_file_inode(&file_inode);
	if(inode < 0)
		ERROR("No available inode\n");

	struct inode dir_inode;
	assert(read_inode(spec_dir->inumber, &dir_inode) == 0);

	file_entry.inumber = inode;
	strncpy((char *)file_entry.filename, (const char *)filename, FILENAME_LENGTH);

	if(add_entry_to_inode(&file_entry, &dir_inode) < 0) {
		INFO("The directory %s is full\n", spec_dir->filename);
		return -1;
	}
	assert(write_inode(spec_dir->inumber, &dir_inode) == 0);

	return inode;
}

int create_file_inode(struct inode *file_inode) {
	int inode = get_free_inode();
	if(inode < 2)
		return -1;

	assert(read_inode(inode, file_inode) == 0);
	allocate_inode(file_inode);
	file_inode->size = 0;
	assert(write_inode(inode, file_inode) == 0);

	return inode;
}

ssize_t read_file_by_inode(struct inode *file_inode, void *buf, size_t count) {
	// if(is_allocated_inode(file_inode) == 0)
	// 	return -1;

	if(count <= 0)
		return 0;

	if(count > file_inode->size)
		count = file_inode->size;

    //struct inode_data data;
	//read_inode_data(file_inode, &data);
	
	int num_full_block = count / BLOCKSIZE;
	struct block tmp_block;
	int i;
	for(i = 0; i < num_full_block; i++) {
		read_block(file_inode->addr[i], &tmp_block);
		memcpy(buf + BLOCKSIZE * i, &tmp_block, BLOCKSIZE);
	}

	int extra_byte_in_block = count - num_full_block * BLOCKSIZE;
	read_block(file_inode->addr[num_full_block], &tmp_block);
	memcpy(buf + BLOCKSIZE * num_full_block, tmp_block.data, (int)extra_byte_in_block);

	return count;
}

ssize_t write_file_by_inode(struct inode *file_inode, void *buf, size_t count) {
	// if(is_allocated_inode(file_inode) == 0)
	// 	return -1;

	if(count <= 0)
		return 0;

	struct inode_data data;
	read_inode_data(file_inode, &data);

	//ensure_enough_blocks(file_inode, file_inode->size + count);
	ensure_enough_blocks(file_inode, count);
	file_inode->size = count;

	int num_full_blocks = count / BLOCKSIZE;
	struct block tmp_block;
	memset(&tmp_block, 0, BLOCKSIZE);
	int i;
	for(i = 0; i < num_full_blocks; i++) {
		memcpy(&tmp_block, buf + BLOCKSIZE * i, BLOCKSIZE);
		write_block(file_inode->addr[i], &tmp_block);
	}

	int extra_byte_in_block = count - num_full_blocks * BLOCKSIZE;
	memcpy(tmp_block.data, buf + BLOCKSIZE * num_full_blocks, (int)extra_byte_in_block);
	write_block(file_inode->addr[num_full_blocks], &tmp_block);
	
	return count;
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

inline int filename_to_inode(char *filename, struct V6_file *curr_dir, struct inode *file_inode) {
	int inode = find_file_in_directory(filename, curr_dir);
	//struct inode file_inode;
	read_inode(inode, file_inode);
	return inode;
}


int get_file_size(struct V6_file *spec_file) {
	struct inode file_inode;
	read_inode(spec_file->inumber, &file_inode);
	
	return file_inode.size;
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


int read_directory(struct inode *dir_inode, struct file_entry **entries, int *entry_num) {
	if(entries == NULL)
	 	ERROR("NULL pointer input to read_directory\n");

	*entry_num = dir_inode->size / FILE_ENTRY_SIZE; 
	*entries = malloc((*entry_num) * FILE_ENTRY_SIZE);

	return read_file_by_inode(dir_inode, (void *)*entries, dir_inode->size);
}


int list_directory(char ***all_filename, struct V6_file *spec_dir) {
	struct file_entry *entries; //allocated in read_directory, you need to free it after use
	int entry_num = 0;
	uint inode = spec_dir->inumber;
	struct inode dir_inode;
	read_inode(inode, &dir_inode);
	read_directory(&dir_inode, &entries, &entry_num);
 	
	//*count = entry_num;
	*all_filename = (char **)malloc(entry_num * sizeof(char *));
	int i = 0;
	for(i = 0; i < entry_num; i++) {
		(*all_filename)[i] = malloc(FILENAME_LENGTH);
		strncpy((*all_filename)[i], (const char *)entries[i].filename, FILENAME_LENGTH);
	}
	
	free((void *)entries);

	return entry_num;
}

// return the pointer just after the '/'
// the *filename must be preallocated either in stack or heap
const char *split_filename_from_path(const char *path, char *filename) {
	const char *tmp = path;
	for(;tmp != NULL && 
		*tmp != 0 &&
		*tmp != '/'; tmp++) {}

	int length = tmp - path;

	if(length > FILENAME_LENGTH - 1)
		ERROR("filename too long\n");

	if(length == 0) {
		strcpy(filename, "/");
	} else {
		strncpy(filename, path, length);
		filename[length] = 0;
	}
	
	return (*tmp == 0) ? tmp : tmp + 1;
}

int current_directory(char *filename, struct V6_file *spec_dir) {
	if(!is_directory(spec_dir->inumber))
		return -1;

	int inode = find_directory_in_directory(filename, spec_dir);
	if(inode > 0)
		spec_dir->inumber = inode;

	return inode;
}

int find_directory_in_directory(const char *filename, struct V6_file *spec_dir) {
	uint inode = find_file_in_directory(filename, spec_dir);
	
	if(inode < 0)
		return -1;

	if(is_directory(inode) == 0)
		return -1;
	return inode;
}

//return inode number 
int find_file_in_directory(const char *filename, struct V6_file *spec_dir) {
	
	struct inode curr_inode;
	char split_filename[FILENAME_LENGTH];
	const char *child_path = split_filename_from_path(filename, split_filename);
	if(strcmp(split_filename, "/") == 0) {
		read_inode(1, &curr_inode);  // absolute path
	}
	else {
		read_inode(spec_dir->inumber, &curr_inode);
	}
	

	if((curr_inode.flags & 0100000) == 0) {
		ERROR("Internal error: an existing file contains unallocated inode\n");
	}

	// if((curr_inode.flags | 040000) == 0) {
	// 	ERROR("Internal error: the current file is NOT directory\n");
	// }

	struct file_entry *entries; //allocated in read_directory, you need to free it after use
	int entry_num = 0;
	read_directory(&curr_inode, &entries, &entry_num);

	int ret = -1;
	int i = 0;
	for(i = 0; i < entry_num; i++) {
		if(is_this_file(&entries[i], split_filename)) {		
			if(*child_path == 0) {  // the last filename of a path  .../.../.../last
				ret = entries[i].inumber;
			} else {
				ret = find_file_in_directory(child_path, (struct V6_file *)&entries[i]);
			}
			break;
		}
	}
	free(entries);
	//INFO("Can NOT find %s\n", split_filename);
	return ret;
}

//return inode number
int find_file_in_current_directory(const char *filename, struct V6_file* spec_dir) {
	return find_file_in_directory(filename, spec_dir);
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

int mark_directory(struct inode *dir_inode) {
	dir_inode->flags |= 040000;
	return 1;
}

inline uint file_to_inode(struct V6_file* file) {
	return file->inumber;
}
